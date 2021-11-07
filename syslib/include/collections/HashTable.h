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
    template<typename T, typename TraitsForT, typename Allocator = HeapAllocator>
    class HashTable
    {
    public:
        template<typename BucketType>
        class Iterator
        {
        friend HashTable;
        private:
            BucketType* bucket = nullptr;

            explicit Iterator(BucketType* bucket) : bucket(bucket) {}

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
            bool operator==(const Iterator& other) const
            { return bucket == other.bucket; }

            bool operator!=(const Iterator& other) const
            { return bucket != other.bucket; }

            T& operator*() 
            { return *bucket->Slot(); }

            T* operator->()
            { return bucket->Slot(); }

            void operator++()
            { SkipToNext(); }
        };

    private:
        static constexpr size_t LoadFactorPercent = 60;

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
            size_t oldCapacity = capacity;
            Itr oldItr = Begin();

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

            for (auto it = sl::move(oldItr); it != End(); it++)
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
        Bucket* LookupWithHash(unsigned hash, UnaryPredicate predicate) const
        {
            if (IsEmpty())
                return nullptr;

            for (;;)
            {
                auto& bucket = buckets[hash % capacity];

                if (bucket.used && preciate(*bucket.Slot()))
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

            auto hash = TraitsForT::hash(value);
            Bucket* firstEmptyBucket = nullptr;
            for(;;)
            {
                auto& bucket = buckets[hash % capacity];

                if (bucket.used && TraitsForT::equals(*bucket.Slot(), value))
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

        using Itr = Iterator<Bucket>;
        Itr Begin()
        {
            for (size_t i = 0; i < capacity; i++)
            {
                if (buckets[i].used)
                    return Itr(&buckets[i]);
                return End();
            }
        }

        Itr End()
        {
            return Itr(nullptr);
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
        Itr Find(unsigned hash, UnaryPredicate predicate)
        {
            return Itr(LookupWithHash(hash, sl::move(predicate)));
        }

        Itr Find(const T& value)
        {
            return Find(TraitsForT::hash(value), [&](auto& other) { return TraitsForT::equals(value, other); });
        }

        bool Remove(const T& value)
        {
            Itr it = Find(value);
            if (it != End())
            {
                Remove(it);
                return true;
            }

            return false;
        }

        bool Remove(Itr iterator)
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
