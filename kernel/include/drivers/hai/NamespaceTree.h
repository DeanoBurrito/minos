#pragma once

#include <String.h>
#include <templates/List.h>

namespace Kernel::Drivers::Hai
{
    struct NamespaceNode
    {
        const string name;
        NamespaceNode* parent;
        sl::List<NamespaceNode*> children;
        
        NamespaceNode(string nName, NamespaceNode* nParent);
    };
}
