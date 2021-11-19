#pragma once

#include <CppStd.h>
#include <stdint.h>
#include <Memory.h>
#include <PlacementNew.h>
#include <Maths.h>
#include <Hashing.h>
#include <Allocators.h>

namespace sl
{
    enum class HashSetResult
    {
        Failed = 0,
        InsertedNew,
        ReplacedExisting,
        KeptExisting,
    };

    enum class HashSetOverwriteBehaviour
    {
        Keep,
        Replace,
    };
    
    template<typename T, typename TraitsForT, typename Allocator = HeapAllocator>
    class HashTable
    {
    public:
        template<typename HashTableType, typename Type, typename BucketType>
        class IteratorBase
        {
        friend HashTableType;
        private:
            BucketType* bucket = nullptr;

            explicit IteratorBase(BucketType* bucket) : bucket(bucket) {}

            void SkipToNext()
            {
                if (!bucket)
                    return;

                do
                {
                    ++bucket;
                    if (bucket->used)
                        return;
                }
                while (!bucket->end);

                if (bucket->end)
                    bucket = nullptr;
            }

        public:
            bool operator==(const IteratorBase& other) const
            { return bucket == other.bucket; }

            bool operator!=(const IteratorBase& other) const
            { return bucket != other.bucket; }

            Type& operator*() 
            { return *bucket->Slot(); }

            Type* operator->()
            { return bucket->Slot(); }

            void operator++()
            { SkipToNext(); }
        };

    private:
        static constexpr size_t LoadFactorPercent = 60;

        struct Bucket
        {
            bool used;
            bool deleted;
            bool end;
            alignas(T) uint8_t storage[sizeof(T)];

            T* Slot()
            { return reinterpret_cast<T*>(storage); }

            const T* Slot() const
            { return reinterpret_cast<const T*>(storage); }
        };

        size_t size = 0;
        size_t capacity = 0;
        size_t deletedCount = 0;
        Bucket* buckets = nullptr;

        Allocator allocator;

        void InsertDuringRehash(T&& value)
        {
            auto& bucket = LookupForWriting(value);
            new(bucket.Slot()) T(sl::move(value));
            bucket.used = true;
        }

        static constexpr size_t SizeInBytes(size_t capacity)
        { return sizeof(Bucket) * capacity; }

        bool TryRehash(size_t newCapacity)
        {
            newCapacity = Max(newCapacity, static_cast<size_t>(4));

            auto* oldBuckets = buckets;
            //size_t oldCapacity = capacity;
            Iterator oldItr = Begin();

            auto newBuckets = allocator.Alloc(SizeInBytes(newCapacity));
            if (!newBuckets)
                return false;

            buckets = (Bucket*)newBuckets;
            sl::memset(buckets, 0, SizeInBytes(newCapacity));
            capacity = newCapacity;
            deletedCount = 0;
            buckets[capacity].end = true;

            if (!oldBuckets)
                return true;

            for (auto it = sl::move(oldItr); it != End(); ++it)
            {
                InsertDuringRehash(sl::move(*it));
                it->~T();
            }

            allocator.Free(buckets); //NOTE: we should probably pass SizeInBytes(oldCapacity)
            return true;
        }

        void Rehash(size_t newCapacity)
        {
            TryRehash(newCapacity);
        }

        template<typename UnaryPredicate>
        Bucket* LookupWithHash(uint32_t hash, UnaryPredicate predicate) const
        {
            if (IsEmpty())
                return nullptr;

            for (;;)
            {
                auto& bucket = buckets[hash % capacity];

                if (bucket.used && predicate(*bucket.Slot()))
                    return &bucket;
                
                if (!bucket.used && !bucket.deleted)
                    return nullptr;

                hash = sl::Hashes::DoubleSimple(hash);
            }
        }

        Bucket* TryLookupForWriting(const T& value)
        {
            if (ShouldGrow())
            {
                if (!TryRehash(Capacity() * 2))
                    return nullptr;
            }

            auto hash = TraitsForT::Hash(value);
            Bucket* firstEmptyBucket = nullptr;
            for(;;)
            {
                auto& bucket = buckets[hash % capacity];

                if (bucket.used && TraitsForT::Equals(*bucket.Slot(), value))
                    return &bucket;

                if (!bucket.used)
                {
                    if (!firstEmptyBucket)
                        firstEmptyBucket = &bucket;
                    if (!bucket.deleted)
                        return const_cast<Bucket*>(firstEmptyBucket);
                }

                hash = sl::Hashes::DoubleSimple(hash);
            }
        }

        Bucket& LookupForWriting(const T& value)
        {
            return *TryLookupForWriting(value);
        }

        size_t UsedBucketCount() const
        { return size + deletedCount; }

