#pragma once

namespace Kernel::Drivers::Hai
{
    class Interpreter
    {
    public:
        static Interpreter* Create();

    private:
        Interpreter();
        ~Interpreter() = delete;

    public:
        Interpreter(const Interpreter& other) = delete;
        Interpreter& operator=(const Interpreter& other) = delete;
        Interpreter(Interpreter&& from) = delete;
        Interpreter& operator=(Interpreter&& from) = delete;
    };
}

/*
    Implementation ideas:
        -It looks like we can AML bytecode, ASL and TDL source mixed together?
        -Meaning we'll need to be able to parse all of these.
            -I'm thinking 3 separate front ends that translate to a custom AST.
        -AST should be constructed in a low-level manner, easy to flatten to an instruction stream.

        -This would be easy for AML, but would require full transformation on ASL and TDL.
        -Upon re-reading, it appears ASL compiles to AML. TDL is for generating data tables,
            and is not encountered by the user (our os).
            -So we'd just an AML interpreter.
*/
