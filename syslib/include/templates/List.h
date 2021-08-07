#pragma once

#include <stddef.h>

namespace Syslib
{
    //Variable length array, similar to std::vector or C#'s List<T>
    template <typename Value>
    class List
    {
    private:
        Value* buffer;
        size_t capacity;
        size_t count;

        void ExpandBuffer()
        {
            size_t newCapacity = capacity + (capacity / 2);
            Value* newBuffer = new Value[newCapacity]; //reserve 50% more space each time
            for (int i = 0; i < count; i++)
                newBuffer[i] = buffer[i];
            
            delete[] buffer;
            buffer = newBuffer;
            capacity = newCapacity;
        }

    public:
        List()
        {
            count = 0;
            capacity = 0;
            buffer = nullptr;
        }

        List(size_t reserveFor) : List()
        {
            capacity = reserveFor;
            buffer = new Value[capacity];
        }

        ~List()
        {
            if (buffer != nullptr)
                delete[] buffer;
        }

        size_t Size()
        {
            return count;
        }

        size_t Capacity()
        {
            return capacity;
        }

        Value First()
        {
            if (count > 0)
                return buffer[0];
            return Value();
        }

        Value Last()
        {
            if (count > 0)
                return buffer[count - 1];
            return Value();
        }

        void Clear()
        {
            count = 0;
        }

        void PushBack(const Value& val)
        {
            if (count == capacity)
                ExpandBuffer();

            buffer[count] = val;
            count++;
        }

        Value PopBack()
        {
            if (count < 1)
                return Value();
            
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
                    ExpandBuffer();
                
                //copy existing items over by 1
                for (int i = count; i > index; i--)
                    buffer[i] = buffer[i - 1];
                buffer[index] = val;
            }
        }

        void RemoveAt(size_t index)
        {
            if (index >= count)
                return;
            
            for (int i = index; i < count - 1; i++)
                buffer[i] = buffer[i + 1];
            count--;
        }

        size_t Find(const Value& val)
        {}

        size_t FindR(const Value& val)
        {}

        Value operator[](size_t index)
        {
            if (index < count)
                return buffer[index];
            return Value();
        }
    };
}