#ifndef USERNAME_PROBABILITY_HPP
#define USERNAME_PROBABILITY_HPP
#include <bits/stdc.h>
class UsernameProbability
{
public:
    struct Score
    {
        double pattern = 0.0;
        double separators = 0.0;
        double tokenLayout = 0.0;
        double charTransitions = 0.0;
        double combined = 0.0;
    };

    void feed(const std::string &normalized)
    {
        if (normalized.empty())
            return;
        totalSamples++;
        const std::string pattern = buildPattern(normalized);
        const std::string separatorPattern = buildSeparatorPattern(normalized);
        const std::string tokenLayout = buildTokenLayout(normalized);
        patternCount[pattern]++;
        separatorPatternCount[separatorPattern]++;
        tokenLayoutCount[tokenLayout]++;
        lengthCount[normalized.length()]++;
        trainTransitions(normalized);
    }
    void feedAll(const std::vector<std::string> &normalizedStrings)
    {
        for (const std::string &normalized : normalizedStrings)
            feed(normalized);
    }

    int samples() const
    {
        return totalSamples;
    }

    double patternProbability(const std::string &normalized) const
    {
        return smoothedProbability(patternCount, buildPattern(normalized));
    }

    double separatorProbability(const std::string &normalized) const
    {
        return smoothedProbability(separatorPatternCount, buildSeparatorPattern(normalized));
    }

    double tokenLayoutProbability(const std::string &normalized) const
    {
        return smoothedProbability(tokenLayoutCount, buildTokenLayout(normalized));
    }

    double lengthProbability(const std::string &normalized) const
    {
        if (totalSamples == 0)
            return 0.0;

        const std::size_t length = normalized.length();
        const int kinds = std::max<int>(1, lengthCount.size() + 1);
        auto it = lengthCount.find(length);
        const int count = (it == lengthCount.end()) ? 0 : it->second;

        return static_cast<double>(count + 1) / static_cast<double>(totalSamples + kinds);
    }

    double transitionProbability(const std::string &normalized) const
    {
        if (normalized.empty() || totalTransitions == 0)
            return 0.0;

        double logScore = 0.0;
        char previous = '^';

        for (char current : normalized)
        {
            logScore += std::log(smoothedTransitionProbability(previous, current));
            previous = current;
        }

        logScore += std::log(smoothedTransitionProbability(previous, '$'));

        const double steps = static_cast<double>(normalized.length() + 1);
        return std::exp(logScore / steps);
    }

    Score score(const std::string &normalized) const
    {
        Score result;

        result.pattern = patternProbability(normalized);
        result.separators = separatorProbability(normalized);
        result.tokenLayout = tokenLayoutProbability(normalized);
        result.charTransitions = transitionProbability(normalized);

        if (totalSamples == 0)
            return result;

        const double length = lengthProbability(normalized);

        result.combined = geometricMean({
            result.pattern,
            result.separators,
            result.tokenLayout,
            result.charTransitions,
            length,
        });

        return result;
    }

    std::vector<std::pair<std::string, double>> topPatterns(int limit = 10) const
    {
        return topProbabilities(patternCount, limit);
    }

    std::vector<std::pair<std::string, double>> topSeparatorPatterns(int limit = 10) const
    {
        return topProbabilities(separatorPatternCount, limit);
    }

    std::vector<std::pair<std::string, double>> topTokenLayouts(int limit = 10) const
    {
        return topProbabilities(tokenLayoutCount, limit);
    }

    static std::string buildPattern(const std::string &normalized)
    {
        std::string pattern;
        pattern.reserve(normalized.length());

        for (char c : normalized)
        {
            if (isLetter(c))
                pattern.push_back('a');
            else if (isDigit(c))
                pattern.push_back('0');
            else if (isSeparator(c))
                pattern.push_back(c);
            else
                pattern.push_back('?');
        }

        return compressRuns(pattern);
    }

    static std::string buildSeparatorPattern(const std::string &normalized)
    {
        std::string pattern;

        for (char c : normalized)
        {
            if (isSeparator(c))
                pattern.push_back(c);
        }

        return pattern.empty() ? "none" : pattern;
    }

    static std::string buildTokenLayout(const std::string &normalized)
    {
        std::vector<std::string> tokens = splitBySeparators(normalized);
        if (tokens.empty())
            return "empty";

        std::string layout;

        for (std::size_t i = 0; i < tokens.size(); i++)
        {
            if (i > 0)
                layout += "+";

            layout += tokenKind(tokens[i]);
        }

        return layout;
    }

private:
    int totalSamples = 0;
    int totalTransitions = 0;

