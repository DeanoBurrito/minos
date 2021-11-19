#pragma once

#include <Hashing.h>
#include <Memory.h>
#include <Optional.h>
#include <collections/HashTable.h>

namespace sl
{
    template<typename Key, typename Value, typename KeyTraits>
    class HashMap
    {
    private:
        struct Entry
        {
            Key key;
            Value value;
        };

        struct EntryTraits
        {
            static uint32_t Hash(const Entry& entry)
            { return KeyTraits::Hash(entry.key); }

            static bool Equals(const Entry& a, const Entry& b)
            { return KeyTraits::Equals(a.key, b.key); }
        };

        HashTable<Entry, EntryTraits> table;

    public:

        HashMap() = default;

        //TODO: initializer list ctor

        bool IsEmpty() const
        { return table.IsEmpty(); }

        size_t Size() const
        { return table.Size(); }

        size_t Capacity() const
        { return table.Capacity(); }

        void Clear()
        { table.Clear(); }

        HashSetResult Set(const Key& k, const Value& v)
        { return table.Set({k, v}); }

        HashSetResult Set(const Key& k, const Value&& v)
        { return table.Set({k, sl::move(v)}); }

        HashSetResult TrySet(const Key& k, const Value& v)
        { return table.TrySet({k, v}); }

        bool TrySet(const Key& k, const Value&& v)
        { return table.TrySet({k, sl::move(v)}); }

        bool Remove(const Key& k)
        {
            auto it = Find(k);
            if (it != End())
            {
                table.Remove(it);
                return true;
            }
            return false;
        }

        using HashTableType = HashTable<Entry, EntryTraits>;
        using IteratorType = typename HashTableType::Iterator;
        using ConstIteratorType = typename HashTableType::ConstIterator;

        IteratorType Begin()
        { return table.Begin(); }
        
        IteratorType End()
        { return table.End(); }
        
        IteratorType Find(const Key& key)
        {
            return table.Find(KeyTraits::Hash(key), [&](auto& entry) { return KeyTraits::Equals(key, entry.key); });
        }

        template<typename UnaryPredicate>
        IteratorType Find(uint32_t hash, UnaryPredicate predicate)
        { return table.Find(hash, predicate); }

        ConstIteratorType Begin() const
        { return table.Begin(); }

        ConstIteratorType End() const
        { return table.End(); }

        ConstIteratorType Find(const Key& key) const
        {
            return table.Find(KeyTraits::Hash(key), [&](auto& entry) { return KeyTraits::Equals(key, entry.key); });
        }

        template<typename UnaryPredicate>
        ConstIteratorType Find(uint32_t hash, UnaryPredicate predicate) const
        { return table.Find(hash, predicate); }

        void EnsureCapacity(size_t capacity)
        { table.EnsureCapacity(capacity); }

        Optional<Value> Get(const Key& key) const
        {
            auto it = Find(key);
            if (it == End())
                return {};
            return (*it).value;
        }

        bool Contains(const Key& key)
        { return Find(key) != End(); }

        void Remove(IteratorType it)
        { table.Remove(it); }

        Value& Ensure(const Key& key)
        {
            auto it = Find(key);
            if (it != End())
                return it->value;

            //not found, so create it
            auto result = Set(key, Value());
            return Find(key)->value;
        }

        template<typename Callback>
        Value& Ensure(const Key& key, Callback initCallback)
        {
            auto it = Find(key);
            if (it != End())
                return it->value;

            auto result = Set(key, initCallback);
            return Find(key);
        }

        //TODO: vector GetKeys()

        uint32_t Hash() const
        {
            uint32_t hash = 0;
            for (auto& it : *this)
            {
                auto entryHash = Hashes::PairSimple(it.key.Hash(), it.value.Hash());
                hash = Hashes::PairSimple(hash, entryHash);
            }
            return hash;
        }
    };
}