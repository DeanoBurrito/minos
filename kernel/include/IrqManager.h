#pragma once

#include <stddef.h>
#include <collections/List.h>
#include <multiprocessing/Scheduler.h>
#include <Platform.h>

#define IRQ_VECTOR_NONE 0

namespace Kernel
{
    typedef size_t IrqVector;
    typedef void (*IrqHandler)(IrqVector);
    
    //Responsible for handling the platform specific parts of setting up interrupts,
    //and helps manage complex routing
    class IrqManager
    {
    private:
        //TODO: hashtable with no key for exclusive, or list of void* handlers for shared

    public:
        static IrqManager* The();

        void Init();
        //Makes this IrqManager the current set of interrupts
        void Load();

        //called from the general interrupt stub
        void SharedIrqDispatch(MinosInterruptFrame* frame);

        //returns true if a vector number is reserved for the platform
        bool IsArchitectural(IrqVector vector);
        //returns if vector number is available to assign
        bool IsAvailable(IrqVector vector);

        //gets a free vector number
        IrqVector GetFree();
        //attempts to get a free vector number from a given list
        IrqVector GetFreeFrom(sl::List<IrqVector> vectors);

        //returns if operation was successful or not
        bool AssignVector(IrqVector vector, void* handler, bool exclusive = true);
        //if a handler is already installed, this will evict it
        void ClearVector(IrqVector vector);

        //prints out a nice table for my viewing pleasure :)
        void PrintVectorTable();

        //enables interrupts to use this regardless of which PIC is being used.
        void SendEOI();
    };
}
