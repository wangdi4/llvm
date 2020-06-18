; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser -hir-details-dims 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-framework-debug=parser -hir-details-dims 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Verify that the mumber of elements in the lower dimension of (%A)[i1][0] are computed as 10 based on stride information.

; CHECK: + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK: |   (%A)[0:i1:40(i32*:0)][0:0:4(i32*:10)] = 5;
; CHECK: + END LOOP


define void @MAIN__(i32* %A) {
alloca:
  br label %bb17

bb17:                                             ; preds = %alloca, %bb17
  %iv = phi i64 [ 1, %alloca ], [ %add11, %bb17 ]
  %t1 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 40, i32* %A, i64 %iv)
  %t2 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %t1, i64 1)
  store i32 5, i32* %t2, align 4
  %add11 = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %add11, 4
  br i1 %exitcond, label %bb1, label %bb17

bb1:                                              ; preds = %bb17
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

attributes #1 = { nounwind readnone speculatable }

