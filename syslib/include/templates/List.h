#pragma once

#define SYSLIB_LIST_NOT_FOUND 0xFFFF'FFFF'FFFF'FFFF

namespace Syslib
{
    //Variable length array, similar to std::vector or C#'s List<T>
    template <typename Value>
    class List
    {
    private:
        Value* buffer;
        unsigned long bufferSize;
        unsigned long count;

    public:
        List()
        {
            count = 0;
            bufferSize = 0;
            buffer = nullptr;
        }

        List(unsigned int reserveFor) : List()
        {
            bufferSize = reserveFor;
            buffer = new Value[bufferSize];
        }

        ~List()
        {
            if (bufferSize > 0)
                delete[] buffer;
        }

        void EnsureBigEnough()
        {
            if (count >= bufferSize)
            {
                //allocate some more space
                unsigned long newReservedCount = count + (count * 2); //reserve an extra 50% each time
                Value* newSpace = new Value[newReservedCount];
                
                //copy existing data across, then free current buffer, and swap pointers
                for (int i = 0; i < count; i++)
                    newSpace[i] = buffer[i]; 
                
                delete[] buffer;
                buffer = newSpace;
                bufferSize = newReservedCount;
            }
        }

        unsigned long Size()
        {
            return count;
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
            EnsureBigEnough();

            count++;
            buffer[count] = val;
        }

        Value PopBack()
        {
            if (count < 1)
                return Value();
            
            count--;
            return buffer[count];
        }

        void InsertAt(unsigned long index, const Value& val)
        {}

        void RemoveAt(unsigned long index)
        {}

        void Remove(const Value& val)
        {
            unsigned long index = Find(val);
            if (index == SYSLIB_LIST_NOT_FOUND)
                return;
            
            RemoveAt(index);
        }

        unsigned long Find(const Value& val)
        {}

        unsigned long FindR(const Value& val)
        {}

        Value operator[](unsigned long index)
        {
            if (index < count)
                return buffer[index];
            return Value();
        }
    };
}