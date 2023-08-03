
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; This test checks that the partial opt predicate was applied for the condition
; '%t > 1'. The reason is that the first predicate ('B[0] > B[i1]') will try
; to trigger the partial unswitching for load instructions, but then it is
; found that is not a candidate. Therefore, it will try doing PU for
; independent IV.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   if ((%B)[0] > (%B)[i1] & %t > 1)
;       |   {
;       |      (%A)[i1] = i1;
;       |   }
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:       if (%t > 1)
; CHECK:       {
; CHECK:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:          |   if ((%B)[0] > (%B)[i1])
; CHECK:          |   {
; CHECK:          |      (%A)[i1] = i1;
; CHECK:          |   }
; CHECK:          + END LOOP
; CHECK:       }
; CHECK: END REGION


; ModuleID = 'new.ll'
source_filename = "new.ll"

define dso_local void @foo(i32 %t, ptr %A, ptr %B) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %i.05 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %idz = sext i32 %i.05 to i64
  %lhs = getelementptr inbounds i32, ptr %B, i64 0
  %rhs = getelementptr inbounds i32, ptr %B, i64 %idz
  %lhs.ld = load i32, ptr %lhs
  %rhs.ld = load i32, ptr %rhs
  %cmp1 = icmp sgt i32 %lhs.ld, %rhs.ld
  %cmp2 = icmp sgt i32 %t, 1
  %and = and i1 %cmp1, %cmp2
  br i1 %and, label %if.then, label %for.inc

if.then:                                          ; preds = %land.lhs.true
  %idxprom = sext i32 %i.05 to i64
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %idxprom
  store i32 %i.05, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %i.05, 1
  %cmp = icmp slt i32 %inc, 100
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.inc
  ret void
}

