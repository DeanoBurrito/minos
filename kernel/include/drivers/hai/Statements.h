#pragma once

#include <stddef.h>
#include <templates/List.h>
#include <drivers/hai/OpCodes.h>

namespace Kernel::Drivers::Hai
{
    enum class StatementType
    {
        Nop,
        Block,

        Name,
    };
    
    struct Statement
    {
        StatementType nodeType;

        virtual void Execute() = 0;
    };

    struct NopStatement : Statement
    {
        NopStatement(__attribute__((unused)) AmlOp nopCode)
        { 
            //NOTE: we dont really need to check input operand, since only the interpreter can load AML bytestreams.
            nodeType = StatementType::Nop;
        }
        
        void Execute() override
        { return; }
    };

    struct BlockStatement : Statement
    {
        sl::List<Statement*> statements;
        
        BlockStatement(sl::List<Statement*> block)
        { 
            statements = block;
            nodeType = StatementType::Block; 
        }

        void Execute() override
        {
            for (size_t i = 0; i < statements.Size(); i++)
                statements[i]->Execute();
        }
    };

    struct NameStatement : Statement
    {
        AmlOp* begin;
        AmlOp* end;

        NameStatement(AmlOp* first, AmlOp* last)
        {
            begin = first;
            end = last++;
        }

        void Execute() override
        {} //TODO: install into local namespace, or into root namespace if begins with root

        bool StartsWithRoot()
        {
            return *begin == AmlOp::RootChar;
        }

        size_t ScopeEscapes()
        {
            size_t count = 0;
            AmlOp* scan = begin;
            while (scan < end && *scan == AmlOp::ParentPrefix)
            {
                count++;
                scan++;
            }

            return count;
        }

        size_t GetNameStart()
        {
            if (StartsWithRoot())
                return 1;
            return ScopeEscapes();
        }

        size_t Length()
        {
            return end - begin - GetNameStart();
        }

        size_t FullLength()
        {
            return end - begin;
        }
    };
}
