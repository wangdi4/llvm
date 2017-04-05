; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; This command checks that -hir-ssa-deconstruction invalidates SCEV so that the parser doesn't pick up the cached version. HIR output should be the same as for the above command.
; RUN: opt < %s -hir-ssa-deconstruction -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll 2>&1 | FileCheck %s


; Check parsing output for the loop verifying that the select instruction is parsed correctly.

; CHECK: + DO i1 = 0, %n + -2, 1   <DO_LOOP>
; CHECK: |   %1 = (@x)[0][i1 + 1];
; CHECK: |   %maxval.011 = (%maxval.011 < %1) ? %1 : %maxval.011;
; CHECK: + END LOOP


; ModuleID = 'kernel24.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = common global [1000 x double] zeroinitializer, align 16
@mm = common global i64 0, align 8

define void @kernel24(i64 %n) {
entry:
  %0 = load double, double* getelementptr inbounds ([1000 x double], [1000 x double]* @x, i64 0, i64 0), align 16
  %cmp.9 = icmp sgt i64 %n, 1
  br i1 %cmp.9, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %maxval.011 = phi double [ %2, %for.body ], [ %0, %for.body.preheader ]
  %k.010 = phi i64 [ %inc, %for.body ], [ 1, %for.body.preheader ]
  %arrayidx = getelementptr inbounds [1000 x double], [1000 x double]* @x, i64 0, i64 %k.010
  %1 = load double, double* %arrayidx, align 8
  %cmp1 = fcmp olt double %maxval.011, %1
  %2 = select i1 %cmp1, double %1, double %maxval.011
  %inc = add nuw nsw i64 %k.010, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %maxval.0.lcssa = phi double [ %0, %entry ], [ %2, %for.end.loopexit ]
  store double %maxval.0.lcssa, double* getelementptr inbounds ([1000 x double], [1000 x double]* @x, i64 0, i64 0), align 16
  ret void
}
