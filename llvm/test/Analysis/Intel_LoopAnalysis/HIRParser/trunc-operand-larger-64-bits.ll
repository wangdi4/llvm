; RUN: opt < %s -hir-ssa-deconstruction -hir-framework -analyze | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

; Verify that we can handle truncate operand greater than 64 bits by pasring it as a blob.

; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   (%A)[0] = trunc.i128.i32((%t /u 1208925819614629174706176));
; CHECK: + END LOOP


define void @foo(i128 %t, i32* %A) {
entry:
  %t.lshr = lshr i128 %t, 80
  %t.trunc = trunc i128 %t.lshr to i32
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop ]
  store i32 %t.trunc, i32* %A, align 8
  %iv.inc = add nsw i64 %iv, 1
  %cmp17.i.i = icmp eq i64 %iv.inc, 4
  br i1 %cmp17.i.i, label %exit, label %loop

exit:
  ret void
} 
