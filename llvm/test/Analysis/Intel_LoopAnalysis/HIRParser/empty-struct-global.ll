; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -disable-output 2>&1 | FileCheck %s

; Verify that we are able to parse this test case correctly. Parser was
; hitting an assert for trying to access 0th element of the empty struct.

; CHECK: + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK: |   (@A)[i1] = 5;
; CHECK: + END LOOP


%empty_struct = type <{}>
%empty_struct_wrapper = type <{ %empty_struct, i32 }>

@A = internal unnamed_addr global %empty_struct_wrapper zeroinitializer, align 16

define void @foo() {
entry:
  br label %bb17

bb17:                                             ; preds = %entry, %bb17
  %iv = phi i64 [ 1, %entry ], [ %add11, %bb17 ]
  %ld = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 4, ptr elementtype(i32) @A, i64 %iv)
  store i32 5, ptr %ld, align 4
  %add11 = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %add11, 4
  br i1 %exitcond, label %bb1, label %bb17

bb1:                                              ; preds = %bb17
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #1 = { nounwind readnone speculatable }

