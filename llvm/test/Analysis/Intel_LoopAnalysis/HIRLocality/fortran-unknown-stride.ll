; REQUIRES: asserts

; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-locality-analysis -hir-spatial-locality -debug-only=hir-locality-analysis -hir-details-dims 2>&1 | FileCheck %s

; Verify that the two refs (%ptr)[i1] and (%ptr)[i1+1] are put into different
; spatial groups because of unknown stride.

; TODO: This test helps to verify that the two refs do not have a (literal)
; constant distance due to current implementation but it is not a long term fix.
; Locality analysis should use heuristics in the absence of such information.

; CHECK: Group 0 contains:
; CHECK:        (%ptr)[0:i1:%stride(i32*:0)] {sb:13}
; CHECK: Group 1 contains:
; CHECK:        (%ptr)[0:i1 + 1:%stride(i32*:0)] {sb:13}


define void @func(i32* %ptr, i64 %stride, i64 %t) {
bb60:
  br label %bb64

bb64:                                             ; preds = %bb64, %bb60
  %iv = phi i64 [ %iv.next, %bb64 ], [ 1, %bb60 ]
  %sub0 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride, i32* elementtype(i32) %ptr, i64 %iv)
  %ld1 = load i32, i32* %sub0, align 4
  %add = add nsw i64 %iv, 1
  %sub.0 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride, i32* elementtype(i32) %ptr, i64 %add)
  %ld2 = load i32, i32* %sub.0, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond2488 = icmp eq i64 %iv.next, 10
  br i1 %exitcond2488, label %bb65, label %bb64

bb65:
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1


