; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; Test that checks if Whole Program isn't safe

; RUN: opt -dtransanalysis -debug-only=dtransanalysis -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -passes='require<dtransanalysis>' -debug-only=dtransanalysis -disable-output %s 2>&1 | FileCheck %s

define void @foo(i32 %n) local_unnamed_addr {
entry:
  br label %loop
loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.next, %loop ]
  %iv.next = add i32 %iv, 1
  %cmp = icmp eq i32 %iv, %n
  br i1 %cmp, label %exit, label %loop
exit:
  ret void
}

; CHECK: dtrans: Whole Program not safe ... DTransAnalysis didn't run
