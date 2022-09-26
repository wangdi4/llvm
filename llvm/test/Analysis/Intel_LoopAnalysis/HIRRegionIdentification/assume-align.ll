; RUN: opt < %s -passes='print<hir-region-identification>' -disable-output 2>&1 | FileCheck %s

; Verify that we allow assume intrinsic with unknown operand bundle 'align' in
; HIR region.

; CHECK: Region 1
; CHECK: EntryBB: %loop


define void @foo(double* %ptr) {
entry:
  br label %loop

loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.inc, %loop ]
  %cmp = icmp sgt i32 %iv, 5
  call void @llvm.assume(i1 true) [ "align"(double* %ptr, i64 64) ]
  %iv.inc = add nsw i32 %iv, 1
  %cmp1 = icmp sgt i32 %iv.inc, 15
  br i1 %cmp1, label %exit, label %loop

exit:
  ret void
}

declare void @llvm.assume(i1 noundef) #1

attributes #1 = { inaccessiblememonly nocallback nofree nosync nounwind willreturn }
