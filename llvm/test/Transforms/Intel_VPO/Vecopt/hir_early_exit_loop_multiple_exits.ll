; Test to verify that HIRParVecAnalysis deems an early exit loop
; with multiple exits as unsafe to vectorize.

; REQUIRES: asserts
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-vec-dir-insert" -print-after=hir-vec-dir-insert -vplan-enable-early-exit-loops -debug-only=parvec-analysis -disable-output 2>&1 | FileCheck %s
; RUN: opt %s -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-cg,simplifycfg,intel-ir-optreport-emitter" -vplan-enable-early-exit-loops -hir-enable-parvec-diag -disable-output 2>&1 | FileCheck %s --check-prefix=OPTRPT

; CHECK: EarlyExitVecSafety: Multiple early exits is not supported.

; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, sext.i32.i64(%n) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:        |   %ld = (%a)[i1];
; CHECK:        |   if (%ld != %val1)
; CHECK:        |   {
; CHECK:        |      goto cleanup.loopexit;
; CHECK:        |   }
; CHECK:        |   if (%ld != %val2)
; CHECK:        |   {
; CHECK:        |      goto cleanup.loopexit;
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION

; OPTRPT: #15579: Multi-exit loop is not safe to vectorize or has unsupported structure.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local void @_Z3fooiPKaPaa(i32 %n, ptr nocapture readonly %a, i8 signext %val1, i8 signext %val2) local_unnamed_addr {
entry:
  %cmp8 = icmp sgt i32 %n, 0
  br i1 %cmp8, label %for.body.preheader, label %cleanup

for.body.preheader:                               ; preds = %entry
  %0 = sext i32 %n to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %arrayidx = getelementptr inbounds i8, ptr %a, i64 %indvars.iv
  %ld = load i8, ptr %arrayidx, align 1
  %cmp1 = icmp eq i8 %ld, %val1
  br i1 %cmp1, label %sibling, label %cleanup.loopexit

sibling:                                          ; preds = %for.body
  %cmp2 = icmp eq i8 %ld, %val2
  br i1 %cmp2, label %for.inc, label %cleanup.loopexit

for.inc:                                          ; preds = %sibling
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %0
  br i1 %cmp, label %for.body, label %cleanup.loopexit

cleanup.loopexit:                                 ; preds = %for.inc, %for.body, %sibling
  br label %cleanup

cleanup:                                          ; preds = %cleanup.loopexit, %entry
  ret void
}
