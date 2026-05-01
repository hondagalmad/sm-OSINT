#ifndef SM_OSINT_NODE_H
#define SM_OSINT_NODE_H
#include <cstring>
class TRIENODE {
public:
    TRIENODE* children[26];
    int pfx, end;
    TRIENODE() {
        pfx = 0;
        end = 0;
        memset(children, 0, sizeof(children));
    }
    ~TRIENODE() {
        for (int i=0; i<26; i++) {
            if (children[i]) delete children[i];
        }
    }

};
#endif