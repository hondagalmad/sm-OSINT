#ifndef NORMALIZATION_HPP
#define NORMALIZATION_HPP
#include <bits/stdc++.h>

class Normalization
{
private:
    std::string initial;

public:
    Normalization() = default;
    Normalization(std::string initial)
    {
        this->initial = initial;
        for (int i = 0; i < initial.length(); i++)
        {
            if ((initial[i] >= '0' && initial[i] <= '9') || (initial[i] >= 'a' && initial[i] <= 'z'))
                continue;

            if (initial[i] >= 'A' && initial[i] <= 'Z')
            {
                initial[i] = (initial[i] - 'A' + 'a');
            }
            else if (initial[i] == '.' || initial[i] == '-' || initial[i] == '_' || initial[i] == '/')
            {
                initial[i] = '.';
            }
            else if (initial[i] == '!' || initial[i] == '@' || initial[i] == '#' || initial[i] == '$')
            {
                initial[i] = '@';
            }
            else if (initial[i] == '%' || initial[i] == '^' || initial[i] == '&' || initial[i] == '*')
            {
                initial[i] = '&';
            }
        }
    };

    std::string get() { return this->initial; }
    ~Normalization() {}
};

#endif
