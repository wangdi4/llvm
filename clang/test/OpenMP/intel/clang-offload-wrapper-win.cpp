// REQUIRES: system-windows

// RUN: %clang -o %t.tgt.o -c %s
// RUN: clang-offload-wrapper -host x86_64-pc-windows-msvc -o %t.wrapped.bc -kind=openmp -target=x86_64-pc-windows-msvc %t.tgt.o
// RUN: llvm-dis %t.wrapped.bc -o - | FileCheck %s

// CHECK-DAG: @.openmp_offloading.entries_begin = hidden local_unnamed_addr constant [0 x i8] zeroinitializer, section "omp_offloading_entries$A", align 32
// CHECK-DAG: @.openmp_offloading.entries_end = hidden local_unnamed_addr constant [0 x i8] zeroinitializer, section "omp_offloading_entries$C", align 32
// CHECK-DAG: @__dummy.omp_offloading.entry = hidden constant [1 x %__tgt_offload_entry] zeroinitializer, section "omp_offloading_entries$D", align 32
