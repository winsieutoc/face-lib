#include "vwscapture/util.h"
#include <algorithm>
#include <string>
#include <iostream>
#include <cctype>

Util::Util()
{}

void Util::print_hex_memory(void *mem, int pos, int n)
{
    int i;
    unsigned char *p = (unsigned char *)mem;
    for (i=pos;i<n;i++) {
        // printf("0x%02x ", p[i] & 0xff);
        printf("%d ", p[i] & 0xff);
        if (((i-pos)%16==0) && (i!=pos))
            printf("\n");
    }
    printf("\n");
}

void Util::exportVectorToFile(string url, vector<char> v)
{
    std::ofstream outfile;
    outfile.open(url, std::ios_base::app);
    for (const auto &e : v) outfile << e;
}

string Util::delSpaces(string str)
{
    // remove_if(str.begin(), str.end(), std::isspace);
    auto f = [](unsigned char const c) { return std::isspace(c); };
    str.erase(std::remove_if(str.begin(), str.end(), f), str.end());
    return str;
}

std::vector<string> Util::split(const string &s, char delim)
{
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
