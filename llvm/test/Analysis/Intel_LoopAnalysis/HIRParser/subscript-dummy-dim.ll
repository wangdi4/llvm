; RUN: opt -passes="hir-ssa-deconstruction,print<hir>" < %s -disable-output 2>&1 | FileCheck %s

; Verify that dimensions with rank 1 and 2 are eliminated as they are dummy
; dimensions. They have an index of 0 and the exact same stride as the next
; higher dimension. Previously, this ref was parsed as (%A)[i1][0][0][i1].

; CHECK: + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK: |   (%A)[i1][i1] = 5;
; CHECK: + END LOOP


define void @foo(ptr %A) {
alloca:
  br label %bb17

bb17:                                             ; preds = %alloca, %bb17
  %iv = phi i64 [ 1, %alloca ], [ %add11, %bb17 ]
  %s1 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 40, ptr elementtype(i32) %A, i64 %iv)
  %s2 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 40, ptr elementtype(i32) %s1, i64 1)
  %s3 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(i32) %s2, i64 1)
  %s4 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %s3, i64 %iv)
  store i32 5, ptr %s4, align 4
  %add11 = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %add11, 4
  br i1 %exitcond, label %bb1, label %bb17

bb1:                                              ; preds = %bb17
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #1 = { nounwind readnone speculatable }

