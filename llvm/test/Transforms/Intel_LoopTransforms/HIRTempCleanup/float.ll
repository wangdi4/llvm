; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" 2>&1 | FileCheck %s

; RUN: opt < %s -opaque-pointers -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" 2>&1 | FileCheck %s

; Verify that we are able to get rid of the single loads.

; CHECK: Function

; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   %0 = (%B)[i1];
; CHECK: |   %1 = (%A)[i1];
; CHECK: |   %add = %0  +  %1;
; CHECK: |   (%A)[i1] = %add;
; CHECK: + END LOOP

; CHECK: Function

; CHECK: + DO i1 = 0, sext.i32.i64((-1 + %n)), 1   <DO_LOOP>
; CHECK: |   %add = (%B)[i1]  +  (%A)[i1];
; CHECK: |   (%A)[i1] = %add;
; CHECK: + END LOOP


; Verify HIR print functionality with new pass manager.

; This should pass with default CHECKs above.
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup" -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s

; Verify that we dump HIR with print-before-all/print-before-all.
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup" -print-before-all -print-after-all 2>&1 | FileCheck %s --check-prefix=PRINT-ALL

; PRINT-ALL-COUNT-1: HIRTempCleanup
; PRINT-ALL: DO i1

; PRINT-ALL-COUNT-1: HIRTempCleanup
; PRINT-ALL: DO i1

; Verify that we only dump HIR passes with hir-print-before-all/hir-print-after-all with the correct function filtering.
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,instcombine" -hir-print-before-all -hir-print-after-all -filter-print-funcs=foo 2>&1 | FileCheck %s --check-prefix=HIR-PRINT-ALL

; HIR-PRINT-ALL-COUNT-1: HIRTempCleanup
; HIR-PRINT-ALL: DO i1
; HIR-PRINT-ALL-NOT: InstCombine

; HIR-PRINT-ALL-COUNT-1: HIRTempCleanup
; HIR-PRINT-ALL: DO i1
; HIR-PRINT-ALL-NOT: InstCombine

; Verify that we do not dump anything with the wrong filter.
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup" -filter-print-funcs=bar -print-after-all | FileCheck %s --check-prefix=FILTER
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup" -filter-print-funcs=bar -hir-print-after-all | FileCheck %s --check-prefix=FILTER

; FILTER-NOT: Dump

; ModuleID = 'float.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(float* nocapture %A, float* nocapture readonly %B, i32 %n) {
entry:
  %cmp.7 = icmp sgt i32 %n, 0
  br i1 %cmp.7, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds float, float* %B, i64 %indvars.iv
  %0 = load float, float* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  %1 = load float, float* %arrayidx2, align 4
  %add = fadd float %0, %1
  store float %add, float* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}
