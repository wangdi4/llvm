; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir>"  2>&1 < %s | FileCheck %s

; This test case checks that the reduction inside the If condition is
; identified, and the temporary is created inside a new If. This case
; handles when there is another possible dependency, but the opcode can
; still be handled as commutative (one instruction is an add and the
; other is a subtract).

; HIR before transformation:

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %0 = (i64*)(%b)[i1];
;       |   if (%n > 10)
;       |   {
;       |      %add = (%a)[5]  +  2.000000e+00;
;       |      (%a)[5] = %add;
;       |      %sub = (%a)[%0]  -  %m;
;       |      (%a)[%0] = %sub;
;       |   }
;       + END LOOP
; END REGION


; HIR after transformation

; CHECK: BEGIN REGION { modified }
; CHECK:          %tmp = 0.000000e+00;
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   %0 = (i64*)(%b)[i1];
; CHECK:       |   if (%n > 10)
; CHECK:       |   {
; CHECK:       |      %tmp = %tmp  +  2.000000e+00;
; CHECK:       |      %sub = (%a)[%0]  -  %m;
; CHECK:       |      (%a)[%0] = %sub;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK:       if (%tmp !=u 0.000000e+00)
; CHECK:       {
; CHECK:          %add = (%a)[5]  +  %tmp;
; CHECK:          (%a)[5] = %add;
; CHECK:       }
; CHECK: END REGION

;Module Before HIR
; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3fooPiS_(ptr nocapture noalias noundef %a, ptr nocapture noundef readonly %b, i32 %n, float %m) {
entry:
  %arrayidx2 = getelementptr inbounds float, ptr %a, i64 5
  br label %for.body

for.cond.cleanup:                                 ; preds = %if.end
  ret void

for.body:                                         ; preds = %entry, %if.end
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %if.end ]
  %arrayidx = getelementptr inbounds float, ptr %b, i64 %indvars.iv
  %0 = load i64, ptr %arrayidx
  %arrayidx3 = getelementptr inbounds float, ptr %a, i64 %0
  %cmp1 = icmp sgt i32 %n, 10
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %1 = load float, ptr %arrayidx2
  %add = fadd fast float %1, 2.0
  store float %add, ptr %arrayidx2
  %2 = load float, ptr %arrayidx3
  %sub = fsub fast float %2, %m
  store float %sub, ptr %arrayidx3
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body
}