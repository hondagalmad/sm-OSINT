#ifndef TRIE_H
#define TRIE_H
#include "Trie_Node.hpp"
#include <string>
#include <vector>

class TRIE
{
private:
    TRIENODE *root = new TRIENODE();

    static int getIndex(char c)
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

    static char getChar(int idx)
    {
        if (idx >= 0 && idx < 26)
            return static_cast<char>('a' + idx);
        if (idx >= 26 && idx < 36)
            return static_cast<char>('0' + idx - 26);
        if (idx == 36)
            return '_';
        if (idx == 37)
            return '.';
        return '\0';
    }

    TRIENODE *findNode(const std::string &prefix) const
    {
        TRIENODE *curr = root;

        for (char c : prefix)
        {
            int idx = getIndex(c);
            if (idx == -1 || curr->children[idx] == NULL)
                return NULL;
            curr = curr->children[idx];
        }

        return curr;
    }

    void collectSuggestions(TRIENODE *node, std::string &current, std::vector<std::string> &results, int limit) const
    {
        if (node == NULL || static_cast<int>(results.size()) >= limit)
            return;

        if (node->end > 0)
            results.push_back(current);

        if (static_cast<int>(results.size()) >= limit)
            return;

        for (int i = 0; i < 38; i++)
        {
            if (node->children[i] == NULL)
                continue;

            char next = getChar(i);
            if (next == '\0')
                continue;

            current.push_back(next);
            collectSuggestions(node->children[i], current, results, limit);
            current.pop_back();

            if (static_cast<int>(results.size()) >= limit)
                return;
        }
    }

public:
    TRIE() = default;
    TRIE(const TRIE &) = delete;
    TRIE &operator=(const TRIE &) = delete;

    ~TRIE()
    {
        delete root;
    }

    void insert(const std::string &word)
    {
        TRIENODE *CURR = root;

        for (char c : word)
        {
            int idx = getIndex(c);
            if (idx == -1)
                continue;

            if (CURR->children[idx] == NULL)
            {
                CURR->children[idx] = new TRIENODE();
            }
            CURR = CURR->children[idx];
            CURR->pfx++;
        }
        CURR->end++;
    }

    int countPrefix(const std::string &prefix) const
    {
        TRIENODE *node = findNode(prefix);
        return node == NULL ? 0 : node->pfx;
    }

    int countExact(const std::string &word) const
    {
        TRIENODE *node = findNode(word);
        return node == NULL ? 0 : node->end;
    }

    bool contains(const std::string &word) const
    {
        return countExact(word) > 0;
    }

    std::vector<std::string> suggest(const std::string &prefix, int limit = 10) const
    {
        std::vector<std::string> results;
        if (limit <= 0)
            return results;

        TRIENODE *node = findNode(prefix);
        if (node == NULL)
            return results;

        std::string current = prefix;
        collectSuggestions(node, current, results, limit);
        return results;
    }

    int count(const std::string &word)
    {
        TRIENODE *CURR = root;
        for (char c : word)
        {
            int idx = getIndex(c);
            if (idx == -1 || CURR->children[idx] == NULL)
            {
                return 0;
            }
            CURR = CURR->children[idx];
        }
        return CURR->pfx;
    }
};
#endif
