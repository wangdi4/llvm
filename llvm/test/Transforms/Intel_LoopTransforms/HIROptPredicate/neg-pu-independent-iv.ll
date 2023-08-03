; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck -check-prefix=CHECK-SINGLE %s

; This test checks that the partial opt predicate was NOT applied for the
; condition '%t > 1'. The reason is that the whole condition inside the If
; can be hoisted.

; HIR before transformation

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   if (i1 != 0 & %t > 1)
;       |   |   {
;       |   |      (%A)[i2] = i2;
;       |   |   }
;       |   + END LOOP
;       + END LOOP
; END REGION


; HIR after transformation


; CHECK-SINGLE: BEGIN REGION { modified }
; CHECK-SINGLE:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-SINGLE:       |   if (i1 != 0 & %t > 1)
; CHECK-SINGLE:       |   {
; CHECK-SINGLE:       |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK-SINGLE:       |      |   (%A)[i2] = i2;
; CHECK-SINGLE:       |      + END LOOP
; CHECK-SINGLE:       |   }
; CHECK-SINGLE:       + END LOOP
; CHECK-SINGLE: END REGION

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck -check-prefix=CHECK-COMBINED %s

; This test case runs the opt predicate twice. The first time it will generate
; the HIR shown before in the previous RUN command. The second call to
; opt-predicate should identify that we can do partial hoisting and apply it.

; CHECK-COMBINED: BEGIN REGION { modified }
; CHECK-COMBINED:       if (%t > 1)
; CHECK-COMBINED:       {
; CHECK-COMBINED:          + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-COMBINED:          |   if (i1 != 0)
; CHECK-COMBINED:          |   {
; CHECK-COMBINED:          |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK-COMBINED:          |      |   (%A)[i2] = i2;
; CHECK-COMBINED:          |      + END LOOP
; CHECK-COMBINED:          |   }
; CHECK-COMBINED:          + END LOOP
; CHECK-COMBINED:       }
; CHECK-COMBINED: END REGION


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
  %cmp1 = icmp ne i32 %j.05, 0
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

