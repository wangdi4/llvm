; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-dead-store-elimination,print<hir>" -hir-create-function-level-region  -disable-output  2>&1 < %s | FileCheck %s

; This test case checks that the first store to %je7, and the load to %je7
; inside the loop aren't converted into temps. The issue is that the store to
; %je7 inside the loop won't be converted into the same temp. There still extra
; analysis to be done for isValidParentChain in order to enable this substitution.

; HIR before transformation

; BEGIN REGION { }
;       (%je7)[0][3] = 0;
;
;       + DO i1 = 0, 16, 1   <DO_LOOP>
;       |   %new.load.3 = (%je7)[0][3];
;       |   (%je7)[0][3] = %new.load.3 + 2;
;       + END LOOP
;
;       (%je7)[0][3] = 3;
;       ret ;
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { }
; CHECK:       (%je7)[0][3] = 0;
; CHECK:       + DO i1 = 0, 16, 1   <DO_LOOP>
; CHECK:       |   %new.load.3 = (%je7)[0][3];
; CHECK:       |   (%je7)[0][3] = %new.load.3 + 2;
; CHECK:       + END LOOP
; CHECK:       (%je7)[0][3] = 3;
; CHECK:       ret ;
; CHECK: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr %je7) {
entry:
  br label %bb

bb:
  %gep3 = getelementptr inbounds [100 x i32], ptr %je7, i64 0, i64 3
  store i32 0, ptr %gep3
  br label %outer.loop

outer.loop:
  %iv.outer = phi i32 [ 0, %bb], [ %iv.outer.inc, %latch]
  %loop2.gep3 = getelementptr inbounds [100 x i32], ptr %je7, i64 0, i64 3
  %new.load.3 = load i32, ptr %loop2.gep3
  %update.temp3 = add i32 2, %new.load.3
  store i32 %update.temp3, ptr %loop2.gep3, align 4
  br label %latch

latch:
  %iv.outer.inc = add i32 %iv.outer, 1
  %yump1 = icmp eq i32 %iv.outer.inc, 17
  br i1 %yump1, label %post.loop, label %outer.loop

post.loop:
  store i32 3, ptr %gep3
  br label %exit

exit:
  ret void
}
