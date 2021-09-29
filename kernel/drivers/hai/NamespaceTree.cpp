#include <drivers/hai/NamespaceTree.h>

namespace Kernel::Drivers::Hai
{
    NamespaceNode::NamespaceNode(string nName, NamespaceNode* nParent) : name(nName)
    {
        parent = nParent;
    }
}
