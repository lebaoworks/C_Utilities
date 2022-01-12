#pragma once

#include <string>

template<typename TCHAR>
void trim(std::basic_string<TCHAR>& s)
{
    size_t ibegin, iend;
    if constexpr (sizeof(TCHAR) == 1) {
        ibegin = s.find_first_not_of(" \t\r\n");
        iend = s.find_last_not_of(" \t\r\n");
    }
    else {
        ibegin = s.find_first_not_of(L" \t\r\n");
        iend = s.find_last_not_of(L" \t\r\n");
    }
    
    if (iend == -1)
        s = basic_string<TCHAR>();
    else
        s = s.substr(ibegin, iend - ibegin + 1);
}