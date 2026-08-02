#ifndef NORMALIZATION_HPP
#define NORMALIZATION_HPP
#include <bits/stdc++.h>

class Normalizer
{
private:
    std::string initial;

    std::string Normalization(std::string str)
    {
        for (int i = 0; i < str.length(); i++)
        {
            if ((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'a' && str[i] <= 'z'))
                continue;

            if (str[i] >= 'A' && str[i] <= 'Z')
            {
                str[i] = (str[i] - 'A' + 'a');
            }
            else if (str[i] == '.' || str[i] == '-' || str[i] == '_' || str[i] == '/')
            {
                str[i] = '.';
            }
            else if (str[i] == '!' || str[i] == '@' || str[i] == '#' || str[i] == '$')
            {
                str[i] = '@';
            }
            else if (str[i] == '%' || str[i] == '^' || str[i] == '&' || str[i] == '*')
            {
                str[i] = '&';
            }
        }
    }

    std::vector<std::string> deNormalization(std::string str)
    {

    }
     void generator(string &s) {
        
    }
public:
    Normalizer() = default;

    Normalizer(std::string initial)
    {
        this->initial = initial;
    };

    // return the normalized string 
    std::string Normalized()
    {
        return Normalization(initial);
    }

    // return all possible denormalized strings
    std::vector<std::string> deNormalized()
    {
        return deNormalization(initial);
    }

    ~Normalizer() {}
};

#endif
