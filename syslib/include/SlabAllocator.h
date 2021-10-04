#pragma once

#include <stddef.h>
#include <collections/Bitmap.h>
#include <Memory.h>

namespace sl
{
    template <typename SlabItem>
    class SlabAllocator
    {
    private:
        SlabItem* buffer;
        size_t bufferLength; //TODO: self-expanding slabs - as these will innevitably saturate
        Bitmap bitmap;

    public:
        SlabAllocator(size_t count)
        {
            buffer = new SlabItem[count];
            sl::memset(buffer, 0, sizeof(SlabItem) * count);
            bitmap.Resize(count);
            bufferLength = count;
        }

        ~SlabAllocator()
        {
            if (buffer)
                delete[] buffer;
        }
        
        SlabAllocator(const SlabAllocator& other) = delete; //non-copyable
        SlabAllocator& operator=(const SlabAllocator& other) = delete;

        SlabAllocator(SlabAllocator&& from)
        {
            buffer = from.buffer;
            bufferLength = from.bufferLength;
            bitmap = sl::move(from.bitmap);

            from.buffer = nullptr;
            from.bufferLength = 0;
        }

        SlabAllocator& operator=(SlabAllocator&& from)
        {
            if (buffer)
                delete[] buffer;
            
            buffer = from.buffer;
            bufferLength = from.bufferLength;
            bitmap = sl::move(from.bitmap);

            from.buffer = nullptr;
            from.bufferLength = 0;
        
            return *this;
        }

        bool TryAlloc(void** ptr, const size_t size)
        {
            if (size != sizeof(SlabItem))
                return false; //size mismatch
            
            for (size_t i = 0; i < bufferLength; i++)
            {
                if (!bitmap.Get(i))
                {
                    bitmap.Set(i);
                    *ptr = &buffer[i];
                    return true;
                }
            }

            return false; //could not find free space
        }

        bool TryFree(const void* const ptr)
        {
            uint64_t addr = (uint64_t)ptr - (uint64_t)buffer; //get address of byte offset relative to buffer start (overflow will wrap around)
            if (addr > (uint64_t)buffer + (bufferLength * sizeof(SlabItem)))
                return false; //address is outside of our scope

            addr = addr / sizeof(SlabItem);
            bitmap.Clear(addr); //no need to zero it, just clear the flag.
            return true;
        }

        SlabItem* Alloc()
        {
            SlabItem* ptr = nullptr;
            TryAlloc(reinterpret_cast<void**>(&ptr), sizeof(SlabItem));
            return ptr;
        }

        void Free(SlabItem* item)
        {
            TryFree(item);
        }
    };
}
