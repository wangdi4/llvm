; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir>" 2>&1 < %s | FileCheck %s

; This test case checks that the reductions inside the multiple cases are
; identified, and the temporary is created inside a new Ifs. There should be
; an If for each case.

; HIR before transformation:

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   switch((%b)[i1])
;       |   {
;       |   case 10:
;       |      %1 = (%a)[5];
;       |      (%a)[5] = %1 + 2;
;       |      break;
;       |   case 12:
;       |      %2 = (%a)[5];
;       |      (%a)[5] = %2 + 4;
;       |      break;
;       |   default:
;       |      %3 = (%a)[5];
;       |      (%a)[5] = %3 + 8;
;       |      break;
;       |   }
;       + END LOOP
; END REGION

; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:          %tmp = 0;
; CHECK:          %tmp6 = 0;
; CHECK:          %tmp8 = 0;
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   switch((%b)[i1])
; CHECK:       |   {
; CHECK:       |   case 10:
; CHECK:       |      %tmp8 = %tmp8  +  2;
; CHECK:       |      break;
; CHECK:       |   case 12:
; CHECK:       |      %tmp6 = %tmp6  +  4;
; CHECK:       |      break;
; CHECK:       |   default:
; CHECK:       |      %tmp = %tmp  +  8;
; CHECK:       |      break;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK:       if (%tmp8 != 0)
; CHECK:       {
; CHECK:          %1 = (%a)[5];
; CHECK:          (%a)[5] = %1 + %tmp8;
; CHECK:       }
; CHECK:       if (%tmp6 != 0)
; CHECK:       {
; CHECK:          %2 = (%a)[5];
; CHECK:          (%a)[5] = %2 + %tmp6;
; CHECK:       }
; CHECK:       if (%tmp != 0)
; CHECK:       {
; CHECK:          %3 = (%a)[5];
; CHECK:          (%a)[5] = %3 + %tmp;
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
  br label %for.inc

sw.bb1:                                         ; preds = %for.body
  %2 = load i32, ptr %arrayidx2
  %add2 = add nsw i32 %2, 4
  store i32 %add2, ptr %arrayidx2
  br label %for.inc

sw.default:                                     ; preds = %for.body
  %3 = load i32, ptr %arrayidx2
  %add3 = add nsw i32 %3, 8
  store i32 %add3, ptr %arrayidx2
  br label %for.inc

for.inc:                                           ; preds = %sw.bb, %sw.bb1, %sw.default
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}