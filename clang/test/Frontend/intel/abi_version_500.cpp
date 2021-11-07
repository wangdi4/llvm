// RUN: %clang_cc1 -std=c++11 %s -S -fintel-compatibility -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics
// REQUIRES: !system-windows

template < typename ... Args > int foo(Args && ...);

void multiQueryFuture(int&& args)
{
   foo(args);
}
// CHECK: call noundef i32 @_Z3fooIJRiEEiDpOT_
