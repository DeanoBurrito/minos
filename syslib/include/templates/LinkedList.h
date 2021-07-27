#pragma once

namespace Syslib
{    
    template <typename Value>
    class LinkedListEntry
    {
    public:
        LinkedListEntry<Value>* next = nullptr;
        LinkedListEntry<Value>* prev = nullptr;
        Value val;

        LinkedListEntry(Value val) : val(val) {}
    };
    
    template <typename Value>
    class LinkedList
    {
    private:
        LinkedListEntry<Value>* head;
        LinkedListEntry<Value>* tail;
        unsigned long count;

    public:
        LinkedList()
        {
            //TODO: this is slow af. Worth going to a bucket based implementation at some point.
            head = tail = nullptr;
            count = 0;
        }

        ~LinkedList()
        {
            Clear();
        }

        unsigned int Size()
        {
            return count;
        }

        void PushBack(Value val)
        {
            InsertAfter(tail, val);
        }

        void PushFront(Value val)
        {
            InsertBefore(head, val);
        }

        Value PopBack()
        {
            if (tail == nullptr)
                return Value();

            Value retval = tail->val;
            if (tail->prev != nullptr)
            {
                auto newTail = tail->prev;
                newTail->next = nullptr;

                delete tail; //get rid of old tail, and set tail to new value
                tail = newTail;
            }
            else
            {
                delete tail;
                tail = nullptr;
            }
            
            count--;
            return retval;
        }

        Value PopFront()
        {
            if (head == nullptr)
                return Value();
            
            Value retval = head->val;
            if (head->next != nullptr)
            {
                auto newHead = head->next;
                newHead->prev = nullptr;

                delete head; //get rid of old head
                head = newHead;
            }
            else
            {
                delete head;
                head = nullptr;
            }
            
            count--;
            return retval;
        }

        void Clear()
        {
            if (head == nullptr)
                return;
            
            LinkedListEntry<Value>* next = head->next;
            while (head != nullptr)
            {
                delete head;
                head = next;
            }

            count = 0;
            //TODO: asset that end is also nullptr
        }

        LinkedListEntry<Value>* Find(Value value)
        {
            if (head == nullptr)
                return nullptr;
            
            LinkedListEntry<Value>* current = head;
            while (current->next != nullptr)
            {
                if (current->val == value)
                    return current;
            }

            return nullptr;
        }

        LinkedListEntry<Value>* FindR(Value value)
        {
            if (tail == nullptr)
                return nullptr;
            
            LinkedListEntry<Value>* current = tail;
            while (current->prev != nullptr)
            {
                if (current->val == value)
                    return current;
            }

            return nullptr;
        }

        void Remove(LinkedListEntry<Value>* entry)
        {
            if (entry->next != nullptr)
                entry->next->prev = entry->prev;
            if (entry->prev != nullptr)
                entry->prev->next = entry->next;
            
            if (head == entry)
                head = entry->next;
            if (tail == entry)
                tail = entry->prev;
            
            count--;
            delete entry;
        }

        void InsertBefore(LinkedListEntry<Value>* entry, Value val)
        {
            if (entry == nullptr)
            {
                //if entry is null, do nothing UNLESS the head is also null, then we set it as the head.
                if (head == nullptr)
                {
                    head = entry;
                    head->next = tail;

                    count++;
                    return;
                }
                else
                    return;
            }

            LinkedListEntry<Value>* newEntry = new LinkedListEntry<Value>(val);
            if(entry->prev != nullptr)
                entry->prev->next = newEntry;
            newEntry->prev = entry->prev;
            newEntry->next = entry;
            entry->prev = newEntry;

            if (entry == head)
                head = newEntry;

            count++;
        }

        void InsertAfter(LinkedListEntry<Value>* entry, Value val)
        {
            if (entry == nullptr)
            {
                //if tail is null, set this to the tail
                if (tail == nullptr)
                {
                    tail = entry;
                    tail->prev = head;

                    count++;
                    return;
                }
                else
                    return;
            }

            LinkedListEntry<Value>* newEntry = new LinkedListEntry<Value>(val);
            if (entry->next != nullptr)
                entry->next->prev = newEntry;
            newEntry->next = entry->next;
            newEntry->prev = entry;

            if (entry == tail)
                tail = newEntry;
            
            count++;
        }
    };
}