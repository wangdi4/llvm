
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test checks that the partial opt predicate was applied for the condition
; '%t > 1', but the inner condition '%n < 2' wasn't hoisted.
;
; TODO: This needs to be fixed.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   if (i1 != 0 & %t > 1)
;       |   {
;       |      if (%n < 2)
;       |      {
;       |         (%A)[i1] = i1;
;       |      }
;       |   }
;       |   else
;       |   {
;       |      (%A)[i1] = i1 + 1;
;       |   }
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%t > 1)
; CHECK:       {
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   if (i1 != 0)
; CHECK:          |   {
; CHECK:          |      if (%n < 2)
; CHECK:          |      {
; CHECK:          |         (%A)[i1] = i1;
; CHECK:          |      }
; CHECK:          |   }
; CHECK:          |   else
; CHECK:          |   {
; CHECK:          |      (%A)[i1] = i1 + 1;
; CHECK:          |   }
; CHECK:          + END LOOP
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   (%A)[i1] = i1 + 1;
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION


; ModuleID = 'new.ll'
source_filename = "new.ll"

define dso_local void @foo(i32 %t, i32* %A, i32 %n) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.05 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp1 = icmp ne i32 %i.05, 0
  %cmp2 = icmp sgt i32 %t, 1
  %and = and i1 %cmp1, %cmp2
  br i1 %and, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true
  %cmp3 = icmp slt i32 %n, 2
  br i1 %cmp3, label %if.inner, label %for.inc

if.inner:
  %idxprom = sext i32 %i.05 to i64
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  store i32 %i.05, i32* %arrayidx, align 4
  br label %for.inc

if.else:
  %idxprom2 = sext i32 %i.05 to i64
  %arrayidx2 = getelementptr inbounds i32, i32* %A, i64 %idxprom2
  %add = add nsw i32 %i.05, 1
  store i32 %add, i32* %arrayidx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.end, %if.else
  %inc = add nsw i32 %i.05, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

