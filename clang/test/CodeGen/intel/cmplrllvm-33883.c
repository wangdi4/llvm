// RUN: %clang_cc1 -triple x86_64-linux-gnu -disable-cpudispatch-ifuncs \
// RUN: -disable-llvm-passes -O2 -debug-info-kind=limited -dwarf-version=4 \
// RUN: -debugger-tuning=gdb -fintel-compatibility -emit-llvm -o - %s | FileCheck %s

__attribute__((cpu_dispatch(generic, pentium_iii))) int foo(void);
__attribute__((cpu_specific(generic))) int foo(void) { return 0; }
__attribute__((cpu_specific(pentium_iii))) int foo(void) { return 1; }

__attribute__((cpu_dispatch(generic, pentium_iii))) int foo2(void){}
__attribute__((cpu_specific(generic))) int foo2(void) { return 0; }
__attribute__((cpu_specific(pentium_iii))) int foo2(void) { return 1; }

int bar(void) { return foo() + foo2(); }

// Ensure that this doesn't generate debug info for the dispatch function, since
// the 'musttail' calls inside it requires that we have debug info for those
// call locations, and since this is generated code, it doesn't make sense to
// have debug info for these. So instead, we suppress the debug info for the
// dispatcher.
// CHECK: define weak_odr i32 @foo() comdat {
// CHECK: define weak_odr i32 @foo2() comdat {
