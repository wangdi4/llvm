; RUN: opt < %s -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,print<hir>,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that we successfully build DD graph after complete unroll of the loop.
; The test was asserting during invalid construction of GEP location pointer for
; 2 dimensional subscript references like (%A)[0][0].

; Note: DD is creating unnecessary edges between references in this case so we
; are not checking for them for maintaining stability of the test case in case
; the logic is refined in the future.

; Incoming HIR-
; + DO i1 = 0, 2, 1   <DO_LOOP>
; |   (%A)[i1][0] = 5;
; + END LOOP

; Verify that complete unroll happened

; CHECK: (%A)[0][0] = 5;
; CHECK: (%A)[1][0] = 5;
; CHECK: (%A)[2][0] = 5;


define void @MAIN__(i32* %A) {
alloca:
  br label %bb17

bb17:                                             ; preds = %alloca, %bb17
  %iv = phi i64 [ 1, %alloca ], [ %add11, %bb17 ]
  %t1 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 40, i32* elementtype(i32) %A, i64 %iv)
  %t2 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %t1, i64 1)
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

