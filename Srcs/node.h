#ifndef SM_OSINT_NODE_H
#define SM_OSINT_NODE_H
#include <cstring>

class TRIENODE
{
public:
    TRIENODE *children[40];
    int pfx, end;

    TRIENODE()
    {
        pfx = 0;
        end = 0;
        memset(children, 0, sizeof(children));
    }

    ~TRIENODE() // memory deallocation
    {
        for (int i = 0; i < 40; i++)
        {
            if (children[i])
                delete children[i];
        }
    }
};
#endif