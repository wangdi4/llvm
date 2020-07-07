; RUN: opt < %s -hir-ssa-deconstruction -hir-cost-model-throttling=0 -analyze -hir-framework -hir-framework-debug=parser -hir-details | FileCheck %s

; Verify that we mark %a.0.lcssa as liveout of i2-i3 multi-exit loopnest.
; It is a phi defined in the exit block of the loopnest

; + DO i1 = 0, 9, 1   <DO_LOOP>
; |   %a.0 = 1.000000e+00;
; |   %div6268.i = 5.000000e-01;
; |
; |   + DO i2 = 0, 13, 1   <DO_MULTI_EXIT_LOOP>
; |   |   %a.0.out = %a.0;
; |   |   %div64.i = %div6268.i;
; |   |
; |   |   + DO i3 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; |   |   |   %add.i = %a.0  +  %b.1;
; |   |   |   %div6.i = %add.i  *  5.000000e-01;
; |   |   |   %add7.i = %div64.i  +  %div6.i;
; |   |   |   %a.0.lcssa = %a.0.out;
; |   |   |   if (%add7.i == %div6.i)
; |   |   |   {
; |   |   |      goto if.end.loopexit;
; |   |   |   }
; |   |   |   %div64.i = %div64.i  *  5.000000e-01;
; |   |   |   if (%add7.i >=u 1.000000e+00)
; |   |   |   {
; |   |   |      goto if.end21.i;
; |   |   |   }
; |   |   |   %a.0.lcssa = %a.0.out;
; |   |   + END LOOP
; |   |
; |   |   goto if.end.loopexit;
; |   |   if.end21.i:
; |   |   %a.0 = %div6.i;
; |   |   %div6268.i = %div6.i;
; |   + END LOOP
; |
; |   %a.1 = %div6.i;
; |   goto if.end;
; |   if.end.loopexit:
; |   %a.1 = %a.0.lcssa;
; |   if.end:
; + END LOOP

; CHECK: DO i32 i1

; CHECK: LiveOut symbases: [[DIV_SB:.*]], [[A_SB:.*]]
; CHECK: DO i32 i2

; CHECK: LiveOut symbases: [[DIV_SB:.*]], [[A_SB:.*]]
; CHECK: DO i32 i3

; CHECK: END LOOP
; CHECK: END LOOP

; CHECK: %a.1 = %div6.i;
; CHECK: <RVAL-REG> NON-LINEAR double %div6.i {sb:[[DIV_SB]]}

; CHECK: %a.1 = %a.0.lcssa;
; CHECK: <RVAL-REG> NON-LINEAR double %a.0.lcssa {sb:[[A_SB]]}


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(double %b.1) {
entry:
  br label %if.end5.lr.ph.i.preheader

if.end5.lr.ph.i.preheader:                        ; preds = %if.end, %entry
  %i.044 = phi i32 [ 1, %entry ], [ %inc, %if.end ]
  br label %if.end5.lr.ph.i

if.end5.lr.ph.i:                                  ; preds = %if.end21.i, %if.end5.lr.ph.i.preheader
  %j.044 = phi i32 [ %inc.j, %if.end21.i ], [ 1, %if.end5.lr.ph.i.preheader ]
  %a.0 = phi double [ %div6.i.lcssa56, %if.end21.i ], [ 1.000000e+00, %if.end5.lr.ph.i.preheader ]
  %div6268.i = phi double [ %div6.i.lcssa56, %if.end21.i ], [ 5.000000e-01, %if.end5.lr.ph.i.preheader ]
  br label %if.end5.i

if.end5.i:                                        ; preds = %L9.i, %if.end5.lr.ph.i
  %k.044 = phi i32 [ 1, %if.end5.lr.ph.i ], [ %inc.k, %L9.i ]
  %div64.i = phi double [ %div6268.i, %if.end5.lr.ph.i ], [ %div.i, %L9.i ]
  %add.i = fadd double %a.0, %b.1
  %div6.i = fmul double %add.i, 5.000000e-01
  %add7.i = fadd double %div64.i, %div6.i
  %cmp8.i = fcmp oeq double %add7.i, %div6.i
  br i1 %cmp8.i, label %if.end.loopexit, label %if.end10.i

if.end10.i:                                       ; preds = %if.end5.i
  %cmp19.i = fcmp olt double %add7.i, 1.000000e+00
  %div.i = fmul double %div64.i, 5.000000e-01
  %cmp3.i = fcmp olt double %div.i, 0x3EB0C6F7A0B5ED8D
  br i1 %cmp19.i, label %L9.i, label %if.end21.i

L9.i:                                             ; preds = %if.end10.i
  %inc.k = add nuw nsw i32 %k.044, 1
  %cmp4.i = icmp eq i32 %inc.k, 5
  br i1 %cmp4.i, label %if.end.loopexit, label %if.end5.i

if.end21.i:                                       ; preds = %if.end10.i
  %div6.i.lcssa56 = phi double [ %div6.i, %if.end10.i ]
  %inc.j = add nuw nsw i32 %j.044, 1
  %cmp3.i.lcssa = icmp eq i32 %inc.j, 15
  br i1 %cmp3.i.lcssa, label %if.end.loopexit55, label %if.end5.lr.ph.i

if.end.loopexit:                                  ; preds = %L9.i, %if.end5.i
  %a.0.lcssa = phi double [ %a.0, %if.end5.i ], [ %a.0, %L9.i ]
  br label %if.end

if.end.loopexit55:                                ; preds = %if.end21.i
  %div6.i.lcssa56.lcssa = phi double [ %div6.i.lcssa56, %if.end21.i ]
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit55, %if.end.loopexit
  %a.1 = phi double [ %a.0.lcssa, %if.end.loopexit ], [ %div6.i.lcssa56.lcssa, %if.end.loopexit55 ]
  %inc = add nuw nsw i32 %i.044, 1
  %exitcond = icmp eq i32 %inc, 11
  br i1 %exitcond, label %L999, label %if.end5.lr.ph.i.preheader

L999:                                             ; preds = %if.end
  %a.1.lcssa.lcssa = phi double [ %a.1, %if.end ]
  ret i32 0
}

