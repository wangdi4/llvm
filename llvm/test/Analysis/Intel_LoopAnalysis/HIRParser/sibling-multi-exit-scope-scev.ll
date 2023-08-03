; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output < %s 2>&1 | FileCheck %s

; Verify that the upper of the loop in the second region (header %for.body97)
; is correctly parsed as (%t28 + -1 * %indvars.iv246.lcssa + -1).

; It was being incorrectly simplied to (%t28 + -2) using the backedge count of
; sibling multi-exit loop (header %for.body60).

; The exit value of %indvars.iv246.lcssa was determined to be 1. This is because
; ScalarEvolution assumed that if the backedge count of a loop is available, it
; is a single-exit loop.

; This is not true in HIR mode as we form trip counts of multi-exit loops.

; The outer loop %for.body (unknown loop which is not formed) is required to
; reproduce the issue so that the two sibling loops are considered in the same
; 'scope' for HIR mode during HIRRegionIdentification.


; CHECK: BEGIN REGION

; CHECK: + DO i1 = 0, %t28 + -1, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   %indvars.iv246.out = -1 * i1 + %t28;
; CHECK: |   %t29 = (%t3)[-1 * i1 + %t28];
; CHECK: |   if (%t29 <= %sub65)
; CHECK: |   {
; CHECK: |      goto for.end73;
; CHECK: |   }
; CHECK: + END LOOP

; CHECK: BEGIN REGION

; CHECK: + DO i1 = 0, %t28 + -1 * %indvars.iv246.lcssa + -1, 1   <DO_LOOP>
; CHECK: |   %t36 = (%t3)[i1 + %indvars.iv246.lcssa + 1];
; CHECK: |   (%t4)[i1 + %indvars.iv246.lcssa + 1] = %t36;
; CHECK: + END LOOP


define void @foo(i64 %t28, ptr %t3, ptr %t4, i32 %sub65, ptr %t1) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.end111, %entry
  %indvars.iv253 = phi i64 [ %indvars.iv253.next, %for.end111 ], [ 1, %entry ]
  %cmp3220 = icmp sgt i64 %t28, 0
  br i1 %cmp3220, label %for.body60.ph, label %for.end111

for.body60.ph:                                    ; preds = %for.body
  br label %for.body60

for.body60:                                       ; preds = %for.inc71, %for.body60.ph
  %indvars.iv246 = phi i64 [ %t28, %for.body60.ph ], [ %indvars.iv.next247, %for.inc71 ]
  %arrayidx64 = getelementptr inbounds i32, ptr %t3, i64 %indvars.iv246
  %t29 = load i32, ptr %arrayidx64, align 4
  %cmp67 = icmp sgt i32 %t29, %sub65
  br i1 %cmp67, label %for.inc71, label %for.end73

for.inc71:                                        ; preds = %for.body60
  %indvars.iv.next247 = add nsw i64 %indvars.iv246, -1
  %cmp58 = icmp sgt i64 %indvars.iv.next247, 0
  br i1 %cmp58, label %for.body60, label %for.end111.loopexit1

for.end73:                                        ; preds = %for.body60
  %indvars.iv246.lcssa = phi i64 [ %indvars.iv246, %for.body60 ]
  br label %for.body97.ph

for.body97.ph:                                    ; preds = %for.end73
  br label %for.body97

for.body97:                                       ; preds = %for.body97, %for.body97.ph
  %indvars.iv248 = phi i64 [ %indvars.iv246.lcssa, %for.body97.ph ], [ %indvars.iv.next249, %for.body97 ]
  %indvars.iv.next249 = add nsw i64 %indvars.iv248, 1
  %arrayidx99 = getelementptr inbounds i32, ptr %t3, i64 %indvars.iv.next249
  %t36 = load i32, ptr %arrayidx99, align 4
  %arrayidx108 = getelementptr inbounds i32, ptr %t4, i64 %indvars.iv.next249
  store i32 %t36, ptr %arrayidx108, align 4
  %exitcond251 = icmp eq i64 %indvars.iv.next249, %t28
  br i1 %exitcond251, label %for.end111.loopexit, label %for.body97

for.end111.loopexit:                              ; preds = %for.body97
  br label %for.end111

for.end111.loopexit1:                             ; preds = %for.inc71
  br label %for.end111

for.end111:                                       ; preds = %for.end111.loopexit1, %for.end111.loopexit, %for.body
  %arrayidx = getelementptr inbounds ptr, ptr %t1, i64 %indvars.iv253
  %ld = load ptr, ptr %arrayidx, align 4
  %indvars.iv253.next = add nsw i64 %indvars.iv253, 1
  %exitcond253 = icmp eq ptr %ld, null
  br i1 %exitcond253, label %exit, label %for.body

exit:                                             ; preds = %for.end111
  ret void
}
