; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-non-zero-sinking-for-perfect-loopnest -print-after=hir-non-zero-sinking-for-perfect-loopnest < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-non-zero-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Non-Zero Sinking For Perfect Loopnest ***
;Function: dgemm_
;
;<0>          BEGIN REGION { }
;<38>               + DO i1 = 0, 999, 1   <DO_LOOP>
;<3>                |   %"dgemm_$B_entry[][]_fetch" = (%"dgemm_$B")[-1][i1];
;<5>                |   if (%"dgemm_$B_entry[][]_fetch" != 0.000000e+00)
;<5>                |   {
;<11>               |      %mul = %"dgemm_$B_entry[][]_fetch"  *  (%"dgemm_$D")[-1][i1];
;<39>               |
;<39>               |      + DO i2 = 0, 999, 1   <DO_LOOP>
;<20>               |      |   %mul18 = %mul  *  (%"dgemm_$A")[i1][i2];
;<21>               |      |   %add = (%"dgemm_$C")[-1][i2]  +  %mul18;
;<22>               |      |   (%"dgemm_$C")[-1][i2] = %add;
;<39>               |      + END LOOP
;<5>                |   }
;<38>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Non-Zero Sinking For Perfect Loopnest ***
;Function: dgemm_
;
; CHECK:      BEGIN REGION { }
; CHECK:            + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:            |   %mul = (%"dgemm_$B")[-1][i1]  *  (%"dgemm_$D")[-1][i1];
; CHECK:            |
; CHECK:            |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK:            |   |   %mul18 = %mul  *  (%"dgemm_$A")[i1][i2];
; CHECK:            |   |   %add = (%"dgemm_$C")[-1][i2]  +  %mul18;
; CHECK:            |   |   (%"dgemm_$C")[-1][i2] = %add;
; CHECK:            |   + END LOOP
; CHECK:           + END LOOP
; CHECK:      END REGION
;
;Module Before HIR
; ModuleID = 't.f90'
source_filename = "t.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @dgemm_(double* noalias nocapture readonly %"dgemm_$A", double* noalias nocapture readonly %"dgemm_$B", double* noalias nocapture %"dgemm_$C", double* noalias nocapture readonly %"dgemm_$D") local_unnamed_addr #0 {
alloca_0:
  %"dgemm_$B_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 8000, double* %"dgemm_$B", i64 0)
  %"dgemm_$D_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 8000, double* %"dgemm_$D", i64 0)
  %"dgemm_$C_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 8000, double* %"dgemm_$C", i64 0)
  br label %bb3

bb3:                                              ; preds = %bb40_endif, %alloca_0
  %indvars.iv73 = phi i64 [ %indvars.iv.next74, %bb40_endif ], [ 1, %alloca_0 ]
  %"dgemm_$B_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"dgemm_$B_entry[]", i64 %indvars.iv73)
  %"dgemm_$B_entry[][]_fetch" = load double, double* %"dgemm_$B_entry[][]", align 1
  %rel = fcmp ueq double %"dgemm_$B_entry[][]_fetch", 0.000000e+00
  br i1 %rel, label %bb40_endif, label %bb12_then

bb22:                                             ; preds = %bb22, %bb12_then
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb22 ], [ 1, %bb12_then ]
  %"dgemm_$C_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"dgemm_$C_entry[]", i64 %indvars.iv)
  %"dgemm_$C_entry[][]_fetch" = load double, double* %"dgemm_$C_entry[][]", align 1
  %"dgemm_$A_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* %"dgemm_$A_entry[]", i64 %indvars.iv)
  %"dgemm_$A_entry[][]_fetch" = load double, double* %"dgemm_$A_entry[][]", align 1
  %mul18 = fmul double %mul, %"dgemm_$A_entry[][]_fetch"
  %add = fadd double %"dgemm_$C_entry[][]_fetch", %mul18
  store double %add, double* %"dgemm_$C_entry[][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1001
  br i1 %exitcond, label %bb40_endif.loopexit, label %bb22

bb12_then:                                        ; preds = %bb3
  %"dgemm_$D_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"dgemm_$D_entry[]", i64 %indvars.iv73)
  %"dgemm_$D_entry[][]_fetch" = load double, double* %"dgemm_$D_entry[][]", align 1
  %mul = fmul double %"dgemm_$B_entry[][]_fetch", %"dgemm_$D_entry[][]_fetch"
  %"dgemm_$A_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 8000, double* %"dgemm_$A", i64 %indvars.iv73)
  br label %bb22

bb40_endif.loopexit:                              ; preds = %bb22
  br label %bb40_endif

bb40_endif:                                       ; preds = %bb40_endif.loopexit, %bb3
  %indvars.iv.next74 = add nuw nsw i64 %indvars.iv73, 1
  %exitcond75 = icmp eq i64 %indvars.iv.next74, 1001
  br i1 %exitcond75, label %bb1, label %bb3

bb1:                                              ; preds = %bb40_endif
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
