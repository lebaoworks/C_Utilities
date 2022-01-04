#pragma once

#include <windows.h>
#include <type_traits>

class ProcPtr {
public:
    explicit ProcPtr(FARPROC ptr) : _ptr(ptr) {}

    template <typename T>
    operator T* () const {
        return reinterpret_cast<T*>(_ptr);
    }

private:
    FARPROC _ptr;
};

class DllHelper {
public:
    explicit DllHelper(LPCTSTR filename) : _module(LoadLibrary(filename)) {}

    ~DllHelper() { FreeLibrary(_module); }

    ProcPtr operator[](LPCSTR proc_name) const {
        return ProcPtr(GetProcAddress(_module, proc_name));
    }

    static HMODULE _parent_module;

private:
    HMODULE _module;
};