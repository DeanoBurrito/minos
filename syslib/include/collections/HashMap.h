/*
    Original code from managarm/frigg 0 a lightweight c++ utilities and algorithms library,
    written by avdgrinten and others. (see https://github.com/managarm/frigg)

    Used under the MIT license which permits free use as long as the original copyright notice is retained. 
    The original party isnt held responsible in any way for the outcome of the software (good or bad).

    Big thanks from myself, the managarm codebase is an amazing resource to be able to use in my own projects. <3
*/

#pragma once

#include <Hashing.h>
#include <Memory.h>

namespace sl
{
    template <typename Key, typename Value>
    class HashMap
    {
    private:
        class MapEntry
        {
        public:
            Key key;
            Value value;
            MapEntry* next;

            MapEntry(const Key& k, const Value& v) 
            : key(k), value(v), next(nullptr) 
            {}

            MapEntry(const Key& k, Value&& v) 
            : key(k), value(sl::move(v)), next(nullptr) 
            {}
        };

        size_t count;
        size_t capacity;

    public:
        class ConstIterator
        {
        friend class HashMap;

        private:

        public:
        };

        class Iterator
        {
        friend class HashMap;

        private:
            HashMap* owner;
            size_t bucket;
            MapEntry* entry;

            Iterator(HashMap* owner, size_t bucket, MapEntry* entry)
            : owner(owner), bucket(bucket), entry(entry)
            {}
        
        public:
            Iterator& operator++()
            {
                entry = entry->next;
                if (entry)
                    return *this;

                //NOTE: we should check that our bucket is still within range here
                while (true)
                {
                    bucket++;
                    if (bucket == owner->capacity)
                        break;
                    
                    entry = owner->entries[bucket];
                    if (entry)
                        break;
                }

                return *this;
            }

            bool operator==(const Iterator& other) const
            {
                return bucket == other.bucket && entry == other.entry;
            }


        };

    private:
        MapEntry** entries;

        void Adjust();
    };
}