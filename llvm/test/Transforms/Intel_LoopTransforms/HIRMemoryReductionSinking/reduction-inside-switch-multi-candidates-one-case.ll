; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir>" 2>&1 < %s | FileCheck %s

; This test case checks that the multiple reductions inside one case are
; identified, and the temps are created inside a new If.

; HIR before transformation:

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   switch((%b)[i1])
;       |   {
;       |   case 10:
;       |      %1 = (%a)[5];
;       |      (%a)[5] = %1 + 2;
;       |      %2 = (%a)[7];
;       |      (%a)[7] = %2 + 4;
;       |      %3 = (%a)[9];
;       |      (%a)[9] = %3 + 8;
;       |      break;
;       |   case 12:
;       |      (%b)[i1] = 3;
;       |      break;
;       |   default:
;       |      (%b)[i1] = 4;
;       |      break;
;       |   }
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:          %tmp = 0;
; CHECK:          %tmp6 = 0;
; CHECK:          %tmp9 = 0;
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   switch((%b)[i1])
; CHECK:       |   {
; CHECK:       |   case 10:
; CHECK:       |      %tmp9 = %tmp9  +  2;
; CHECK:       |      %tmp6 = %tmp6  +  4;
; CHECK:       |      %tmp = %tmp  +  8;
; CHECK:       |      break;
; CHECK:       |   case 12:
; CHECK:       |      (%b)[i1] = 3;
; CHECK:       |      break;
; CHECK:       |   default:
; CHECK:       |      (%b)[i1] = 4;
; CHECK:       |      break;
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
; CHECK:          %2 = (%a)[7];
; CHECK:          (%a)[7] = %2 + %tmp6;
; CHECK:          %3 = (%a)[9];
; CHECK:          (%a)[9] = %3 + %tmp;
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
  %arrayidx3 = getelementptr inbounds i32, ptr %a, i64 7
  %arrayidx4 = getelementptr inbounds i32, ptr %a, i64 9
  %ret = alloca i32
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.inc
  ret void

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i32, ptr %b, i64 %indvars.iv
  %0 = load i32, ptr %arrayidx
  switch i32 %0, label %sw.default [
    i32 10, label %sw.bb
    i32 12, label %sw.bb1
  ]

sw.bb:                                          ; preds = %for.body
  %1 = load i32, ptr %arrayidx2
  %add = add nsw i32 %1, 2
  store i32 %add, ptr %arrayidx2
  %2 = load i32, ptr %arrayidx3
  %add2 = add nsw i32 %2, 4
  store i32 %add2, ptr %arrayidx3
  %3 = load i32, ptr %arrayidx4
  %add3 = add nsw i32 %3, 8
  store i32 %add3, ptr %arrayidx4
  br label %for.inc

sw.bb1:                                         ; preds = %for.body
  store i32 3, ptr %arrayidx
  br label %for.inc

sw.default:                                     ; preds = %for.body
  store i32 4, ptr %arrayidx
  br label %for.inc

for.inc:                                           ; preds = %sw.bb, %sw.bb1, %sw.default
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}