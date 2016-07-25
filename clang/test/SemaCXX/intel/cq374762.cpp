// RUN: %clang_cc1 -DT=1 -fsyntax-only -fintel-compatibility -verify %s
// RUN: %clang_cc1 -DT=2 -fsyntax-only -fintel-compatibility -verify %s
// RUN: %clang_cc1 -DT=3 -fsyntax-only -fintel-compatibility -verify %s
// RUN: %clang_cc1 -DT=4 -fsyntax-only -fintel-compatibility -verify %s

#if T == 1
namespace abi = __cxxabiv1; // expected-warning {{namespace '__cxxabiv1' is not defined}}
#elif T == 2
namespace abi = ::__cxxabiv1; // expected-warning {{namespace '__cxxabiv1' is not defined}}
#elif T == 3
using namespace __cxxabiv1; // expected-warning {{namespace '__cxxabiv1' is not defined}}
#elif T == 4
using namespace ::__cxxabiv1; // expected-warning {{namespace '__cxxabiv1' is not defined}}
#endif
