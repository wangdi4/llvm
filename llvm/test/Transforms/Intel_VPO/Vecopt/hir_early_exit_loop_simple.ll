; Test to verify that HIRParVecAnalysis and VPlan HIR vectorizer can handle
; simple early exit loops.

; RUN: opt %s -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec" -print-before=hir-vplan-vec -vplan-enable-early-exit-loops -disable-output 2>&1 | FileCheck %s

; CHECK:  BEGIN REGION { }
; CHECK:        %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC() ]
; CHECK:        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:        |   %1 = (%a)[i1];
; CHECK:        |   if (%1 == %val)
; CHECK:        |   {
; CHECK:        |      goto cleanup.loopexit;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:        @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK:  END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @_Z3fooiPKaPaa(i32 %n, ptr nocapture readonly %a, i8 signext %val) local_unnamed_addr {
entry:
  %cmp8 = icmp sgt i32 %n, 0
  br i1 %cmp8, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i8, ptr %a, i64 %indvars.iv
  %1 = load i8, ptr %arrayidx, align 1
  %cmp2 = icmp eq i8 %1, %val
  br i1 %cmp2, label %cleanup.loopexit, label %for.inc

for.inc:                                          ; preds = %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %cleanup.loopexit

cleanup.loopexit:                                 ; preds = %for.inc
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %entry
  ret i32 0
}
