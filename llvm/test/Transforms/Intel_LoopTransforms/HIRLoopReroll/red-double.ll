; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; CHECK: Function: foo
;
; CHECK:         BEGIN REGION { }
; CHECK:                + DO i1 = 0, 499, 1   <DO_LOOP>
; CHECK:                |   %add = %S.013  +  (@A)[0][2 * i1];
; CHECK:                |   %S.013 = %add  +  (@A)[0][2 * i1 + 1];
; CHECK:                + END LOOP
; CHECK:          END REGION
;
; CHECK: Function: foo
;
; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:               |   %S.013 = %S.013  +  (@A)[0][i1];
; CHECK:               + END LOOP
; CHECK:         END REGION

;Module Before HIR; ModuleID = 'red-unrolled.c'
source_filename = "red-unrolled.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [1000 x double] zeroinitializer, align 16

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local double @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %add4.lcssa = phi double [ %add4, %for.body ]
  ret double %add4.lcssa

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %S.013 = phi double [ 0.000000e+00, %entry ], [ %add4, %for.body ]
  %arrayidx = getelementptr inbounds [1000 x double], ptr @A, i64 0, i64 %indvars.iv
  %0 = load double, ptr %arrayidx, align 16
  %add = fadd double %S.013, %0
  %1 = or i64 %indvars.iv, 1
  %arrayidx3 = getelementptr inbounds [1000 x double], ptr @A, i64 0, i64 %1
  %2 = load double, ptr %arrayidx3, align 8
  %add4 = fadd double %add, %2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 2
  %cmp = icmp ult i64 %indvars.iv.next, 1000
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}



