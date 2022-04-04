; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -debug-only=hir-complete-unroll  2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll" -debug-only=hir-complete-unroll  2>&1 < %s | FileCheck %s

; Verify that GEPCost of this conditinal ref is ((2 + 1) * 8) = 24. 2 from base
; cost of non-simplified ref and 1 from code size increase of non-simplified
; dimension [%idx1].

; Also verify that GEPSavings of this conditional ref is capped to (2 * 8) = 16
; even though 3 dimensions of the ref [i1][i1][i1] can be simplified.

; CHECK: GEPCost: 24
; CHECK: GEPSavings: 16

; CHECK: + DO i1 = 0, 7, 1   <DO_LOOP>
; CHECK: |   if (i1 + 1 != %idx1)
; CHECK: |   {
; CHECK: |      (%ptr)[i1][i1][i1][%idx1] = 0;
; CHECK: |   }
; CHECK: + END LOOP


define void @foo(i32* %ptr, i64 %idx1, i64 %idx2, i64 %idx3) {
entry:
  %add1 = add i64 %idx1, 1
  br label %pre

pre:
  br label %bb475

bb475:                                            ; preds = %bb103, %pre
  %iv = phi i64 [ %indvars.iv.next5749, %bb103 ], [ 1, %pre ]
  %rel.73 = icmp eq i64 %iv, %idx1
  br i1 %rel.73, label %bb103, label %bb479_else

bb479_else:                                       ; preds = %bb475
  %sub1 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %ptr, i64 %add1)
  %sub2 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 20, i32* elementtype(i32) %sub1, i64 %iv)
  %sub3 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 100, i32* elementtype(i32) %sub2, i64 %iv)
  %sub4 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 3, i64 1, i64 500, i32* elementtype(i32) %sub3, i64 %iv)
  store i32 0, i32* %sub4, align 1
  br label %bb103

bb103:                                            ; preds = %bb479_else, %bb475
  %indvars.iv.next5749 = add nuw nsw i64 %iv, 1
  %exitcond5750.not = icmp eq i64 %indvars.iv.next5749, 9
  br i1 %exitcond5750.not, label %bb124.loopexit, label %bb475

bb124.loopexit:
  ret void
}

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #2

attributes #2 = { nounwind readnone speculatable }

