#pragma once

#include <Hashing.h>
#include <Memory.h>

#define SL_HASHTABLE_CAPACITY 256

namespace sl
{
    template <typename Key, typename Value>
    class HashTable
    {
    private:
        class EntryNode
        {
        public:
            Key key;
            Value value;
            EntryNode* next;

            EntryNode(Key k, Value v) : key(k), value(v), next(nullptr) {}
        };

        size_t count;
        size_t capacity;
        Value defaultValue; //TODO: get rid of this hack. See also List/LinkedList implementations
        EntryNode** store;

    public:
        HashTable() : count(0), capacity(SL_HASHTABLE_CAPACITY)
        {
            store = new EntryNode*[capacity];
        }

        ~HashTable()
        {
            for (size_t i = 0; i < capacity; i++)
            {
                EntryNode* deleteMe = store[i];
                while (store[i] != nullptr)
                {
                    store[i] = deleteMe->next;
                    delete deleteMe;
                }
            }
            delete[] store;
        }

        HashTable(const HashTable& other) = delete;
        HashTable& operator=(const HashTable& other) = delete;
        HashTable(HashTable&& from) = delete;
        HashTable& operator=(HashTable&& from) = delete;

        size_t Size()
        {
            return count;
        }

        void Insert(const Key& key, const Value value)
        {
            count++;
            auto hash = CRC8(&key, sizeof(key));
            hash = hash % SL_HASHTABLE_CAPACITY;
            
            if (store[hash] == nullptr)
            {
                //no collision, easy algo
                store[hash] = new EntryNode(key, value);
                return;
            }

            //traverse list, and insert at end
            EntryNode* parent = store[hash];
            while (parent->next != nullptr)
                parent = parent->next;
            
            parent->next = new EntryNode(key, value);
        }

        Value& At(const Key& key)
        {
            auto hash = CRC8(&key, sizeof(key));
            hash = hash % SL_HASHTABLE_CAPACITY;

            EntryNode* parent = store[hash];
            while (parent != nullptr)
            {
                if (parent->key == key)
                    return parent->value;

                parent = parent->next;
            }

            return defaultValue;
        }

        bool Has(const Key& key)
        {
            auto hash = CRC8(&key, sizeof(key));
            hash = hash % SL_HASHTABLE_CAPACITY;

            EntryNode* parent = store[hash];
            while (parent != nullptr)
            {
                if (parent->key == key)
                    return true;

                parent = parent->next;
            }

            return false;
        }

        Value Remove(const Key& key)
        {
            auto hash = CRC8(&key, sizeof(key));
            hash = hash % SL_HASHTABLE_CAPACITY;

            EntryNode* current = store[hash];
            EntryNode* parent = nullptr;
            while (current != nullptr)
            {
                if (current->key == key)
                {
                    if (parent) //stitch over current node
                        parent->next = current->next;
                    else if (current->next) //no parent, node is new list head
                        store[hash] = current->next;
                    else //no parent or child, mark it as empty
                        store[hash] = nullptr;

                    Value v = move(current->value);
                    delete current;
                    count--;

                    return v;
                }

                parent = current;
                current = current->next;
            }

            return defaultValue;
        }

        Value& operator[](const Key& key)
        {
            return At(key);
        }
    };
}