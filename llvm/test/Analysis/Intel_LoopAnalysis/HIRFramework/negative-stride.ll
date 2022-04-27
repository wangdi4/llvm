; RUN: opt -hir-ssa-deconstruction -analyze -hir-framework -hir-details-dims -enable-new-pm=0 < %s  2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details-dims -disable-output < %s 2>&1 | FileCheck %s

; Verify that we correctly handle negative stride of -4 for the lower dimension
; while computing the number of elements as 20.

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK: |   |   (%ptr)[0:i1:80(float*:0)][0:i2:-4(float*:20)] = 1.000000e+00;
; CHECK: |   + END LOOP
; CHECK: + END LOOP

define void @foo(float* %ptr) {
entry:
  br label %loop.outer

loop.outer:                             ; preds = %entry, %loop_exit6
  %iv.outer = phi i64 [ 1, %entry ], [ %add.9, %loop_exit6 ]
  %"subn_$B.addr_a0$_fetch.1[]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 1, i64 1, i64 80, float* elementtype(float) %ptr, i64 %iv.outer)
  br label %loop_body5

loop_body5:                                       ; preds = %loop.outer, %loop_body5
  %iv.inner = phi i64 [ 1, %loop.outer ], [ %add.7, %loop_body5 ]
  %"subn_$B.addr_a0$_fetch.1[][]" = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 1, i64 -4, float* elementtype(float) %"subn_$B.addr_a0$_fetch.1[]", i64 %iv.inner)
  store float 1.000000e+00, float* %"subn_$B.addr_a0$_fetch.1[][]", align 1
  %add.7 = add nuw nsw i64 %iv.inner, 1
  %exitcond = icmp eq i64 %add.7, 10
  br i1 %exitcond, label %loop_exit6, label %loop_body5

loop_exit6:                                       ; preds = %loop_body5
  %add.9 = add nuw nsw i64 %iv.outer, 1
  %exitcond71 = icmp eq i64 %add.9, 101
  br i1 %exitcond71, label %loop_exit10, label %loop.outer

loop_exit10:                                      ; preds = %loop_exit6
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #1

attributes #1 = { nofree nosync nounwind readnone speculatable }

