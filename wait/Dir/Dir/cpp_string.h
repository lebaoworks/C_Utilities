#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <cwchar>

template<typename ... Args>
std::wstring string_format(const std::wstring& format, Args ... args)
{
    size_t size_s = swprintf(nullptr, 0, format.c_str(), args ...) + 1;
    if (size_s <= 0) {
        throw std::runtime_error("Formatting swprintf fail");
    }
    wchar_t* buf = (wchar_t*) calloc(size_s, sizeof(wchar_t));
    if (buf == NULL) {
        throw std::runtime_error("Formatting not enough memory.");
    }
    swprintf(buf, size_s, format.c_str(), args ...);
    std::wstring ret = buf;
    free(buf);
    return ret;
}
