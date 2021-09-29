#include <drivers/hai/Interpreter.h>
#include <drivers/hai/OpCodes.h>

namespace Kernel::Drivers::Hai
{
    Interpreter* Interpreter::Create()
    {
        return nullptr;
    }

    Interpreter::Interpreter()
    {}

    // Interpreter::~Interpreter()
    // {} Deleted because it realistically should never actually run. Once we assume power/thermal control, firmware is not gauranteed to come back up without a reset.

    // Interpreter(const Interpreter& other)
    // {} Deleted currently, to ensure the compiler isnt being sneaky with any operations. Will enable for prod.

    // Interpreter& operator=(const Interpreter& other)
    // {} Deleted currently, to ensure the compiler isnt being sneaky with any operations. Will enable for prod.

    // Interpreter(Interpreter&& from)
    // {} Deleted currently, to ensure the compiler isnt being sneaky with any operations. Will enable for prod.

    // Interpreter& operator=(Interpreter&& from)
    // {} Deleted currently, to ensure the compiler isnt being sneaky with any operations. Will enable for prod.

}
