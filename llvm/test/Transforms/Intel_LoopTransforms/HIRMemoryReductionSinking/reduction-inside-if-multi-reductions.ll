; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir>" 2>&1 < %s | FileCheck %s

; This test case checks that multiple reductions inside the same If are
; identified. Also, this test case checks that the reductions were ORed
; properly and handled inside one condition.

; HIR before transformation:

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   if ((%b)[i1] > 10)
;       |   {
;       |      %1 = (%a)[5];
;       |      (%a)[5] = %1 + 2;
;       |      %2 = (%c)[7];
;       |      (%c)[7] = %2 + 4;
;       |      %3 = (%d)[10];
;       |      (%d)[10] = %3 + 8;
;       |   }
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:          %tmp = 0;
; CHECK:          %tmp6 = 0;
; CHECK:          %tmp9 = 0;
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   if ((%b)[i1] > 10)
; CHECK:       |   {
; CHECK:       |      %tmp9 = %tmp9  +  2;
; CHECK:       |      %tmp6 = %tmp6  +  4;
; CHECK:       |      %tmp = %tmp  +  8;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK:       %cmp = %tmp != 0;
; CHECK:       %cmp8 = %tmp6 != 0;
; CHECK:       %or = %cmp  |  %cmp8;
; CHECK:       %cmp11 = %tmp9 != 0;
; CHECK:       %or12 = %or  |  %cmp11;
; CHECK:       if (%or12 != 0)
; CHECK:       {
; CHECK:          %1 = (%a)[5];
; CHECK:          (%a)[5] = %1 + %tmp9;
; CHECK:          %2 = (%c)[7];
; CHECK:          (%c)[7] = %2 + %tmp6;
; CHECK:          %3 = (%d)[10];
; CHECK:          (%d)[10] = %3 + %tmp;
; CHECK:       }
; CHECK: END REGION


;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiS_(ptr nocapture noalias noundef %a, ptr nocapture noundef readonly %b, ptr nocapture noundef noalias readonly %c, ptr nocapture noundef noalias readonly %d) {
entry:
  %arrayidx2 = getelementptr inbounds i32, ptr %a, i64 5
  %arrayidx3 = getelementptr inbounds i32, ptr %c, i64 7
  %arrayidx4 = getelementptr inbounds i32, ptr %d, i64 10
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  ret void

for.body:                                         ; preds = %entry, %if.end
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx
  %cmp1 = icmp sgt i32 %0, 10
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %1 = load i32, ptr %arrayidx2
  %add = add nsw i32 %1, 2
  store i32 %add, ptr %arrayidx2
  %2 = load i32, ptr %arrayidx3
  %add2 = add nsw i32 %2, 4
  store i32 %add2, ptr %arrayidx3
  %3 = load i32, ptr %arrayidx4
  %add3 = add nsw i32 %3, 8
  store i32 %add3, ptr %arrayidx4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}