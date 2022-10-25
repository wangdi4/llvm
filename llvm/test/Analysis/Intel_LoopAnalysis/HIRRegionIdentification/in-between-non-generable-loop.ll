; RUN: opt < %s -passes='print<hir-region-identification>' -disable-output 2>&1 | FileCheck %s

; Verify that we are able to handle this case successfully where the
; non-generable loop %non.gen.loop prevents creation of fused region for %loop1
; and %loop2. We create separate regions for %loop1 and %loop2.

; The was previously resulting in a compfail.

; CHECK: Region 1
; CHECK:   EntryBB: %loop1

; CHECK: Region 2
; CHECK:   EntryBB: %loop2


target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define void @foo() {
entry:
  br label %loop1

loop1:                                                ; preds = %loop1, %entry
  %iv1 = phi i64 [ 0, %entry ], [ %iv1.inc, %loop1 ]
  %iv1.inc = add nuw nsw i64 %iv1, 1
  %cmp1 = icmp eq i64 %iv1, 51
  br i1 %cmp1, label %loop.exit1, label %loop1

loop.exit1:                                                ; preds = %loop1
  br label %non.gen.loop

non.gen.loop:                                     ; preds = %non.gen.latch, %loop.exit1
  %bool = phi i1 [ 0, %loop.exit1 ], [ 1, %non.gen.latch ]
  br i1 %bool, label %loop.exit2, label %non.gen.latch

loop.exit2:                                                ; preds = %non.gen.loop
  br label %loop2

non.gen.latch:                                    ; preds = %non.gen.loop
  br label %non.gen.loop

loop2:                                                ; preds = %loop2, %loop.exit2
  %iv2 = phi i64 [ 0, %loop.exit2 ], [ %iv2.inc, %loop2 ]
  %iv2.inc = add nuw nsw i64 %iv2, 1
  %cmp2 = icmp eq i64 %iv2, 51
  br i1 %cmp2, label %exit, label %loop2

exit:                                               ; preds = %loop2
  ret void
}
