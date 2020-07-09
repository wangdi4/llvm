; This transformation cannot be trigged because there is a (%"dgemm_$D")[i1][i3] = %int_cast store instruction
; in the innermost loop guarded by an if condition.
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-non-zero-sinking-for-perfect-loopnest -print-after=hir-non-zero-sinking-for-perfect-loopnest < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-non-zero-sinking-for-perfect-loopnest,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Non-Zero Sinking For Perfect Loopnest ***
;Function: dgemm_
;
;<0>          BEGIN REGION { }
;<52>               + DO i1 = 0, 999, 1   <DO_LOOP>
;<53>               |   + DO i2 = 0, 999, 1   <DO_LOOP>
;<9>                |   |   %"dgemm_$B_entry[][]_fetch" = (%"dgemm_$B")[i1][i2];
;<11>               |   |   if (%"dgemm_$B_entry[][]_fetch" != 0.000000e+00)
;<11>               |   |   {
;<54>               |   |      + DO i3 = 0, 999, 1   <DO_LOOP>
;<23>               |   |      |   %mul = (%"dgemm_$A")[i2][i3]  *  %"dgemm_$B_entry[][]_fetch";
;<24>               |   |      |   %sub = (%"dgemm_$C")[i1][i3]  -  %mul;
;<25>               |   |      |   (%"dgemm_$C")[i1][i3] = %sub;
;<27>               |   |      |   %int_cast = sitofp.i32.double(i3 + 1);
;<29>               |   |      |   (%"dgemm_$D")[i1][i3] = %int_cast;
;<54>               |   |      + END LOOP
;<11>               |   |   }
;<53>               |   + END LOOP
;<52>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Non-Zero Sinking For Perfect Loopnest ***
;Function: dgemm_
;
; CHECK:    BEGIN REGION { }
; CHECK:           + DO i1 = 0, 999, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 999, 1   <DO_LOOP>
; CHECK:           |   |   %"dgemm_$B_entry[][]_fetch" = (%"dgemm_$B")[i1][i2];
; CHECK:           |   |   if (%"dgemm_$B_entry[][]_fetch" != 0.000000e+00)
; CHECK:           |   |   {
; CHECK:           |   |      + DO i3 = 0, 999, 1   <DO_LOOP>
; CHECK:           |   |      |   %mul = (%"dgemm_$A")[i2][i3]  *  %"dgemm_$B_entry[][]_fetch";
; CHECK:           |   |      |   %sub = (%"dgemm_$C")[i1][i3]  -  %mul;
; CHECK:           |   |      |   (%"dgemm_$C")[i1][i3] = %sub;
; CHECK:           |   |      |   %int_cast = sitofp.i32.double(i3 + 1);
; CHECK:           |   |      |   (%"dgemm_$D")[i1][i3] = %int_cast;
; CHECK:           |   |      + END LOOP
; CHECK:           |   |   }
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 'dgemm.f90'
source_filename = "dgemm.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @dgemm_(double* noalias nocapture readonly %"dgemm_$A", double* noalias nocapture readonly %"dgemm_$B", double* noalias nocapture %"dgemm_$C", double* noalias nocapture %"dgemm_$D") local_unnamed_addr #0 {
alloca_0:
  br label %bb3

bb3:                                              ; preds = %bb8, %alloca_0
  %indvars.iv84 = phi i64 [ %indvars.iv.next85, %bb8 ], [ 1, %alloca_0 ]
  %"dgemm_$B_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 8000, double* %"dgemm_$B", i64 %indvars.iv84)
  %"dgemm_$C_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 8000, double* %"dgemm_$C", i64 %indvars.iv84)
  %"dgemm_$D_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 8000, double* %"dgemm_$D", i64 %indvars.iv84)
  br label %bb7

bb7:                                              ; preds = %bb45_endif, %bb3
  %indvars.iv81 = phi i64 [ %indvars.iv.next82, %bb45_endif ], [ 1, %bb3 ]
  %"dgemm_$B_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* %"dgemm_$B_entry[]", i64 %indvars.iv81)
  %"dgemm_$B_entry[][]_fetch" = load double, double* %"dgemm_$B_entry[][]", align 1
  %rel = fcmp fast ueq double %"dgemm_$B_entry[][]_fetch", 0.000000e+00
  br i1 %rel, label %bb45_endif, label %bb21.preheader

bb21.preheader:                                   ; preds = %bb7
  %"dgemm_$A_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 8000, double* %"dgemm_$A", i64 %indvars.iv81)
  br label %bb21

bb21:                                             ; preds = %bb21, %bb21.preheader
  %indvars.iv = phi i64 [ 1, %bb21.preheader ], [ %indvars.iv.next, %bb21 ]
  %"dgemm_$C_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* %"dgemm_$C_entry[]", i64 %indvars.iv)
  %"dgemm_$C_entry[][]_fetch" = load double, double* %"dgemm_$C_entry[][]", align 1
  %"dgemm_$A_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* %"dgemm_$A_entry[]", i64 %indvars.iv)
  %"dgemm_$A_entry[][]_fetch" = load double, double* %"dgemm_$A_entry[][]", align 1
  %mul = fmul fast double %"dgemm_$A_entry[][]_fetch", %"dgemm_$B_entry[][]_fetch"
  %sub = fsub fast double %"dgemm_$C_entry[][]_fetch", %mul
  store double %sub, double* %"dgemm_$C_entry[][]", align 1
  %0 = trunc i64 %indvars.iv to i32
  %int_cast = sitofp i32 %0 to double
  %"dgemm_$D_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* %"dgemm_$D_entry[]", i64 %indvars.iv)
  store double %int_cast, double* %"dgemm_$D_entry[][]", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1001
  br i1 %exitcond, label %bb45_endif.loopexit, label %bb21

bb45_endif.loopexit:                              ; preds = %bb21
  br label %bb45_endif

bb45_endif:                                       ; preds = %bb45_endif.loopexit, %bb7
  %indvars.iv.next82 = add nuw nsw i64 %indvars.iv81, 1
  %exitcond83 = icmp eq i64 %indvars.iv.next82, 1001
  br i1 %exitcond83, label %bb8, label %bb7

bb8:                                              ; preds = %bb45_endif
  %indvars.iv.next85 = add nuw nsw i64 %indvars.iv84, 1
  %exitcond86 = icmp eq i64 %indvars.iv.next85, 1001
  br i1 %exitcond86, label %bb1, label %bb3

bb1:                                              ; preds = %bb8
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
