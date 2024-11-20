#include <iostream>
using namespace std;

bool match_location(string &conf_location, string uri)
{
    size_t i = 0;
    while (i < conf_location.length() && i < uri.length() && conf_location[i] == uri[i])
        i++;
    return (i == conf_location.length() || i == conf_location.length() - 1 && i == uri.length());
}

int main()
{
}