; RUN: opt -switch-to-offload -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -switch-to-offload -S %s | FileCheck %s
;
; This test checks that paropt target filtering retains functions and variables
; from the "llvm.used" and "llvm.compiler.used" lists in addition to the objects
; with "openmp-target-declare" attribute.
;
; CHECK-DAG: @var = global
; CHECK-DAG: define void @foo
; CHECK-DAG: define void @bar

target datalayout = "e-m:e-i64:64-n32:64"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

@llvm.used = appending global [1 x i8*] [i8* bitcast (void ()* @bar to i8*)], section "llvm.metadata"
@llvm.compiler.used = appending global [1 x i8*] [i8* bitcast (i32* @var to i8*)], section "llvm.metadata"

@var = global i32 123, align 4

define void @foo() #0 {
entry:
  ret void
}

define void @bar() {
entry:
  ret void
}

attributes #0 = { "openmp-target-declare"="true" }
