#pragma once

#include <stddef.h>
#include <Memory.h>
#include <CppStd.h>

namespace sl
{
    //A fixed-size circular FIFO queue, this implementation IS NOT thread-safe. You'll need to lock around accesses yourself.
    template<typename T, size_t BufferLength>
    class CircularQueue
    {
    public:
        class Iterator
        {
        friend CircularQueue;
        private:
            size_t index;
            CircularQueue* owner;

            Iterator(CircularQueue* ownerQueue, size_t idx) : index(idx), owner(ownerQueue)
            {}

            //non-movable - just dont bother with that for such a small struct
            Iterator(Iterator&& from) = delete;
            Iterator& operator=(Iterator&& from) = delete;

        public:
            operator bool() const
            {
                return index == owner->Capacity();
            }

            T* operator->()
            {
                return &owner->buffer[index];
            }

            T& operator*()
            {
                return owner->buffer[index];
            }

            const T* operator->() const
            {
                return const_cast<const T*>(&owner->buffer[index]);
            }

            const T& operator*() const
            {
                return const_cast<const T&>(owner->buffer[index]);
            }

            void operator++()
            {   
                //we're at the end, jump to an invalid element
                if (index == owner->last)
                {
                    index = BufferLength;
                    return;
                }

                //handling wrapping
                if (index == BufferLength - 1 && owner->first > owner->last)
                {
                    index = 0;
                    return;
                }
                
                //otherwise we just increment normally
                index++;
            }

            void operator--()
            {
                if (index == owner->first)
                {
                    index = BufferLength;
                    return;
                }

                if (index == 0 && owner->last < owner->first)
                {
                    index = BufferLength - 1;
                    return;
                }
                
                index--;
            }
        };

    private:
        size_t first;
        size_t last;
        T* buffer;

        size_t NextFreeSlot()
        {
            //easy case: buffer is empty
            if (first == BufferLength)
            {
                first = last = 0;
                return 0;
            }

            if (first <= last) //linear mode
            {
                if (last == BufferLength - 1)
                {
                    if (first == 0)
                        return BufferLength; //cant push, buffer full
                    
                    last = 0;
                    return 0;
                }
                
                //base case, just increment
                return ++last;
            }
            else //wrap around mode
            {
                if (last == first - 1)
                    return BufferLength;
                
                return ++last;
            }
        }

    public:
        CircularQueue()
        {
            buffer = new T[BufferLength];
            first = last = BufferLength;
        }

        ~CircularQueue()
        {
            if (buffer)
                delete[] buffer;
        }

        CircularQueue(const CircularQueue& other)
        {
            first = other.first;
            last = other.last;

            buffer = new T[BufferLength];
            for (size_t i = 0; i < BufferLength; i++)
                buffer[i] = other.buffer[i];
        }

        CircularQueue& operator=(const CircularQueue& other)
        {
            if (buffer)
                delete[] buffer;
            
            first = other.first;
            last = other.last;

            buffer = new T[BufferLength];
            for (size_t i = 0; i < BufferLength; i++)
                buffer[i] = other.buffer[i];

            return *this;
        }

        CircularQueue(CircularQueue&& from)
        {
            sl::swap(buffer, from.buffer);
            sl::swap(first, from.first);
            sl::swap(last, from.last);
        }

        CircularQueue& operator=(CircularQueue&& from)
        {
            sl::swap(buffer, from.buffer);
            sl::swap(first, from.first);
            sl::swap(last, from.last);

            return *this;
        }

        constexpr size_t Capacity()
        { return BufferLength; }

        size_t Size() const
        { 
            if (last < first)
                return last + (BufferLength - first) + 1;
            else if (first == BufferLength)
                return 0;
            else
                return last - first + 1;
        }

        void Push(T& entry)
        {
            size_t nextSlot = NextFreeSlot();

            if (nextSlot != BufferLength)
                buffer[nextSlot] = entry;
        }

        template<typename... Parameters>
        void Emplace(Parameters&&... params)
        {
            size_t where = NextFreeSlot();

            if (where != BufferLength)
                new(&buffer[where]) T(Forward<Parameters>(params)...);
        }

        bool TryPush(T& entry)
        { 
            if (Size() == Capacity())
                return false; //no room
            
            Push(entry);
            return true;
        }

        template<typename... Parameters>
        bool TryEmplace(Parameters&&... params)
        { 
            if (Size() == Capacity())
                return false;

            Emplace(sl::Forward<Parameters>(params)...);
            return true;
        }

        [[nodiscard]] T Pop()
        {
            if (first == BufferLength)
                return sl::move(T()); //TODO: there might be a better solution than default ctor?
            
            size_t outIndex = first;

            if (first == last)
                first = last = BufferLength;
            else if (first == BufferLength - 1)
                first = 0;
            else
                first++;
            
            return sl::move(buffer[outIndex]);
        }

        //if returns false, T& transport is unmodified.
        [[nodiscard]] bool TryPop(T& transport)
        { 
            if (Size() == 0)
                return false;

            transport = Pop();
            return true;
        }

        Iterator Begin() const
        {
            return Iterator(this, first);
        }

        Iterator End() const
        {
            return Iterator(this, BufferLength);
        }
    };
}
