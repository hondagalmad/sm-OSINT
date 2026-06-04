#ifndef LEVENSHTEIN_HPP
#define LEVENSHTEIN_HPP

#include <bits/stdc++.h>

class LCS
{
public:
    static int distance(const std::string &a, const std::string &b)
    {
        int n = a.size();
        int m = b.size();

        std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1));

        for (int i = 0; i <= n; i++)
            dp[i][0] = i;

        for (int j = 0; j <= m; j++)
            dp[0][j] = j;

        for (int i = 1; i <= n; i++)
        {
            for (int j = 1; j <= m; j++)
            {
                if (a[i - 1] == b[j - 1])
                {
                    dp[i][j] = dp[i - 1][j - 1];
                }
                else
                {
                    dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
                }
            }
        }

        return dp[n][m];
    }
};

#endif
