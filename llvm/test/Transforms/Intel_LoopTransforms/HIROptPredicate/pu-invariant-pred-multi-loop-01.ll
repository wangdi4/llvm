
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test checks that the partial opt predicate was applied for the condition
; '%t > 1'. The goal of this test is that the condition is hoisted outside of
; the loopnest.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   if (i2 != 0 & %t > 1)
;       |   |   {
;       |   |      (%A)[i2] = i2;
;       |   |   }
;       |   + END LOOP
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%t > 1)
; CHECK:       {
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   |   if (i2 != 0)
; CHECK:          |   |   {
; CHECK:          |   |      (%A)[i2] = i2;
; CHECK:          |   |   }
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION

; ModuleID = 'new.ll'
source_filename = "new.ll"

define dso_local void @foo(i32 %t, ptr %A) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %j.05 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  br label %for.inner.body

for.inner.body:                                         ; preds = %entry, %for.inc
  %i.05 = phi i32 [ 0, %for.body ], [ %inc2, %for.inner.inc ]
  %cmp1 = icmp ne i32 %i.05, 0
  %cmp2 = icmp sgt i32 %t, 1
  %and = and i1 %cmp1, %cmp2
  br i1 %and, label %if.then, label %for.inner.inc

if.then:                                          ; preds = %land.lhs.true
  %idxprom = sext i32 %i.05 to i64
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %idxprom
  store i32 %i.05, ptr %arrayidx, align 4
  br label %for.inner.inc

for.inner.inc:                                          ; preds = %if.end
  %inc2 = add nsw i32 %i.05, 1
  %cmp3 = icmp slt i32 %inc2, 100
  br i1 %cmp3, label %for.inner.body, label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %j.05, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

