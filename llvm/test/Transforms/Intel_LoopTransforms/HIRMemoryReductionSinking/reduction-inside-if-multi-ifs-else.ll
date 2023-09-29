; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir>"  2>&1 < %s | FileCheck %s

; This test case checks that the reduction inside the If condition is
; identified, and the temporary is created inside a new If. This case
; handles when there are Then and Else branches, and the Else branch
; has multiple reductions, which should be combined inside one
; reduction if.

; HIR before transformation:

;   BEGIN REGION { }
;         + DO i1 = 0, 99, 1   <DO_LOOP>
;         |   if ((%b)[i1] > 10)
;         |   {
;         |      %1 = (%a)[5];
;         |      (%a)[5] = %1 + 2;
;         |   }
;         |   else
;         |   {
;         |      %2 = (%a)[7];
;         |      (%a)[7] = %2 + 3;
;         |      %3 = (%a)[7];
;         |      (%a)[7] = %3 + 4;
;         |   }
;         + END LOOP
;   END REGION


; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:          %tmp = 0;
; CHECK:          %tmp6 = 0;
; CHECK:          %tmp9 = 0;
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   if ((%b)[i1] > 10)
; CHECK:       |   {
; CHECK:       |      %tmp9 = %tmp9  +  2;
; CHECK:       |   }
; CHECK:       |   else
; CHECK:       |   {
; CHECK:       |      %tmp6 = %tmp6  +  3;
; CHECK:       |      %tmp = %tmp  +  4;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK:       if (%tmp9 != 0)
; CHECK:       {
; CHECK:          %1 = (%a)[5];
; CHECK:          (%a)[5] = %1 + %tmp9;
; CHECK:       }
; CHECK:       %cmp = %tmp != 0;
; CHECK:       %cmp8 = %tmp6 != 0;
; CHECK:       %or = %cmp  |  %cmp8;
; CHECK:       if (%or != 0)
; CHECK:       {
; CHECK:          %2 = (%a)[7];
; CHECK:          (%a)[7] = %2 + %tmp6;
; CHECK:          %3 = (%a)[7];
; CHECK:          (%a)[7] = %3 + %tmp;
; CHECK:       }
; CHECK: END REGION

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiS_(ptr nocapture noalias noundef %a, ptr nocapture noundef readonly %b) {
entry:
  %arrayidx2 = getelementptr inbounds i32, ptr %a, i64 5
  %arrayidx4 = getelementptr inbounds i32, ptr %a, i64 7
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  ret void

for.body:                                         ; preds = %entry, %if.end
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx
  %cmp1 = icmp sgt i32 %0, 10
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %1 = load i32, ptr %arrayidx2
  %add = add nsw i32 %1, 2
  store i32 %add, ptr %arrayidx2
  br label %if.end

if.else:
  %2 = load i32, ptr %arrayidx4
  %add2 = add nsw i32 %2, 3
  store i32 %add2, ptr %arrayidx4

  %3 = load i32, ptr %arrayidx4
  %add3 = add nsw i32 %3, 4
  store i32 %add3, ptr %arrayidx4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}