        bool ShouldGrow() const
        {
            return ((UsedBucketCount() + 1) * 100) >= (capacity * LoadFactorPercent);
        }

    public:
        HashTable() = default;
        explicit HashTable(size_t capacity)
        { Rehash(capacity); }

        ~HashTable()
        {
            if (!buckets)
                return;

            for (size_t i = 0; i < capacity; i++)
            {
                if (buckets[i].used)
                    buckets[i].Slot()->~T();
            }

            allocator.Free(buckets);
        }

        HashTable(const HashTable& other)
        {
            Rehash(other.capacity);
            for (auto& it : other)
                Set(it);
        }

        HashTable& operator=(const HashTable& other)
        {
            HashTable temp(other);
            swap(*this, temp);
            return *this;
        }

        HashTable(HashTable&& from)
        {
            swap(*this, from);
        }
        
        HashTable& operator=(HashTable&& from)
        {
            HashTable temp { sl::move(from) };
            swap(*this, temp);
            return *this;
        }

        friend void swap(HashTable& a, HashTable& b)
        {
            sl::swap(a.buckets, b.buckets);
            sl::swap(a.size, b.size);
            sl::swap(a.capacity, b.capacity);
            sl::swap(a.deletedCount, b.deletedCount);
        }

        bool IsEmpty() const
        { return !size; }

        size_t Size() const
        { return size; }

        size_t Capacity() const
        { return capacity; }

        template<typename U, size_t N>
        bool TrySetFrom(U (&FromArray)[N])
        {
            for (size_t i = 0; i < N; i++)
            {
                if (TrySet(FromArray[i]) == HashSetResult::Failed)
                    return false;
            }
            return true;
        }

        template<typename U, size_t N>
        void SetFrom(U (&FromArray)[N])
        {
            TrySetFrom(FromArray);
        }

        void EnsureCapacity(size_t capacity)
        {
            Rehash(capacity * 2);
        }

        [[nodiscard]] bool Contains(const T& value) const
        {
            return Find(value) != End();
        }

        using Iterator = IteratorBase<HashTable, T, Bucket>;
        Iterator Begin()
        {
            for (size_t i = 0; i < capacity; i++)
            {
                if (buckets[i].used)
                    return Iterator(&buckets[i]);
            }
            return End();
        }

        Iterator End()
        {
            return Iterator(nullptr);
        }

        using ConstIterator = IteratorBase<const HashTable, const T, const Bucket>;
        ConstIterator Begin() const
        {
            for (size_t i = 0; i < capacity; i++)
            {
                if (buckets[i].used)
                    return ConstIterator(&buckets[i]);
            }
            return End();
        }

        ConstIterator End() const
        {
            return ConstIterator(nullptr);
        }

        void Clear()
        {
            *this = HashTable();
        }

        template<typename U = T>
        HashSetResult TrySet(U&& value, HashSetOverwriteBehaviour overwriteBehaviour = HashSetOverwriteBehaviour::Replace)
        {
            Bucket* bucket = TryLookupForWriting(value);
            if (!bucket)
                return HashSetResult::Failed;

            if (bucket->used)
            {
                if (overwriteBehaviour == HashSetOverwriteBehaviour::Keep)
                    return HashSetResult::KeptExisting;
                
                (*bucket->Slot()) = sl::Forward<U>(value);
                return HashSetResult::ReplacedExisting;
            }

            new(bucket->Slot()) T(sl::Forward<U>(value));
            bucket->used = true;
            if (bucket->deleted)
            {
                bucket->deleted = false;
                deletedCount--;
            }

            size++;
            return HashSetResult::InsertedNew;
        }

        template<typename U = T>
        HashSetResult Set(U&& value, HashSetOverwriteBehaviour overwriteBehaviour = HashSetOverwriteBehaviour::Replace)
        {
            return TrySet(sl::Forward<U>(value), overwriteBehaviour); 
        }

        template<typename UnaryPredicate>
        Iterator Find(uint32_t hash, UnaryPredicate predicate)
        {
            return Iterator(LookupWithHash(hash, sl::move(predicate)));
        }

        Iterator Find(const T& value)
        {
            return Find(TraitsForT::Hash(value), [&](auto& other) { return TraitsForT::Equals(value, other); });
        }

        bool Remove(const T& value)
        {
            Iterator it = Find(value);
            if (it != End())
            {
                Remove(it);
                return true;
            }

            return false;
        }

        bool Remove(Iterator iterator)
        {
            if (!iterator.bucket)
                return false;
            auto& bucket = *iterator.bucket;
            if (!bucket.used || bucket.deleted)
                return false;

            bucket.Slot()->~T();
            bucket.used = false;
            bucket.deleted = true;
            size--;
            deletedCount++;
        }
    };
}
