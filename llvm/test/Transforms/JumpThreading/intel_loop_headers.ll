; RUN: opt -S -jump-threading -jump-thread-loop-header < %s | FileCheck %s
;
target triple = "x86_64-unknown-linux-gnu"

; Do a sanity check that we are, in fact, jump threading across loop headers.
; In f1, we can thread the b2-->b2 edge through b2 to b3.
;
; CHECK-LABEL: f1
; CHECK-LABEL: b2.thread
; 
define void @f1(i1 %arg1) {
b1:
  br label %b2

b2:
  %cond = phi i1 [ %arg1, %b1 ], [ 0, %b2 ]
  call void @f0()
  br i1 %cond, label %b2, label %b3

b3:
  ret void
}

; Check that we correctly re-write the reference to %val2 when we thread the
; b2-->b2 edge through b2 to b3.
;
; CHECK-LABEL: f2
; CHECK: shl
; CHECK-LABEL: b2.thread
; CHECK: shl
;
define i32 @f2(i1 %arg1, i32 %arg2) {
b1:
  br label %b2

b2:
  %cond = phi i1 [ %arg1, %b1 ], [ 0, %b2 ]
  %val1 = phi i32 [ %arg2, %b1 ], [ %val2, %b2 ]
  %val2 = shl i32 %val1, 1
  br i1 %cond, label %b2, label %b3

b3:
  ret i32 %val2
}

; Check that we do not thread the b2-->b2 edge through b2 to b3. This is not
; valid, but the jump threading code was incorrectly concluding that %cond
; was provably false after taking the b2-->b2 edge, because after phi
; translation on that edge, it was incorrectly simplifying the comparison as
; %val2 + 1 == %val2, not recognizing that the incoming %val2 is killed before
; the comparison.
;
; CHECK-LABEL: f3
; CHECK-NOT: b2.thread
;
define i32 @f3(i32 %arg1, i32* %arg2) {
b1:
  br label %b2

b2:
  %val1 = phi i32 [ %arg1, %b1 ], [ %val3, %b2 ]
  %val2 = load volatile i32, i32* %arg2
  %val3 = add i32 %val2, 1
  %cond = icmp eq i32 %val1, %val2
  br i1 %cond, label %b2, label %b3

b3:
  ret i32 %val2
}

; Check that we avoid the "duplicate conditional branch on phi into pred"
; transformation that duplicates b2 into b1. After making that transformation
; and simplifying b1, the transformed code is identical to the original. That
; causes an infinite loop where we continually make the same nop transformation.
;
; CHECK-LABEL: f4
;
define void @f4(i1 %arg1) {
b1:
  br label %b2

b2:
  %cond = phi i1 [ 1, %b1 ], [ %cond, %b2 ]
  br i1 %cond, label %b2, label %b3

b3:
  ret void
}

declare void @f0()
