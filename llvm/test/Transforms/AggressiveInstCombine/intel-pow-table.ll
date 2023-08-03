; RUN: opt -passes=aggressive-instcombine -S %s | FileCheck -vv %s

; Recognize the IR below as:
; powf(arg2 * (arg ? Scale0 : Scale1) * Multiplier + Offset, Exp)
; and pre-compute a table for all values of arg and arg2 (512 elements).

; CHECK: _PowTable = {{.*}}float 1.000000e+00, float 4.900000e+01, {{.*}}float 5.107600e+06 
; CHECK: @table
; CHECK: getelementptr{{.*}}_PowTable
; CHECK: @table2
; CHECK: getelementptr{{.*}}_PowTable
; CHECK: @neg1
; CHECK-NOT: _PowTable
; CHECK: call{{.*}}llvm.pow.f32
; CHECK: @neg2
; CHECK-NOT: _PowTable
; CHECK: call{{.*}}llvm.pow.f32

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.pow.f32(float, float) #0

define dso_local float @table(i1 %arg, i8 %arg2) local_unnamed_addr #1 {
bb:
  %select = select fast i1 %arg, float 3.000000e+00, float 2.000000e+00
  br label %bb1

bb1:                                              ; preds = %bb
  %uitofp = uitofp i8 %arg2 to float
  %fmul = fmul fast float %select, %uitofp
  %fmul3 = fmul fast float %fmul, 3.00000e+00
  %fadd = fadd fast float %fmul3, 1.000000e+00
  %call = call fast float @llvm.pow.f32(float %fadd, float 2.0e+00)
  ret float %call
}

define dso_local float @table2(i1 %arg, i8 %arg2) local_unnamed_addr #1 {
bb:
  %select = select fast i1 %arg, float 3.000000e+00, float 2.000000e+00
  br label %bb1

bb1:                                              ; preds = %bb
  %uitofp = uitofp i8 %arg2 to float
  %fmul = fmul fast float %select, %uitofp
  %fmul3 = fmul fast float %fmul, 3.00000e+00
  %fadd = fadd fast float %fmul3, 1.000000e+00
  %call = call fast float @llvm.pow.f32(float %fadd, float 2.0e+00)
  ret float %call
}

; pow is not "fast", cannot use this optimization.
define dso_local float @neg1(i1 %arg, i8 %arg2) local_unnamed_addr #1 {
bb:
  %select = select fast i1 %arg, float 3.000000e+00, float 2.000000e+00
  br label %bb1

bb1:                                              ; preds = %bb
  %uitofp = uitofp i8 %arg2 to float
  %fmul = fmul fast float %select, %uitofp
  %fmul3 = fmul fast float %fmul, 3.00000e+00
  %fadd = fadd fast float %fmul3, 1.000000e+00
  %call = call float @llvm.pow.f32(float %fadd, float 2.0e+00)
  ret float %call
}

; currently only 1 table can be generated. The 2nd fmul has a different
; constant, and the existing table cannot be used.
define dso_local float @neg2(i1 %arg, i8 %arg2) local_unnamed_addr #1 {
bb:
  %select = select fast i1 %arg, float 3.000000e+00, float 2.000000e+00
  br label %bb1

bb1:                                              ; preds = %bb
  %uitofp = uitofp i8 %arg2 to float
  %fmul = fmul fast float %select, %uitofp
  %fmul3 = fmul fast float %fmul, 2.00000e+00
  %fadd = fadd fast float %fmul3, 1.000000e+00
  %call = call fast float @llvm.pow.f32(float %fadd, float 2.0e+00)
  ret float %call
}


attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { "unsafe-fp-math"="true" }
