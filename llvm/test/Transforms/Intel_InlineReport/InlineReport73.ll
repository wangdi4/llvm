; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup,cgscc(inline),inlinereportemitter' -inline-report=0xe886 -disable-output < %s 2>&1 | FileCheck %s

; Check that at the default setting, llvm.intel.subscript intrinsics are not
; emitted into the inlining report.

; CHECK: COMPILE FUNC: foo
; CHECK-NOT: llvm.intel.subscript

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

define void @foo_(ptr noalias %"foo_$A", ptr noalias %"foo_$N") local_unnamed_addr #0 {
alloca_0:
  %"foo_$N_fetch2" = load i32, ptr %"foo_$N", align 1
  %rel = icmp slt i32 %"foo_$N_fetch2", 1
  br i1 %rel, label %end_label1, label %bb3

bb3:                                              ; preds = %alloca_0, %bb3
  %"foo_$I.0" = phi i32 [ 1, %alloca_0 ], [ %add7, %bb3 ]
  %add = add nuw nsw i32 %"foo_$I.0", 6
  %int_sext = zext i32 %"foo_$I.0" to i64
  %"foo_$A[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %"foo_$A", i64 %int_sext)
  store i32 %add, ptr %"foo_$A[]", align 1
  %add7 = add nuw nsw i32 %"foo_$I.0", 1
  %rel13 = icmp sgt i32 %add7, %"foo_$N_fetch2"
  br i1 %rel13, label %end_label1, label %bb3

end_label1:                                       ; preds = %alloca_0, %bb3
  ret void
}

attributes #0 = { "intel-lang"="fortran"}
