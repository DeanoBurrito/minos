#pragma once

#include <Memory.h>

namespace sl
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
        Value defaultValue;
        LinkedListEntry<Value>* head;
        LinkedListEntry<Value>* tail;
        size_t count;

        void DoCopy(const LinkedList& other)
        {
            count = 0;
            LinkedListEntry<Value>* scan = other.head;
            LinkedListEntry<Value>* last = nullptr;
            while (scan != nullptr)
            {
                LinkedListEntry<Value>* current = new LinkedListEntry<Value>(scan->val);
                current->prev = last;
                last = current;
                count++;

                if (head == nullptr)
                    head = current;
            }

            tail = last;
        }

    public:
        LinkedList()
        {
            //TODO: this is slow af. Worth going to a bucket based implementation at some point.
            head = tail = nullptr;
            count = 0;
            defaultValue = Value();
        }

        LinkedList(const LinkedList& other)
        {
            DoCopy(other);
        }

        LinkedList& operator=(const LinkedList& other)
        {
            Clear();
            DoCopy(other);
            return *this;
        }

        LinkedList(LinkedList&& from)
        {
            swap(from.head, head);
            swap(from.tail, tail);
            swap(from.count, count);
        }

        LinkedList& operator=(LinkedList&& from)
        {
            Clear();
            swap(from.head, head);
            swap(from.tail, tail);
            swap(from.count, count);
            return *this;
        }

        ~LinkedList()
        {
            Clear();
        }

        unsigned int Size()
        {
            return count;
        }

        LinkedListEntry<Value>* Head()
        {
            return head;
        }

        LinkedListEntry<Value>* Tail()
        {
            return tail;
        }

        Value PeekBack()
        {
            if (tail == nullptr)
                return defaultValue;
            return tail->val;
        }

        Value PeekFront()
        {
            if (head == nullptr)
                return defaultValue;
            return head->val;
        }

        void PushBack(const Value val)
        {
            InsertAfter(tail, val);
        }

        void PushFront(const Value val)
        {
            InsertBefore(head, val);
        }

        Value PopBack()
        {
            if (tail == nullptr)
                return defaultValue;

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
                return defaultValue;
            
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
            
            while (head != nullptr)
            {
                LinkedListEntry<Value>* deleteMe = head;
                head = head->next;
                delete deleteMe;
            }
            count = 0;
            head = tail = nullptr;
        }

        LinkedListEntry<Value>* Find(const Value value)
        {
            if (head == nullptr)
                return nullptr;
            
            LinkedListEntry<Value>* current = head;  
            do
            {
                if (current->val == value)
                    return current;
                
                current = current->next;
            }
            while (current != nullptr);

            return nullptr;
        }

        LinkedListEntry<Value>* FindR(const Value value)
        {
            if (tail == nullptr)
                return nullptr;
            
            LinkedListEntry<Value>* current = tail;
            do
            {
                if (current->val == value)
                    return current;

                current = current->prev;
            }
            while (current != nullptr);

            return nullptr;
        }

        void Remove(LinkedListEntry<Value>* const entry)
        {
            if (entry == nullptr)
                return;
            
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

        void InsertBefore(LinkedListEntry<Value>* const entry, const Value val)
        {
            if (entry == nullptr)
            {
                //if entry is null, do nothing UNLESS the head is also null, then we set it as the head.
                if (head == nullptr)
                {
                    LinkedListEntry<Value>* newEntry = new LinkedListEntry<Value>(val);
                    head = newEntry;
                    tail = newEntry;

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

        void InsertAfter(LinkedListEntry<Value>* const entry, const Value val)
        {
            if (entry == nullptr)
            {
                //if tail is null, set this to the tail
                if (tail == nullptr)
                {
                    LinkedListEntry<Value>* newEntry = new LinkedListEntry<Value>(val);        
                    tail = newEntry;
                    head = newEntry;

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
            entry->next = newEntry;

            if (entry == tail)
                tail = newEntry;
            
            count++;
        }

        Value& At(size_t index)
        {
            if (index >= count)
                return defaultValue;

            size_t current = 0;
            LinkedListEntry<Value>* scan = head;
            while (current != index && scan != nullptr)
            {
                current++;
                scan = scan->next;
            }
            return scan->val;
        }

        Value& operator[](size_t index)
        {
            return At(index);
        }
    };
}