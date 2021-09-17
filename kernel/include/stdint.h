#pragma once

//This is a hack to get around gcc having weird rules with a freestanding compiler.
//It won't include its own stdint, even if I force it too from compile time options.
//This relies on our header taking precedence in the include path, over the provided one.
//Since it's local to the source tree, it should always happen.
#include <stdint-gcc.h>