    std::unordered_map<std::string, int> patternCount;
    std::unordered_map<std::string, int> separatorPatternCount;
    std::unordered_map<std::string, int> tokenLayoutCount;
    std::map<std::size_t, int> lengthCount;
    std::unordered_map<char, std::unordered_map<char, int>> transitionCount;
    std::unordered_map<char, int> outgoingCount;

    static bool isLetter(char c)
    {
        return c >= 'a' && c <= 'z';
    }

    static bool isDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    static bool isSeparator(char c)
    {
        return c == '_' || c == '.';
    }

    static std::string compressRuns(const std::string &str)
    {
        if (str.empty())
            return "";

        std::string compressed;
        char current = str[0];
        int count = 1;

        for (std::size_t i = 1; i < str.length(); i++)
        {
            if (str[i] == current)
            {
                count++;
            }
            else
            {
                appendRun(compressed, current, count);
                current = str[i];
                count = 1;
            }
        }

        appendRun(compressed, current, count);
        return compressed;
    }

    static void appendRun(std::string &out, char kind, int count)
    {
        out.push_back(kind);
        out += std::to_string(count);
    }

    static std::vector<std::string> splitBySeparators(const std::string &normalized)
    {
        std::vector<std::string> tokens;
        std::string current;

        for (char c : normalized)
        {
            if (isSeparator(c))
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

    static std::string tokenKind(const std::string &token)
    {
        bool hasLetter = false;
        bool hasDigit = false;
        bool hasOther = false;

        for (char c : token)
        {
            hasLetter = hasLetter || isLetter(c);
            hasDigit = hasDigit || isDigit(c);
            hasOther = hasOther || (!isLetter(c) && !isDigit(c));
        }

        if (hasOther)
            return "mixed";
        if (hasLetter && hasDigit)
            return "wordnum";
        if (hasLetter)
            return "word";
        if (hasDigit)
            return "num";
        return "empty";
    }

    void trainTransitions(const std::string &normalized)
    {
        char previous = '^';

        for (char current : normalized)
        {
            transitionCount[previous][current]++;
            outgoingCount[previous]++;
            totalTransitions++;
            previous = current;
        }

        transitionCount[previous]['$']++;
        outgoingCount[previous]++;
        totalTransitions++;
    }

    double smoothedProbability(const std::unordered_map<std::string, int> &counts, const std::string &key) const
    {
        if (totalSamples == 0)
            return 0.0;

        const int kinds = std::max<int>(1, counts.size() + 1);
        auto it = counts.find(key);
        const int count = (it == counts.end()) ? 0 : it->second;

        return static_cast<double>(count + 1) / static_cast<double>(totalSamples + kinds);
    }

    double smoothedTransitionProbability(char previous, char current) const
    {
        const int alphabetSize = 40;
        int count = 0;
        int outgoing = 0;

        auto row = transitionCount.find(previous);
        if (row != transitionCount.end())
        {
            auto cell = row->second.find(current);
            if (cell != row->second.end())
                count = cell->second;
        }
        auto out = outgoingCount.find(previous);
        if (out != outgoingCount.end())
            outgoing = out->second;
        return static_cast<double>(count + 1) / static_cast<double>(outgoing + alphabetSize);
    }
    static double geometricMean(const std::vector<double> &values)
    {
        double logSum = 0.0;
        int used = 0;
        for (double value : values)
        {
            if (value <= 0.0)
                continue;
            logSum += std::log(value);
            used++;
        }
        if (used == 0)
            return 0.0;
        return std::exp(logSum / static_cast<double>(used));
    }
    std::vector<std::pair<std::string, double>> topProbabilities(const std::unordered_map<std::string, int> &counts, int limit) const
    {
        std::vector<std::pair<std::string, double>> result;
        if (totalSamples == 0 || limit <= 0)
            return result;
        for (const auto &entry : counts)
            result.push_back({entry.first, static_cast<double>(entry.second) / totalSamples});
        std::sort(result.begin(), result.end(),
                  [](const auto &a, const auto &b)
                  {
                      if (a.second == b.second)
                          return a.first < b.first;
                      return a.second > b.second;
                  });
        if (static_cast<int>(result.size()) > limit)
            result.resize(limit);
        return result;
    }
};
#endif
