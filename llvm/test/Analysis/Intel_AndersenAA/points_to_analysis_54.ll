; This test verifies that "%7 = load float, ptr %2" is NOT removed by
; gvn using Escape Analysis (Andersens's points-to analysis).
; AndersensAA need handle "%5 = ptrtoint ptr %4 to i64" correctly.

; RUN: opt < %s -passes='require<anders-aa>,gvn' -S 2>&1 | FileCheck %s

; CHECK: %i7 = load float, ptr %i2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"TYPE_DEFS$.btSEQ" = type <{ float, [10 x [9 x float]], double, [10 x double], [12 x i8], fp128, [10 x fp128], %complex_64bit, [10 x [9 x %complex_64bit]], [10 x %complex_128bit], %complex_128bit, [8 x i8], %complex_256bit, [10 x %complex_256bit], [9 x i8], [10 x [9 x i8]], i32, [10 x [9 x i32]], i64, [1 x i8], [10 x [9 x [1 x i8]]], [10 x i64], [9 x i8], [10 x [9 x [9 x i8]]], i32, [10 x [9 x i32]], [1 x i8], [10 x [1 x i8]], i32, [10 x i32], [8 x i8] }>
%complex_64bit = type { float, float }
%complex_128bit = type { double, double }
%complex_256bit = type { fp128, fp128 }

define void @MAIN__(ptr %nf_scalars_mp_nf3_, i64 %i0) {
  %i2 = alloca %"TYPE_DEFS$.btSEQ", i64 1, align 8
  store float 0.000000e+00, ptr %i2, align 1
  br label %i3

i3:                                                ; preds = %._crit_edge, %1
  %i4 = getelementptr %"TYPE_DEFS$.btSEQ", ptr %i2, i64 0, i32 0
  %i5 = ptrtoint ptr %i4 to i64
  %foo = add i64 1, %i5
  %bar = sub i64 1, %foo
  %i6 = inttoptr i64 %bar to ptr
  store float 1.000000e+00, ptr %i6, align 1
  %i7 = load float, ptr %i2, align 1
  store float %i7, ptr %nf_scalars_mp_nf3_, align 1
  br label %i8

i8:                                                ; preds = %8, %3
  br label %i8

._crit_edge:                                      ; No predecessors!
  br label %i3
}

