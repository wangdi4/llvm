; RUN: opt -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region -debug-only=hir-dd-test -stats < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-create-function-level-region -hir-dd-analysis-verify=Region -debug-only=hir-dd-test -disable-output -stats  2>&1 < %s | FileCheck %s

; The test case checks that delta test recognize mem refs independence in the following loop:
;     do i=0,1000
;         A(i+2,2*i+1,i+3) = A(2*i,i,i) + 9.0
;     enddo
;

; CHECK:   Coupled = {2}
; CHECK: starting on coupled subscripts
; CHECK: testing subscript 0
; CHECK:   src = 2 * i1 + -1
; CHECK:   dst = i1 + 1
; CHECK: intersect constraints
; CHECK:    X = Any
; CHECK:    Y = Line is 2*X + -1*Y = 2
; CHECK: testing subscript 1
; CHECK:   src = i1 + -1
; CHECK:   dst = 2 * i1
; CHECK: intersect constraints
; CHECK:    X = Line is 2*X + -1*Y = 2
; CHECK:    Y = Line is 1*X + -2*Y = 1
; CHECK:    intersect 2 lines
; CHECK:                different slopes
; CHECK:                Xtop = -3
; CHECK:                Xbot = -3
; CHECK:                Ytop = 0
; CHECK:                Ybot = 3
; CHECK:                X = 1, Y = 0
; CHECK:                upper bound = 1000
; CHECK: testing subscript 2
; CHECK:   src = i1 + -1
; CHECK:   dst = i1 + 2
; CHECK: intersect constraints
; CHECK:    X = Point is <1, 0>
; CHECK:    Y = Distance is -3 (1*X + -1*Y = 3)
; CHECK:            intersect Point and Line
; CHECK: Is Independent!

; CHECK: 2 hir-dd-test               - Delta successes


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @foo(float* noalias nocapture %A, i32* noalias nocapture readonly %N) local_unnamed_addr  {
alloca_0:
  %N_fetch = load i32, i32* %N, align 1
  %int_sext = sext i32 %N_fetch to i64
  %mul = shl nsw i64 %int_sext, 2
  %mul3 = mul nsw i64 %mul, %int_sext
  br label %bb4

bb4:                                              ; preds = %bb4, %alloca_0
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb4 ], [ 0, %alloca_0 ], !in.de.ssa !0
  %0 = shl nuw nsw i64 %indvars.iv, 1
  %A_ = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %mul3, float* elementtype(float) %A, i64 %indvars.iv)
  %A__ = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %mul, float* elementtype(float) %A_, i64 %indvars.iv)
  %A___ = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %A__, i64 %0)
  %A___fetch = load float, float* %A___, align 1
  %add11 = fadd float %A___fetch, 9.000000e+00
  %1 = add nuw nsw i64 %indvars.iv, 2
  %2 = or i64 %0, 1
  %3 = add nuw nsw i64 %indvars.iv, 3
  %A32_ = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 2, i64 1, i64 %mul3, float* elementtype(float) %A, i64 %3)
  %A32__ = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 %mul, float* elementtype(float) %A32_, i64 %2)
  %A32___ = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 4, float* elementtype(float) %A32__, i64 %1)
  store float %add11, float* %A32___, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1001
  %indvars.iv.in = call i64 @llvm.ssa.copy.i64(i64 %indvars.iv.next), !in.de.ssa !0
  br i1 %exitcond, label %bb1, label %bb4

bb1:                                              ; preds = %bb4
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64)

; Function Attrs: nounwind readnone
declare i64 @llvm.ssa.copy.i64(i64 returned)

!omp_offload.info = !{}

!0 = !{!"indvars.iv.de.ssa"}

