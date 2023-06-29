; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking,print<hir>" 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-memory-reduction-sinking" -print-changed -disable-output 2>&1 < %s | FileCheck %s --check-prefix=CHECK-CHANGED

; This test checks that the memory reduction sinking wasn't applied since
; the operation is a subtraction, which is not commutative.

; HIR before transformation

; + DO i1 = 0, 999, 1   <DO_LOOP>
; |   %add.6 = (%B)[i1]  -  (%A)[i1 + 1];
; |   (%A)[i1 + 1] = %add.6;
; |   %add.7 = (%B)[i1 + 1]  -  (%A)[%m];
; |   (%A)[%m] = %add.7;
; + END LOOP


; HIR after transformation

; CHECK: + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK: |   %add.6 = (%B)[i1]  -  (%A)[i1 + 1];
; CHECK: |   (%A)[i1 + 1] = %add.6;
; CHECK: |   %add.7 = (%B)[i1 + 1]  -  (%A)[%m];
; CHECK: |   (%A)[%m] = %add.7;
; CHECK: + END LOOP

; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRMemoryReductionSinking

;Module Before HIR
; ModuleID = 'special.f90'
source_filename = "special.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @specailsum_(ptr noalias nocapture dereferenceable(4) %"A", ptr noalias nocapture readonly dereferenceable(4) %"B", ptr noalias nocapture readonly dereferenceable(4) %"specailsum_$M") local_unnamed_addr #0 {
alloca_1:
  %"m" = load i32, ptr %"specailsum_$M"
  %int_sext3 = sext i32 %"m" to i64
  %"A[]4" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"A", i64 %int_sext3)
  br label %do.body18

do.body18:                                        ; preds = %do.body18, %alloca_1
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body18 ], [ 1, %alloca_1 ]
  %"A[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"A", i64 %indvars.iv)
  %"A[]_fetch.32" = load float, ptr %"A[]"
  %"B[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"B", i64 %indvars.iv)
  %"B[]_fetch.37" = load float, ptr %"B[]"
  %add.6 = fsub float %"B[]_fetch.37", %"A[]_fetch.32"
  store float %add.6, ptr %"A[]"
  %"A[]_fetch.35" = load float, ptr %"A[]4"
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %"B[]1" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(float) %"B", i64 %indvars.iv.next)
  %"B[]_fetch.38" = load float, ptr %"B[]1"
  %add.7 = fsub float %"B[]_fetch.38", %"A[]_fetch.35"
  store float %add.7, ptr %"A[]4"
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1001
  br i1 %exitcond.not, label %do.epilog21, label %do.body18

do.epilog21:                                      ; preds = %do.body18
  ret void
}

attributes #0 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
