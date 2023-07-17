; Checks that WeightedInstCount analyzes optnone functions.

; RUN: opt -mcpu=skx -passes="require<sycl-kernel-weighted-inst-count-analysis>" -debug-only=sycl-kernel-weighted-inst-count-analysis -S %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: Estimate MemOp Cost for : foo

define void @foo() noinline optnone {
entry:
  %fmuladd = call float @llvm.fmuladd.f32(float 0.000000e+00, float 0.000000e+00, float 0.000000e+00)
  ret void
}

declare float @llvm.fmuladd.f32(float, float, float)

!sycl.kernels = !{!0}

!0 = !{ptr @foo}
