#ifndef TRIE_H
#define TRIE_H
#include "node.h"
#include <string>
class TRIE
{
private:
    TRIENODE *root = new TRIENODE();
    int getIndex(char c)
    {
        if (c >= 'a' && c <= 'z')
            return c - 'a';
        if (c >= '0' && c <= '9')
            return 26 + (c - '0');
        if (c == '_')
            return 36;
        if (c == '.')
            return 37;
        return -1;
    }

public:
    void insert(const std::string &word)
    {
        TRIENODE *CURR = root;

        for (char c : word)
        {
            int idx = getIndex(c);
            if (CURR->children[idx] == NULL)
            {
                CURR->children[idx] = new TRIENODE();
            }
            CURR = CURR->children[idx];
            CURR->pfx++;
        }
        CURR->end++;
    }
};
#endif