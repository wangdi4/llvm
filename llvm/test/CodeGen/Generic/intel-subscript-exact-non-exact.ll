; RUN: opt -lower-subscript -S < %s | FileCheck %s
; RUN: opt -passes="lower-subscript" -S < %s | FileCheck %s
; RUN: opt -pre-isel-intrinsic-lowering -S < %s | FileCheck %s

; CHECK-LABEL: foo_exact
; CHECK: %[[OFFSET:.*]] = sdiv exact i64 %s, 4
; CHECK: %[[IDX:.*]] = mul nsw i64 %[[OFFSET]], %"var$2.07"
; CHECK: getelementptr inbounds i32, i32* %"foo_$A", i64 %[[IDX]]

; CHECK-LABEL: bar_nonexact
; CHECK: %[[PTR0:.*]] = bitcast i32* %"foo_$A" to i8*
; CHECK: %[[OFFSET:.*]] = mul nsw i64 %s, %"var$2.07"
; CHECK: %[[PTR1:.*]] = getelementptr inbounds i8, i8* %[[PTR0]], i64 %[[OFFSET]]
; CHECK: bitcast i8* %[[PTR1]] to i32*

define void @foo_exact(i32* noalias nocapture %"foo_$A", i64 %s) {
alloca_0:
  br label %bb12

bb12:                                             ; preds = %alloca_0, %bb12
  %"var$2.07" = phi i64 [ 1, %alloca_0 ], [ %add, %bb12 ]
  %"foo_$A_entry[]" = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 0, i64 %s, i32* %"foo_$A", i64 %"var$2.07")
  store i32 2, i32* %"foo_$A_entry[]", align 1
  %add = add nuw nsw i64 %"var$2.07", 1
  %exitcond = icmp eq i64 %add, 11
  br i1 %exitcond, label %bb13, label %bb12

bb13:                                             ; preds = %bb12
  ret void
}

define void @bar_nonexact(i32* noalias nocapture %"foo_$A", i64 %s) {
alloca_0:
  br label %bb12

  bb12:                                             ; preds = %alloca_0, %bb12
  %"var$2.07" = phi i64 [ 1, %alloca_0 ], [ %add, %bb12 ]
  %"foo_$A_entry[]" = tail call i32* @llvm.intel.subscript.nonexact.p0i32.i64.i64.p0i32.i64(i8 0, i64 0, i64 %s, i32* %"foo_$A", i64 %"var$2.07")
  store i32 2, i32* %"foo_$A_entry[]", align 1
  %add = add nuw nsw i64 %"var$2.07", 1
  %exitcond = icmp eq i64 %add, 11
  br i1 %exitcond, label %bb13, label %bb12

  bb13:                                             ; preds = %bb12
  ret void
}

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64)
declare i32* @llvm.intel.subscript.nonexact.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64)

