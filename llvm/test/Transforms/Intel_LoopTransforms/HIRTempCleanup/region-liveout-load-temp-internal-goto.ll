; RUN: opt < %s -hir-cost-model-throttling=0 -passes="hir-ssa-deconstruction,hir-temp-cleanup" -print-after=hir-temp-cleanup -disable-output 2>&1 | FileCheck %s

; Verify that this test case is compiled successfully. Temp cleanup was trying
; to dereference parent loop of the region liveout temp %ld to check whether 
; the goto exits that loop. It was failing since %ld's parent loop is null.
 
; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 21, 1   <DO_LOOP>
; CHECK: |   (%ptr)[0] = i1 + 2;
; CHECK: + END LOOP

; CHECK: %ld = (%ptr)[0];

; CHECK: + DO i1 = 0, 21, 1   <DO_LOOP>
; CHECK: |   if (%t > -1)
; CHECK: |   {
; CHECK: |      (%ptr)[0] = 2;
; CHECK: |      if (%t > 1)
; CHECK: |      {
; CHECK: |         goto for.latch;
; CHECK: |      }
; CHECK: |   }
; CHECK: |   (%ptr)[0] = 20;
; CHECK: |   for.latch:
; CHECK: + END LOOP
; CHECK: END REGION


define i64 @foo(i64 %t, ptr %ptr) {
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 1, %entry ], [ %iv.next, %for.body ]
  %iv.next = add nuw nsw i64 %iv, 1
  store i64 %iv.next, ptr %ptr
  %cond.not = icmp eq i64 %iv.next, 23
  br i1 %cond.not, label %mid, label %for.body

mid:
  %ld = load i64, ptr %ptr, align 8
  br label %for.body1

for.body1:                                        ; preds = %for.latch, %for.body
  %indvars.iv = phi i64 [ 1, %mid ], [ %indvars.iv.next, %for.latch ]
  %cmp = icmp sgt i64 %t, -1
  br i1 %cmp, label %for.if, label %for.else

for.if:
  %cmp1 = icmp sgt i64 %t, 1 
  store i64 2, ptr %ptr
  br i1 %cmp1, label %for.latch, label %for.else

for.else:
  store i64 20, ptr %ptr
  br label %for.latch

for.latch:                                        ; preds = %for.body1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 23
  br i1 %exitcond.not, label %exit, label %for.body1

exit:                                        ; preds = %for.latch
  ret i64 %ld
}



