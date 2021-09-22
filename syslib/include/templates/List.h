#pragma once

#include <stddef.h>
#include <Memory.h>

#define SL_LIST_EXPANSION_FORMULA(currentCap) ((currentCap) + ((currentCap) / 2))

namespace sl
{
    //Variable length array, similar to std::vector or C#'s List<T>
    template <typename Value>
    class List
    {
    private:
        Value defaultValue; //TODO: find a way to return nothing, without this
        Value* buffer;
        size_t capacity;
        size_t count;

    public:
        List()
        {
            count = 0;
            capacity = 0;
            buffer = nullptr;
            defaultValue = Value();
        }

        List(size_t reserveFor) : List()
        {
            capacity = reserveFor;
            buffer = new Value[capacity];
            defaultValue = Value();
        }

        ~List()
        {
            if (buffer != nullptr)
                delete[] buffer;
        }

        List(const List& other)
        {
            buffer = new Value[other.capacity];
            capacity = other.capacity;
            count = other.count;
            memcopy(other.buffer, buffer, count);
        }
        
        List& operator=(const List& other)
        {
            if (buffer)
                delete[] buffer;
            
            buffer = new Value[other.capacity];
            capacity = other.capacity;
            count = other.count;
            memcopy(other.buffer, buffer, count);

            return *this;
        }

        List(List&& from)
        {
            buffer = from.buffer;
            count = from.count;
            capacity = from.capacity;

            from.buffer = nullptr; 
            from.count = from.capacity = 0;
        }

        List& operator=(List&& from)
        {
            if (buffer)
                delete[] buffer;
            
            buffer = from.buffer;
            count = from.count;
            capacity = from.capacity;

            from.buffer = nullptr; 
            from.count = from.capacity = 0;

            return *this;
        }

        size_t Size()
        {
            return count;
        }

        bool Empty()
        {
            return count == 0;
        }

        size_t Capacity()
        {
            return capacity;
        }

        void Reserve(size_t reserveFor)
        {
            if (reserveFor < capacity)
                return;

            if (reserveFor < capacity + 1)
                reserveFor = capacity + 1;
            
            Value* newBuffer = new Value[reserveFor];
            for (size_t i = 0; i < count; i++)
                newBuffer[i] = buffer[i];
            
            delete[] buffer;
            buffer = newBuffer;
            capacity = reserveFor;
        }

        Value& First()
        {
            if (count > 0)
                return buffer[0];
            return defaultValue;
        }

        Value& Last()
        {
            if (count > 0)
                return buffer[count - 1];
            return defaultValue;
        }

        void Clear()
        {
            count = 0;
        }

        void PushBack(const Value& val)
        {
            if (count == capacity)
                Reserve(SL_LIST_EXPANSION_FORMULA(capacity));

            buffer[count] = val;
            count++;
        }

        Value PopBack()
        {
            if (count < 1)
                return defaultValue;
            
            count--;
            return buffer[count];
        }

        void InsertAt(size_t index, const Value& val)
        {
            if (index >= count)
                PushBack(val);
            else
            {
                if (count == capacity)
                    Reserve(SL_LIST_EXPANSION_FORMULA(capacity));
                
                //copy existing items over by 1
                for (size_t i = count; i > index; i--)
                    buffer[i] = buffer[i - 1];
                buffer[index] = val;
            }
        }

        void RemoveAt(size_t index)
        {
            if (index >= count)
                return;
            
            for (size_t i = index; i < count - 1; i++)
                buffer[i] = buffer[i + 1];
            count--;
        }

        size_t Find(const Value& val)
        { return 0; }

        size_t FindR(const Value& val)
        { return 0; }

        Value& operator[](size_t index)
        {
            if (index < count)
                return buffer[index];
            return defaultValue;
        }
    };
}