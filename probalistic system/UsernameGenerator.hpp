#ifndef USERNAME_GENERATOR_HPP
#define USERNAME_GENERATOR_HPP

#include "UsernameProbability.hpp"
#include<bits/stdc++.h>

class UsernameGenerator
{
public:
    struct Candidate
    {
        std::string username;
        double score = 0.0;
    };

    std::vector<std::string> generate(const std::string &seed, int limit = 50) const
    {
        std::vector<Candidate> ranked = generateRanked(seed, nullptr, limit);
        std::vector<std::string> result;
        result.reserve(ranked.size());

        for (const Candidate &candidate : ranked)
            result.push_back(candidate.username);

        return result;
    }

    std::vector<Candidate> generateRanked(const std::string &seed, const UsernameProbability *model = nullptr, int limit = 50) const
    {
        std::vector<Candidate> candidates;
        if (limit <= 0)
            return candidates;

        std::set<std::string> unique;
        const std::string normalized = normalize(seed);
        if (normalized.empty())
            return candidates;

        const std::vector<std::string> tokens = splitTokens(normalized);
        const std::string compact = compactUsername(normalized);

        add(unique, normalized);
        add(unique, compact);

        addJoinedTokens(unique, tokens, ".");
        addJoinedTokens(unique, tokens, "_");
        addTokenPairVariants(unique, tokens);
        addAffixVariants(unique, compact);
        addAffixVariants(unique, normalized);

        for (const std::string &candidate : unique)
        {
            if (candidate.empty())
                continue;

            Candidate current;
            current.username = candidate;
            current.score = model == nullptr ? 0.0 : model->score(candidate).combined;
            candidates.push_back(current);
        }

        if (model != nullptr && model->samples() > 0)
        {
            std::sort(candidates.begin(), candidates.end(),
                      [](const Candidate &a, const Candidate &b)
                      {
                          if (a.score == b.score)
                              return a.username < b.username;
                          return a.score > b.score;
                      });
        }

        if (static_cast<int>(candidates.size()) > limit)
            candidates.resize(limit);

        return candidates;
    }

private:
    static std::string normalize(const std::string &input)
    {
        std::string out;
        out.reserve(input.size());

        for (unsigned char raw : input)
        {
            char c = static_cast<char>(std::tolower(raw));

            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
            {
                out.push_back(c);
            }
            else if (c == '.' || c == '_' || c == '-' || c == '/')
            {
                if (!out.empty() && out.back() != '.')
                    out.push_back('.');
            }
        }

        while (!out.empty() && out.back() == '.')
            out.pop_back();

        return out;
    }

    static std::vector<std::string> splitTokens(const std::string &username)
    {
        std::vector<std::string> tokens;
        std::string current;

        for (char c : username)
        {
            if (c == '.' || c == '_')
            {
                if (!current.empty())
                {
                    tokens.push_back(current);
                    current.clear();
                }
            }
            else
            {
                current.push_back(c);
            }
        }

        if (!current.empty())
            tokens.push_back(current);

        return tokens;
    }

    static std::string compactUsername(const std::string &username)
    {
        std::string compact;
        compact.reserve(username.size());

        for (char c : username)
        {
            if (c != '.' && c != '_')
                compact.push_back(c);
        }

        return compact;
    }

    static void add(std::set<std::string> &unique, const std::string &value)
    {
        if (!value.empty())
            unique.insert(value);
    }

    static void addJoinedTokens(std::set<std::string> &unique, const std::vector<std::string> &tokens, const std::string &separator)
    {
        if (tokens.size() < 2)
            return;

        std::string joined;
        for (std::size_t i = 0; i < tokens.size(); i++)
        {
            if (i > 0)
                joined += separator;
            joined += tokens[i];
        }

        add(unique, joined);
    }

    static void addTokenPairVariants(std::set<std::string> &unique, const std::vector<std::string> &tokens)
    {
        if (tokens.size() < 2)
            return;

        const std::string first = tokens.front();
        const std::string last = tokens.back();

        add(unique, first + last);
        add(unique, first + "." + last);
        add(unique, first + "_" + last);
        add(unique, last + first);
        add(unique, last + "." + first);
        add(unique, last + "_" + first);

        if (!first.empty() && !last.empty())
        {
            add(unique, std::string(1, first[0]) + last);
            add(unique, first + std::string(1, last[0]));
            add(unique, std::string(1, first[0]) + "." + last);
            add(unique, first + "." + std::string(1, last[0]));
        }
    }

    static void addAffixVariants(std::set<std::string> &unique, const std::string &base)
    {
        if (base.empty())
            return;

        const std::vector<std::string> prefixes = {"real", "official", "the", "iam"};
        const std::vector<std::string> suffixes = {"dev", "yt", "x", "official", "1", "01", "7", "99", "123"};
        const std::vector<std::string> separators = {"", ".", "_"};

        for (const std::string &prefix : prefixes)
        {
            for (const std::string &separator : separators)
                add(unique, prefix + separator + base);
        }

        for (const std::string &suffix : suffixes)
        {
            for (const std::string &separator : separators)
                add(unique, base + separator + suffix);
        }
    }
};

#endif
