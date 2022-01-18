#include <Windows.h>

#include <iostream>
#include <sstream>
using namespace std;

#include "dir.h"

int main()
{
    wstring path = LR"(D:\Desktop)";
    vector<wstring> flags{ L"/A:-D", L"/O:N", L"/Q", L"/X"};
    wstringstream out;
    dir(path, flags, out);
    fwrite(out.str().c_str(), 2, out.str().length(), stdout);
    return 0;
}