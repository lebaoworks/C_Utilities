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

template<typename TCHAR>
bool replace(std::basic_string<TCHAR>& str, const std::basic_string<TCHAR>& from, const std::basic_string<TCHAR>& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::basic_string<TCHAR>::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}