; RUN: opt < %s -enable-new-pm=0 -analyze -hir-region-identification -debug-only=hir-region-identification 2>&1 | FileCheck %s
; RUN: opt < %s -passes='print<hir-region-identification>' -debug-only=hir-region-identification -disable-output 2>&1 | FileCheck %s


; CHECK: Call branch instruction currently not supported.


define void @foo(i32 %x) {
entry:
  br label %loop

loop:
  %iv = phi i32 [ 0, %entry ], [ %iv.inc, %latch ]
  %cmp = icmp sgt i32 %iv, 5
  br i1 %cmp, label %bb, label %latch

bb:
  callbr void asm "", "r,X"(i32 %x, i8* blockaddress(@foo, %indirect))
          to label %latch [label %indirect]

latch:
  %iv.inc = add nsw i32 %iv, 1
  %cmp1 = icmp sgt i32 %iv.inc, 15
  br i1 %cmp1, label %exit, label %loop

exit:
  ret void

indirect:
  ret void
}
