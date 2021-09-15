#pragma once
//---- PLATFORM AND FEATURES ----

//Platform selection: set MINOS_PLATFORM to the desired target
#define MINOS_PLATFORM_NONE 0
#define MINOS_PLATFORM_X86_64 1
//#define MINOS_PLATFORM MINOS_PLATFORM_NONE

//Platform and feature macros
#define PLATFORM_REQUIRED(x) static_assert(x == MINOS_PLATFORM, "Required platform not specified: ##x");
#define FEATURE_REQUIRED(x) static_assert(MINOS_FEATURE_##x, "Missing feature for compilation: ##x");

//---- COMPILER ----

//Compiler selection: 
//These adjust how attribute macros resolve (current only GCC/Clang support, which both use GCC-style)
#define MINOS_COMPILER_ATTRIBS_USE_GCC_STYLE
#define MINOS_COMPILER_USE_GCC_STDINT

//Attribute implementations
#if defined(MINOS_COMPILER_ATTRIBS_USE_GCC_STYLE)
    #define ATTRIB_PACKED __attribute__((packed))
    #define ATTRIB_INTERRUPT __attribute__((interrupt))
    #define ATTRIB_ALIGNED(x) __attribute__((aligned(x)))
#else
    #error "kernel/Platform.h has no compiler selected, or build system is not overriding to a proper value."
#endif
