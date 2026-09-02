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

    void generator(std::string &s, std::string curr, int idx, std::vector<std::string> &res)
    {
        if (idx == s.size())
        {
            res.push_back(curr);
            return;
        }

        char c = s[idx];
        if (c >= 'a' && c <= 'z')
        {
            generator(s, curr + c, idx + 1, res);
            generator(s, curr + char(c - 'a' + 'A'), idx + 1, res);
        }
        else if (c == '.')
        {
            for (char r : {'.', '-', '_', '/'})
            {
                generator(s, curr + r, idx + 1, res);
            }
        }
        else if (c == '@')
        {
            for (char r : {'!', '@', '#', '$'})
            {
                generator(s, curr + r, idx + 1, res);
            }
        }
        else if (c == '&')
        {
            for (char r : {'%', '^', '&', '*'})
            {
                generator(s, curr + r, idx + 1, res);
            }
        }
        else
        {
            generator(s, curr + c, idx + 1, res);
        }
    }

    std::vector<std::string> deNormalization(std::string str)
    {
        std::vector<std::string> res;
        generator(str, "", 0, res);
        return res;
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
