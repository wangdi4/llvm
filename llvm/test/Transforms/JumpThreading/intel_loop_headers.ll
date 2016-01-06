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

; Check that the "duplicate conditional branch on phi into pred"
; transformation doesn't cause us to go into an infinite loop of duplicating
; b1 into its predecessor and then jump threading across b3.
;
; CHECK-LABEL: f5
;
define i8* @f5(i8* %arg1, i64 %arg2) {
b0:
  %cmp1 = icmp sgt i64 %arg2, 0
  br label %b1

b1:
  %cmp2 = phi i1 [ %cmp1, %b0 ], [ %cmp3, %b3 ]
  %m = phi i32 [ -2147483648, %b0 ], [ %shr, %b3 ]
  %j = phi i64 [ 0, %b0 ], [ %inc, %b3 ]
  br i1 %cmp2, label %b2, label %b3

b2:
  %arrayidx = getelementptr inbounds i8, i8* %arg1, i64 %j
  store i8 0, i8* %arrayidx, align 1
  br label %b3

b3:
  %shr = ashr i32 %m, 1
  %inc = add i64 %j, 1
  %tobool2 = icmp eq i32 %shr, 0
  %cmp3 = icmp slt i64 %inc, %arg2
  br i1 %tobool2, label %b4, label %b1

b4:
  ret i8* %arg1
}

; Check that jump threading doesn't attempt to thread the b0->b1 edge across the
; switch statement in b2. In theory, that transform is possible but requires
; that the b1->b1 edge not be duplicated as b1.thread->b1.thread. It must
; instead be copied as b1.thread->b1, which complicates the transform somewhat,
; so it is not currently supported.
;
; CHECK-LABEL: f6
; CHECK-NOT: b1.thread
;
define i32 @f6(i32 %arg1) {
b0:
  br label %b1

b1:
  %0 = phi i32 [ 0, %b0 ], [ %1, %b1 ]
  %1 = add i32 %0, 1
  %2 = call i1 @continue()
  br i1 %2, label %b1, label %b2

b2:
  switch i32 %0, label %b4 [
    i32 42, label %b3
    i32 85, label %b3
  ]

b3:
  ret i32 5

b4:
  ret i32 50
}


declare void @f0()
declare i1 @continue()
