; RUN: opt -passes="lower-subscript" -S < %s | FileCheck %s

; Verify that the index size of 32 bits specified in datalayout is used for
; GEP indices instead of ptr size of 40 bits.

; CHECK: getelementptr inbounds i32, ptr %"foo_$A", i32


target datalayout = "e-m:m-p:40:64:64:32-i32:32-i16:16-i8:8-n32"

define void @foo_(ptr noalias nocapture %"foo_$A") {
alloca_0:
  br label %bb12

bb12:                                             ; preds = %alloca_0, %bb12
  %"var$2.07" = phi i64 [ 1, %alloca_0 ], [ %add, %bb12 ]
  %"foo_$A_entry[]" = tail call ptr @llvm.intel.subscript.p0.i32.i32.p0.i64(i8 0, i32 1, i32 4, ptr elementtype(i32) %"foo_$A", i64 %"var$2.07")
  store i32 2, ptr %"foo_$A_entry[]", align 1
  %add = add nuw nsw i64 %"var$2.07", 1
  %exitcond = icmp eq i64 %add, 11
  br i1 %exitcond, label %bb13, label %bb12

bb13:                                             ; preds = %bb12
  ret void
}

declare ptr @llvm.intel.subscript.p0.i32.i32.p0.i64(i8, i32, i32, ptr, i64)
