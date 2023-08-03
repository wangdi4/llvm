; RUN: opt < %s -disable-output "-passes=print<scalar-evolution>" 2>&1 | FileCheck %s

; Verify that we are able to compute the backedge taken count of inner loop by 
; safely truncating the zero-extended operand %zext.iv to i32. %zext.iv is a 
; zero-extend of IV and %upper has known constant range which fits in 32 bits.

; CHECK: %upper
; CHECK-NEXT: {10,+,10}<nuw><nsw><%loop.outer> U: [10,51) S: [10,51)
 
; Note that the first brace is inside a regex to avoid confusing FileCheck with
; '{{' pattern.

; CHECK: %zext.iv
; CHECK-NEXT: (zext i32 {{[{]}}{1,+,10}<%loop.outer>,+,1}<%loop.inner> to i64)

; CHECK: Loop %loop.inner: backedge-taken count is 9


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


define dso_local i32 @main([50 x i32]* %tab) {
entry:
  br label %loop.outer

loop.outer:                                        ; preds = %outer.latch, %entry
  %indvars.iv2 = phi i64 [ %indvars.iv.next3, %outer.latch ], [ 0, %entry ]
  %I.056 = phi i32 [ 4, %entry ], [ %dec, %outer.latch ]
  %mul = mul nuw nsw i32 %I.056, 10
  br label %loop.inner.preheader

loop.inner.preheader:                              ; preds = %loop.outer
  %upper = add nuw nsw i64 %indvars.iv2, 10
  br label %loop.inner

loop.inner:                                        ; preds = %loop.inner, %loop.inner.preheader
  %indvars.iv = phi i64 [ %indvars.iv.next, %loop.inner ], [ %indvars.iv2, %loop.inner.preheader ]
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %trunc.iv = trunc i64 %indvars.iv.next to i32
  %zext.iv = zext i32 %trunc.iv to i64
  %exitcond = icmp ne i64 %zext.iv, %upper
  br i1 %exitcond, label %loop.inner, label %outer.latch

outer.latch:                                        ; preds = %outer.latch.loopexit
  %dec = add nsw i32 %I.056, -1
  %cmp2 = icmp ugt i32 %I.056, 0
  %indvars.iv.next3 = add nuw nsw i64 %indvars.iv2, 10
  br i1 %cmp2, label %loop.outer, label %exit

exit:                                        ; preds = %for.inc27
  ret i32 0
}
