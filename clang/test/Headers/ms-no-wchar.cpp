// INTEL RUN: %clang_cc1 -fsyntax-only -triple x86_64-pc-windows-msvc \
// INTEL RUN: -fms-compatibility-version=17.00 -fno-wchar \
// INTEL RUN: -isystem %S/Inputs/intel %s
// MSVC defines wchar_t instead of using the builtin if /Zc:wchar_t- is passed

#include <stddef.h>

wchar_t c;
