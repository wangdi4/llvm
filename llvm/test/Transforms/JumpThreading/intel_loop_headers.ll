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
; switch statement in b2. We used to not support thread regions that contain
; internal edges back to RegionTop. The fix for cq407644 added the necessary
; code to transform such regions properly. But enabling them adds performance/
; code size risk, so more experimentation is needing before removing the guard
; that prevents threading across these regions. If/when we remove the guard
; and allow these regions to be threaded, we'll need to change this test.
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

; Check that jump threading correctly threads the b0->b2 edge across the
; switch statement in b4. The thread region includes the loop in b3, but the
; thread is only valid when we reach b3 via b2. So in the duplicated code
; region, the b3->b3 edge needs to become b3.thread->b3, and not
; b3.thread->b3.thread. After successful transformation, the new threaded
; code from b2, b3, and b4 all gets collapsed into one block, so check for
; that here.
;
; CHECK-LABEL: f7
; CHECK: b3.thread:
; CHECK-NEXT: call void @f0
; CHECK-NEXT: call i1 @continue
; CHECK-NEXT: br i1 {{.*}}, label %b3, label %b6
;
define i32 @f7(i32 %arg1) {
b0:
  %cmp1 = icmp sgt i32 %arg1, 0
  br i1 %cmp1, label %b2, label %b1

b1:
  br label %b2

b2:
  %phi1 = phi i32 [ 0, %b0 ], [ %arg1, %b1 ]
  call void @f0()
  br label %b3

b3:
  %phi2 = phi i32 [ %phi1, %b2 ], [ %add, %b3 ]
  %add = add i32 %phi2, 1
  %call = call i1 @continue()
  br i1 %call, label %b3, label %b4

b4:
  switch i32 %phi2, label %b6 [
    i32 42, label %b5
    i32 85, label %b5
  ]

b5:
  ret i32 5

b6:
  ret i32 50
}

declare void @f0()
declare i1 @continue()

;test_4877
; The failure mode for this test is crashing, so as long as the test runs to
; completion it's a success.
;
; c test
;int
;fn1 (int p1)
;{
;  int a = 0;
;  for (; a <= 0; a++) {
;    switch (p1) {
;      case 0:
;        p1++;
;      case 1:
;        if (p1)
;          return 0;
;        return 1;
;    }
;  }
;  return 0;
;}
; The issue here is during thread creation of if.end->sw.bb %cleanup gets
; an extra phi(created by SSAUpdate) due to the clone-copy of inc2 in %cleanup.thread.
; cleanup.thread does not need a mapped value of this phi since it can rely
; on the value of a computed in sw.bb1.
;
; Function Attrs: nounwind uwtable
define i32 @_Z3fn1i(i32 %p1) {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %p1.addr.0 = phi i32 [ %p1, %entry ], [ %p1.addr.2, %for.inc ]
  %a.0 = phi i32 [ 0, %entry ], [ %inc2, %for.inc ]
  %retval.0 = phi i32 [ undef, %entry ], [ %retval.1, %for.inc ]
  %cmp = icmp slt i32 %a.0, 1
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  switch i32 %p1.addr.0, label %sw.epilog [
    i32 0, label %sw.bb
    i32 1, label %sw.bb1
  ]

sw.bb:                                            ; preds = %for.body
  %inc = add nsw i32 %p1.addr.0, 1
  br label %sw.bb1

sw.bb1:                                           ; preds = %for.body, %sw.bb
  %p1.addr.1 = phi i32 [ %p1.addr.0, %for.body ], [ %inc, %sw.bb ]
  %tobool = icmp eq i32 %p1.addr.1, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %sw.bb1
  br label %cleanup

if.end:                                           ; preds = %sw.bb1
  br label %cleanup

sw.epilog:                                        ; preds = %for.body
  br label %cleanup

cleanup:                                          ; preds = %sw.epilog, %if.end, %if.then
  %p1.addr.2 = phi i32 [ %p1.addr.0, %sw.epilog ], [ %p1.addr.1, %if.then ], [ %p1.addr.1, %if.end ]
  %retval.1 = phi i32 [ %retval.0, %sw.epilog ], [ 0, %if.then ], [ 1, %if.end ]
  %cleanup.dest.slot.0 = phi i1 [ true, %sw.epilog ], [ false, %if.then ], [ false, %if.end ]
  br i1 %cleanup.dest.slot.0, label %for.inc, label %cleanup3

for.inc:                                          ; preds = %cleanup
  %inc2 = add nsw i32 %a.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %cleanup3

cleanup3:                                         ; preds = %cleanup, %for.end
  %retval.2 = phi i32 [ %retval.1, %cleanup ], [ 0, %for.end ]
  ret i32 %retval.2
}
