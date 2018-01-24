// RUN: %clang_cc1 -std=c++11 %s --gnu_version=40500 -S -fintel-compatibility -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics
// REQUIRES: !system-windows

template < typename ... Args > int foo(Args && ...);

void multiQueryFuture(int&& args)
{
   foo(args);
}
// CHECK: call i32 @_Z3fooIIRiEEiDpOT_
