; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom,print<hir>" -hir-dd-analysis-verify=L2 -disable-output 2>&1 < %s  | FileCheck %s

; This test case checks that the copy in <40> isn't converted into memcpy
; since there is a backward dependency between <19> and <40>.

; HIR before transformation

; <0>   BEGIN REGION { }
; <54>        + DO i1 = 0, 56, 1   <DO_LOOP>
; <55>        |   + DO i2 = 0, -1 * i1 + 43, 1   <DO_LOOP>  <MAX_TC_EST = 44>
; <56>        |   |   + DO i3 = 0, 16, 1   <DO_LOOP>
; <19>        |   |   |   (%a)[0][-1 * i2 + 44][i3] = (%b)[0][i3 + 2];
; <22>        |   |   |   if (-17 * i2 + -1 * i3 + %nw1.promoted236 == i1 + i2 + -44)
; <22>        |   |   |   {
; <27>        |   |   |      (%p)[i3 + 1] = i3 + 1;
; <22>        |   |   |   }
; <56>        |   |   + END LOOP
; <56>        |   |
; <40>        |   |   (%b)[0][-1 * i2 + 46] = (%c)[0][-1 * i2 + 45];
; <55>        |   + END LOOP
; <54>        + END LOOP
; <0>   END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       + DO i1 = 0, 56, 1   <DO_LOOP>
; CHECK:       |   + DO i2 = 0, -1 * i1 + 43, 1   <DO_LOOP>  <MAX_TC_EST = 44>
; CHECK:       |   |      @llvm.memcpy.p0.p0.i64(&((i8*)(%a)[0][-1 * i2 + 44][0]),  &((i8*)(%b)[0][2]),  68,  0);
; CHECK:       |   |   + DO i3 = 0, 16, 1   <DO_LOOP>
; CHECK:       |   |   |   if (-17 * i2 + -1 * i3 + %nw1.promoted236 == i1 + i2 + -44)
; CHECK:       |   |   |   {
; CHECK:       |   |   |      (%p)[i3 + 1] = i3 + 1;
; CHECK:       |   |   |   }
; CHECK:       |   |   + END LOOP
; CHECK:       |   |
; CHECK:       |   |   (%b)[0][-1 * i2 + 46] = (%c)[0][-1 * i2 + 45];
; CHECK:       |   + END LOOP
; CHECK:       + END LOOP
; CHECK: END REGION



define void @foo(ptr noalias nocapture %a, ptr noalias nocapture %b, ptr noalias nocapture %c, i64 %nw1.promoted236, ptr %p) {
entry:
  br label %loop.preheader

loop.preheader:
  br label %loop.body

loop.body:
  %outer.iv = phi i64 [ 1, %loop.preheader ], [ %outer.iv.next, %latch ]
  %outer.iv.next = add nsw i64 %outer.iv, 1
  br label %mid.loop

mid.loop:
  %mid.iv = phi i64 [ 45, %loop.body ], [ %mid.iv.next, %mid.latch ]
  %dec228.lcssa237 = phi i64 [ %nw1.promoted236, %loop.body ], [ %4, %mid.latch ]
  %mid.iv.next = add nsw i64 %mid.iv, -1
  %0 = add nuw nsw i64 %mid.iv, 1
  %sub = sub nsw i64 %outer.iv, %mid.iv
  br label %inner.loop

inner.loop:
  %inner.iv = phi i64 [ 1, %mid.loop ], [ %iv.inc, %cont ]
  %dec228229 = phi i64 [ %dec228.lcssa237, %mid.loop ], [ %dec, %cont ]
  %iv.inc = add nuw nsw i64 %inner.iv, 1
  %arrayidx27 = getelementptr inbounds [100 x i32], ptr %b, i64 0, i64 %iv.inc
  %1 = load i32, ptr %arrayidx27
  %2 = add nsw i64 %inner.iv, -1
  %arrayidx33 = getelementptr inbounds [100 x [100 x i32]], ptr %a, i64 0, i64 %mid.iv.next, i64 %2
  store i32 %1, ptr %arrayidx33
  %dec = add i64 %dec228229, -1
  %cmp = icmp eq i64 %dec228229, %sub
  br i1 %cmp, label %if.then, label %cont

if.then:
  %arrayidxp = getelementptr inbounds i64, ptr %p, i64 %inner.iv
  store i64 %inner.iv, ptr %arrayidxp
  br label %cont

cont:
  %cmp1 = icmp eq i64 %iv.inc, 18
  br i1 %cmp1, label %mid.latch, label %inner.loop

mid.latch:
  %arrayidx55 = getelementptr inbounds [100 x i32], ptr %c, i64 0, i64 %mid.iv
  %3 = load i32, ptr %arrayidx55
  %arrayidx58 = getelementptr inbounds [100 x i32], ptr %b, i64 0, i64 %0
  store i32 %3, ptr %arrayidx58
  %cmp2 = icmp eq i64 %mid.iv.next, %outer.iv
  %4 = add i64 %dec228.lcssa237, -17
  br i1 %cmp2, label %latch, label %mid.loop

latch:
  %cmp3 = icmp eq i64 %outer.iv.next, 58
  br i1 %cmp3, label %loopexit, label %loop.body

loopexit:
  br label %exit

exit:
  ret void
}