#pragma once

#include <Windows.h>

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWCH   Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING* PUNICODE_STRING;

typedef struct _STRING32 {
    USHORT   Length;
    USHORT   MaximumLength;
    ULONG  Buffer;
} STRING32;

typedef STRING32 UNICODE_STRING32;
typedef UNICODE_STRING32* PUNICODE_STRING32;

typedef STRING32 ANSI_STRING32;
typedef ANSI_STRING32* PANSI_STRING32;


typedef struct _STRING64 {
    USHORT   Length;
    USHORT   MaximumLength;
    ULONGLONG  Buffer;
} STRING64;
typedef STRING64* PSTRING64;

typedef STRING64 UNICODE_STRING64;
typedef UNICODE_STRING64* PUNICODE_STRING64;