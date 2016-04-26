// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fsyntax-only -fintel-compatibility -O0 -verify -std=gnu99 %s
// expected-no-diagnostics

__CHAR16_TYPE__ b1[] = u"a";
__CHAR16_TYPE__ b2 = u'\x33';
char b3[] = u8"a";

__CHAR32_TYPE__ c1[] = U"a";
__CHAR32_TYPE__ c2 = U'\x444';

