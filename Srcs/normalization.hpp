#ifndef NORMALIZATION_HPP
#define NORMALIZATION_HPP
#include <bits/stdc++.h>
class Normalization {
private:
    string initial;
public:
    Normalization(string initial) {
        this->initial = initial;
        for (int i=0; i<initial.length(); i++) {
            if (initial[i]>='0' && initial[i]<='9') continue;
            if (initial[i]=='.'||initial[i]=='-'||initial[i]=='_'||initial[i]=='/')initial[i]='.';
            if (initial[i]=='!'||initial[i]=='@'||initial[i]=='#'||initial[i]=='$')initial[i]='@';
            if (initial[i]=='%'||initial[i]=='^'||initial[i]=='&'||initial[i]=='*')initial='&';
            if (initial[i]>='a'&&initial[i]<='z')continue;
            if (initial[i]>='A'&&initial[i]<='Z')initial[i]=(initial[i]-'A'+'a');

        }
    };
    Normalization();
};
#endif
