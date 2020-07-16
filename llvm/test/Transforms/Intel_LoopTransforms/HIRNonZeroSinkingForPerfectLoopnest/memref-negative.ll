; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-non-zero-sinking-for-perfect-loopnest -print-after=hir-non-zero-sinking-for-perfect-loopnest < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-non-zero-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;If the memref in mul inst has the same symbase as the memref in add inst, the transform cannot be triggered
;
;*** IR Dump Before HIR Non-Zero Sinking For Perfect Loopnest ***
;Function: dgemm_
;
;<0>          BEGIN REGION { }
;<35>               + DO i1 = 0, 999, 1   <DO_LOOP>
;<3>                |   %"dgemm_$B_entry[][]_fetch" = (%"dgemm_$B")[-1][i1];
;<5>                |   if (%"dgemm_$B_entry[][]_fetch" != 0.000000e+00)
;<5>                |   {
;<36>               |      + DO i2 = 0, 999, 1   <DO_LOOP>
;<17>               |      |   %mul = (%"dgemm_$C")[i1][i2]  *  %"dgemm_$B_entry[][]_fetch";
;<18>               |      |   %add = %mul  +  (%"dgemm_$C")[-1][i2];
;<19>               |      |   (%"dgemm_$C")[-1][i2] = %add;
;<36>               |      + END LOOP
;<5>                |   }
;<35>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Non-Zero Sinking For Perfect Loopnest ***
;Function: dgemm_
;
; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:           |   %"dgemm_$B_entry[][]_fetch" = (%"dgemm_$B")[-1][i1];
; CHECK:           |   if (%"dgemm_$B_entry[][]_fetch" != 0.000000e+00)
; CHECK:           |   {
; CHECK:           |      + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK:           |      |   %mul = (%"dgemm_$C")[i1][i2]  *  %"dgemm_$B_entry[][]_fetch";
; CHECK:           |      |   %add = %mul  +  (%"dgemm_$C")[-1][i2];
; CHECK:           |      |   (%"dgemm_$C")[-1][i2] = %add;
; CHECK:           |      + END LOOP
; CHECK:           |   }
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't.f90'
source_filename = "t.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @dgemm_(double* noalias nocapture readnone %"dgemm_$A", double* noalias nocapture readonly %"dgemm_$B", double* noalias nocapture %"dgemm_$C") local_unnamed_addr #0 {
alloca_0:
  %"dgemm_$B_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 8000, double* %"dgemm_$B", i64 0)
  %"dgemm_$C_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 8000, double* %"dgemm_$C", i64 0)
  br label %bb3

bb3:                                              ; preds = %bb35_endif, %alloca_0
  %indvars.iv61 = phi i64 [ %indvars.iv.next62, %bb35_endif ], [ 1, %alloca_0 ]
  %"dgemm_$B_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"dgemm_$B_entry[]", i64 %indvars.iv61)
  %"dgemm_$B_entry[][]_fetch" = load double, double* %"dgemm_$B_entry[][]", align 1
  %rel = fcmp fast ueq double %"dgemm_$B_entry[][]_fetch", 0.000000e+00
  br i1 %rel, label %bb35_endif, label %bb17.preheader

bb17.preheader:                                   ; preds = %bb3
  %"dgemm_$C_entry[]10" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 8000, double* %"dgemm_$C", i64 %indvars.iv61)
  br label %bb17

bb17:                                             ; preds = %bb17, %bb17.preheader
  %indvars.iv = phi i64 [ 1, %bb17.preheader ], [ %indvars.iv.next, %bb17 ]
  %"dgemm_$C_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull %"dgemm_$C_entry[]", i64 %indvars.iv)
  %"dgemm_$C_entry[][]_fetch" = load double, double* %"dgemm_$C_entry[][]", align 1
  %"dgemm_$C_entry[]10[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* %"dgemm_$C_entry[]10", i64 %indvars.iv)
  %"dgemm_$C_entry[]10[]_fetch" = load double, double* %"dgemm_$C_entry[]10[]", align 1
  %mul = fmul fast double %"dgemm_$C_entry[]10[]_fetch", %"dgemm_$B_entry[][]_fetch"
  %add = fadd fast double %mul, %"dgemm_$C_entry[][]_fetch"
  store double %add, double* %"dgemm_$C_entry[][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1001
  br i1 %exitcond, label %bb35_endif.loopexit, label %bb17

bb35_endif.loopexit:                              ; preds = %bb17
  br label %bb35_endif

bb35_endif:                                       ; preds = %bb35_endif.loopexit, %bb3
  %indvars.iv.next62 = add nuw nsw i64 %indvars.iv61, 1
  %exitcond63 = icmp eq i64 %indvars.iv.next62, 1001
  br i1 %exitcond63, label %bb1, label %bb3

bb1:                                              ; preds = %bb35_endif
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
