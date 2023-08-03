
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test checks that the partial opt predicate was applied for the condition
; '%t > 1'. The goal of this test case is to make sure that all the conditions
; are combined into one condition.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   if (%t > 1 & i1 != 0)
;       |   {
;       |      (%A)[i1] = i1;
;       |   }
;       |   if (i1 == 3 & %t > 1)
;       |   {
;       |      (%A)[i1] = i1 + 1;
;       |   }
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK-NEXT:       if (%t > 1)
; CHECK-NEXT:       {
; CHECK-NEXT:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:          |   if (i1 != 0)
; CHECK-NEXT:          |   {
; CHECK-NEXT:          |      (%A)[i1] = i1;
; CHECK-NEXT:          |   }
; CHECK-NEXT:          |   if (i1 == 3)
; CHECK-NEXT:          |   {
; CHECK-NEXT:          |      (%A)[i1] = i1 + 1;
; CHECK-NEXT:          |   }
; CHECK-NEXT:          + END LOOP
; CHECK-NEXT:       }
; CHECK-NEXT: END REGION

; ModuleID = 'new.ll'
source_filename = "new.ll"

define dso_local void @foo(i32 %t, ptr %A) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.05 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %cmp1 = icmp ne i32 %i.05, 0
  %cmp2 = icmp sgt i32 %t, 1
  %and = and i1 %cmp2, %cmp1
  br i1 %and, label %if.then, label %after.if

if.then:                                          ; preds = %land.lhs.true
  %idxprom = sext i32 %i.05 to i64
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %idxprom
  store i32 %i.05, ptr %arrayidx, align 4
  br label %after.if

after.if:
  %cmp3 = icmp eq i32 %i.05, 3
  %and2 = and i1 %cmp3, %cmp2
  br i1 %and2, label %if2.then, label %for.inc

if2.then:                                          ; preds = %land.lhs.true
  %idxprom2 = sext i32 %i.05 to i64
  %arrayidx2 = getelementptr inbounds i32, ptr %A, i64 %idxprom2
  %add = add nsw i32 %i.05, 1
  store i32 %add, ptr %arrayidx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %i.05, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

