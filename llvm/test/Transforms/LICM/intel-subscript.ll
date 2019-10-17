; RUN: opt -S -licm < %s | FileCheck %s
; RUN: opt -S -aa-pipeline=basic-aa -passes='require<opt-remark-emit>,loop(licm)' < %s | FileCheck %s

; Check that loads %A.03 and %s may be hoisted of the loop by LICM.
; Note: the %s load address is computed by @llvm.intel.subscript intrinsic.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%dv = type { float*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

define void @foo_(%dv* noalias dereferenceable(72) nocapture readonly %A, i32* noalias nocapture readonly %N) {
; CHECK: entry:
entry:
  %A.0 = getelementptr inbounds %dv, %dv* %A, i64 0, i32 0
  %A.6.1 = getelementptr inbounds %dv, %dv* %A, i64 0, i32 6, i64 0, i32 1
  %N4 = load i32, i32* %N, align 4
  %rel = icmp sgt i32 %N4, 100
  %sptr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %A.6.1, i32 0)
; CHECK: %A.03 = load float*, float** %A.0, align 8
; CHECK: %s = load i64, i64* %sptr, align 8
  br label %loop_header

; CHECK: loop_header:
loop_header:
  %indvars.iv = phi i64 [ %indvars.iv.next, %loop_latch ], [ 1, %entry ]
  br i1 %rel, label %then, label %loop_latch

then:
  %A.03 = load float*, float** %A.0, align 8
  %s = load i64, i64* %sptr, align 8
  %p = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 %s, float* %A.03, i64 %indvars.iv)
  store float 1.000000e+00, float* %p, align 4
  br label %loop_latch

loop_latch:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond, label %exit, label %loop_header

exit:
  ret void
}

declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #0
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #0

attributes #0 = { nounwind readnone speculatable }

