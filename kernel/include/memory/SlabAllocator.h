#pragma once

#include <stddef.h>
#include <stdint.h>
#include <memory/MemoryUsage.h>
#include <Memory.h>
#include <Bitmap.h>

namespace Kernel::Memory
{
    template <typename Value>
    class SlabAllocator
    {
    private:
        const bool canExpand;
        size_t capacity;
        Value* buffer;
        Bitmap bitmap;
        size_t nextAllocStart;

        void Expand()
        {}

    public:
        SlabAllocator(size_t nCapacity, bool nCanExpand) : canExpand(nCanExpand)
        {
            capacity = nCapacity;
            buffer = new Value[capacity];
            bitmap = Bitmap((sizeof(Value) * capacity) / 8);
            nextAllocStart = 0;
        }

        ~SlabAllocator()
        {
            if (buffer)
                delete[] buffer;
        }
        
        SlabAllocator(const SlabAllocator& other) : canExpand(other.canExpand)
        {
            capacity = other.capacity;
            buffer = new Value[capacity];
            sl::memcopy(other.buffer, buffer, sizeof(Value) * capacity);
            bitmap = other.bitmap;
            nextAllocStart = other.nextAllocStart;
        }

        SlabAllocator& operator=(const SlabAllocator& other)
        {
            if (buffer)
                delete[] buffer;
            
            capacity = other.capacity;
            buffer = new Value[capacity];
            sl::memcopy(other.buffer, buffer, sizeof(Value) * capacity);
            bitmap = other.bitmap;
            nextAllocStart = other.nextAllocStart;

            return *this;
        }

        SlabAllocator(SlabAllocator&& from) : canExpand(from.canExpand)
        {
            sl::swap(from.capacity, capacity);
            sl::swap(from.buffer, buffer);
            sl::swap(from.bitmap, bitmap);
            sl::swap(from.nextAllocStart, nextAllocStart);
        }

        SlabAllocator& operator=(SlabAllocator&& from)
        {
            if (buffer)
                delete[] buffer;
            
            sl::swap(from.capacity, capacity);
            sl::swap(from.buffer, buffer);
            sl::swap(from.bitmap, bitmap);
            sl::swap(from.nextAllocStart, nextAllocStart);

            return *this;
        }

        Value* AllocSingle()
        {
            return AllocMany(1);
        }

        Value* AllocMany(size_t count)
        {
            for (size_t i = nextAllocStart; i < capacity - count; i++)
            {
                //check if count-bits are free
                bool valid = true;
                for (size_t c = 0; c < count; c++)
                {
                    if (bitmap.Get(c))
                        valid = false;
                }

                if (!valid)
                    continue;

                //found a size big enough
                for (size_t c = 0; c < count; c++)
                    bitmap.Set(i + c, true);
                
                nextAllocStart = i + count;
                return buffer[i];
            }

            //try from the beginning (we dont substract count from next alloc because there might be a space big enough, but its before + after nextAlloc hint)
            for (size_t i = 0; i < nextAllocStart; i++)
            {
                bool valid = true;
                for (size_t c = 0; c < count; c++)
                {
                    if (bitmap.Get(c))
                        valid = false;
                }

                if (!valid)
                    continue;

                for (size_t c = 0; c < count; c++)
                    bitmap.Set(i + c, true);
                
                nextAllocStart = i + count;
                return buffer[i];
            }

            nextAllocStart = capacity;

            //expand or die
            if (canExpand)
                Expand();
            else
                return nullptr;

            //can only recurse a single time, as its gauranteed to return a valid value after we expanded.
            return AllocMany(count);
        }

        void FreeSingle(Value* addr)
        {
            FreeMany(addr, 1);
        }

        void FreeMany(Value* addr, size_t count)
        {
            size_t startingAddr = (size_t)((void*)addr);
            size_t startingIndex = startingAddr - (size_t)((void*)buffer);
            
            for (size_t i = startingIndex; i < startingIndex + count; i++)
                bitmap.Set(i, false);
        }

        MemoryUsage GetUsage()
        {
            size_t used = 0;
            for (size_t i = 0; i < capacity; i++)
            {
                if (bitmap.Get(i))
                    used++;
            }

            size_t sizeofValue = sizeof(Value);
            return MemoryUsage(capacity * sizeofValue, (capacity - used) * sizeofValue, 0, used * sizeofValue);
        }
    };
}