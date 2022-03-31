// RUN: %clang_cc1  -ax=broadwell,skylake-avx512 -triple x86_64-linux-gnu -emit-llvm -o - %s -fopenmp -fintel-openmp-region -disable-llvm-passes | FileCheck %s

// Declarations are not marked for auto-CPU dispatch
// CHECK-DAG: declare i32 @foo() #{{[0-9]*}}
int foo(void);

// This function is eligible for auto-CPU dispatch.
// CHECK-DAG: define dso_local i32 @bar() #{{[0-9]*}} !llvm.auto.cpu.dispatch [[MD:![0-9]*]]
int bar(void) { foo(); return 4;}

// Functions below have predefined target and must not be auto-CPU dispatched.

// CHECK-DAG: define dso_local i32 @qux() #{{[0-9]*}} {
int __attribute__((target("sse4"))) qux(void){ return 4; }

// CHECK-DAG: define dso_local i32 @qax() #{{[0-9]*}} {
int __attribute__((target("arch=ivybridge"))) qax(void){ return 4; }

// CHECK-DAG: define dso_local i32 @baz.S() #{{[0-9]*}} {
// CHECK-DAG: define dso_local i32 @baz.A() #{{[0-9]*}} {
// CHECK-DAG: define weak_odr i32 ()* @baz.resolver() comdat {
// CHECK-DAG: @baz = weak_odr alias i32 (), i32 ()* @baz.ifunc
// CHECK-DAG: @baz.ifunc = weak_odr ifunc i32 (), i32 ()* ()* @baz.resolver
int __attribute__((cpu_specific(ivybridge))) baz(void) { return 4;}
int __attribute__((cpu_specific(generic))) baz(void) { return 4;}
int __attribute__((cpu_dispatch(generic,ivybridge))) baz(void) { }

// CHECK-DAG: define dso_local i32 @vec_variant() #{{[0-9]*}} {
#pragma omp declare simd simdlen(4)
int vec_variant(void) { return 4; }

// CHECK: [[MD]] = !{[[MD_TARGET1:![0-9]*]], [[MD_TARGET2:![0-9]*]]}
// CHECK: [[MD_TARGET1]] = !{!"auto-cpu-dispatch-target", !"broadwell"}
// CHECK: [[MD_TARGET2]] = !{!"auto-cpu-dispatch-target", !"skylake-avx512"}
