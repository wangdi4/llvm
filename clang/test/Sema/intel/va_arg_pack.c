// RUN: %clang_cc1 -DFULL -fintel-compatibility -fintel-compatibility-disable=AllowExtraArgument -fsyntax-only -verify %s -triple x86_64-pc-win32
// RUN: %clang_cc1 -DFULL -fintel-compatibility -fintel-compatibility-disable=AllowExtraArgument -fsyntax-only -verify %s -triple x86_64-pc-linux
// RUN: %clang_cc1 -DONLY -fintel-compatibility-enable=VaArgPack -fsyntax-only -verify %s -triple x86_64-pc-win32
// RUN: %clang_cc1 -DONLY -fintel-compatibility-enable=VaArgPack -fsyntax-only -verify %s -triple x86_64-pc-linux
// RUN: %clang_cc1 -DDISABLE -fintel-compatibility -fintel-compatibility-disable=VaArgPack -fsyntax-only -verify %s -triple x86_64-pc-win32
// RUN: %clang_cc1 -DDISABLE -fintel-compatibility -fintel-compatibility-disable=VaArgPack -fsyntax-only -verify %s -triple x86_64-pc-linux
// RUN: %clang_cc1 -DOFF -fsyntax-only -verify %s -triple x86_64-pc-win32
// RUN: %clang_cc1 -DOFF -fsyntax-only -verify %s -triple x86_64-pc-linux

void foo(int x, ...);
void bar(int x, int k, int j);

#if defined(FULL) || defined(ONLY)
void errors1(int x, ...) {
  int k = __builtin_va_arg_pack_len(); // expected-error {{invalid use of '__builtin_va_arg_pack_len()', must be in an inlined function that is not externally visible}}
  foo(k, __builtin_va_arg_pack()); // expected-error {{invalid use of '__builtin_va_arg_pack()', must be in an inlined function that is not externally visible}}
}

static inline __attribute__((always_inline, gnu_inline))
void errors2(int x, ...) {
 // Too many args
  int k = __builtin_va_arg_pack_len(x); // expected-error {{too many arguments to function call, expected 0, have 1}}
  foo(k, __builtin_va_arg_pack(x)); // expected-error {{too many arguments to function call, expected 0, have 1}}

  bar(__builtin_va_arg_pack()); // expected-error {{__builtin_va_arg_pack can only be used as a parameter to a variadic argument}}
  foo(__builtin_va_arg_pack()); // expected-error {{__builtin_va_arg_pack can only be used as a parameter to a variadic argument}}
  foo(x, k, __builtin_va_arg_pack()); // expected-error {{__builtin_va_arg_pack can only be used as a parameter to a variadic argument}}
}
static inline __attribute__((always_inline, gnu_inline))
void pass(int x, ...) {
  int k = __builtin_va_arg_pack_len();
  foo(k, __builtin_va_arg_pack());
}
#elif defined(DISABLE) || defined(OFF)
static inline __attribute__((always_inline, gnu_inline))
void pass(int x, ...) {
  int k = __builtin_va_arg_pack_len(); // expected-error {{use of unknown builtin '__builtin_va_arg_pack_len'}}
  foo(k, __builtin_va_arg_pack()); // expected-error {{use of unknown builtin '__builtin_va_arg_pack'}}
}
#else
#error Invalid Option
#endif
