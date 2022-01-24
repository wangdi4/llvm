; RUN: opt -hir-ssa-deconstruction -hir-create-function-level-region -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
;
; R_N: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" -hir-create-function-level-region -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -S -disable-output 2>&1 < %s | FileCheck %s
; [Note]
; - Run the testcase under the new pass manager is disabled due to JIRA-18272 (leftover dummygep instruction failed LLVM verifier).
;   Once the JIRA closes, the new pass-manager execution path will be restored.
;
; FORTRAN Source Code:
;subroutine sub(A, B, N)
;  real*8 B(N,N)
;  real*8 C(N,N)
;
;  !C = 2.0    !  C   not needed
;  C = B +1
;  s = 0
;  do i=1,5
;    do j=1,5
;      s = s + C(j,i)
;    enddo
;  enddo
;  print*, s
;end
;
;*** IR Dump Before HIR Dead Store Elimination ***
; [Note]
; This region has both a loop and non-loop code.

; CHECK:       BEGIN REGION { }
; CHECK:           + DO i1 = 0, sext.i32.i64(%"sub_$N_fetch") + -1, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%"sub_$N_fetch") + -1, 1   <DO_LOOP>
; CHECK:           |   |   %"sub_$B[][]_fetch" = (%"sub_$B")[i1][i2];
; CHECK:           |   |   %add35 = %"sub_$B[][]_fetch"  +  1.000000e+00;
; CHECK:           |   |   (%"sub_$C")[i1][i2] = %add35;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
;
; CHECK:           %"sub_$S.0" = 0.000000e+00;
;
; CHECK:           + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   |   %float_cast76 = fpext.float.double(%"sub_$S.0");
; CHECK:           |   |   %"sub_$C[]71[]_fetch" = (%"sub_$C")[i1][i2];
; CHECK:           |   |   %add75 = %"sub_$C[]71[]_fetch"  +  %float_cast76;
; CHECK:           |   |   %"sub_$S.0" = fptrunc.double.float(%add75);
; CHECK:           |   + END LOOP
; CHECK:           |
; CHECK:           |   %"sub_$S.0.out" = %"sub_$S.0";
; CHECK:           + END LOOP
;
; CHECK:           (%addressof)[0][0] = 26;
; CHECK:           (%addressof)[0][1] = 1;
; CHECK:           (%addressof)[0][2] = 1;
; CHECK:           (%addressof)[0][3] = 0;
; CHECK:           (%ARGBLOCK_0)[0].0 = %"sub_$S.0.out";
; CHECK:           %func_result102 = @for_write_seq_lis(&((i8*)(%"var$1")[0]),  -1,  1239157112576,  &((%addressof)[0][0]),  &((i8*)(%ARGBLOCK_0)[0]));
; CHECK:           ret ;
; CHECK:     END REGION

;*** IR Dump After HIR Dead Store Elimination ***

; CHECK:       BEGIN REGION { }
; CHECK:           + DO i1 = 0, sext.i32.i64(%"sub_$N_fetch") + -1, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, sext.i32.i64(%"sub_$N_fetch") + -1, 1   <DO_LOOP>
; CHECK:           |   |   %"sub_$B[][]_fetch" = (%"sub_$B")[i1][i2];
; CHECK:           |   |   %add35 = %"sub_$B[][]_fetch"  +  1.000000e+00;
; CHECK:           |   |   (%"sub_$C")[i1][i2] = %add35;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
;
; CHECK:           %"sub_$S.0" = 0.000000e+00;
;
; CHECK:           + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   |   %float_cast76 = fpext.float.double(%"sub_$S.0");
; CHECK:           |   |   %"sub_$C[]71[]_fetch" = (%"sub_$C")[i1][i2];
; CHECK:           |   |   %add75 = %"sub_$C[]71[]_fetch"  +  %float_cast76;
; CHECK:           |   |   %"sub_$S.0" = fptrunc.double.float(%add75);
; CHECK:           |   + END LOOP
; CHECK:           |
; CHECK:           |   %"sub_$S.0.out" = %"sub_$S.0";
; CHECK:           + END LOOP
;
; CHECK:           (%addressof)[0][0] = 26;
; CHECK:           (%addressof)[0][1] = 1;
; CHECK:           (%addressof)[0][2] = 1;
; CHECK:           (%addressof)[0][3] = 0;
; CHECK:           (%ARGBLOCK_0)[0].0 = %"sub_$S.0.out";
; CHECK:           %func_result102 = @for_write_seq_lis(&((i8*)(%"var$1")[0]),  -1,  1239157112576,  &((%addressof)[0][0]),  &((i8*)(%ARGBLOCK_0)[0]));
; CHECK:           ret ;
; CHECK:     END REGION

;Module Before HIR
; ModuleID = 'non-dead-local-store.f90'
source_filename = "non-dead-local-store.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @sub_(float* noalias nocapture readnone %"sub_$A", double* noalias nocapture readonly %"sub_$B", i32* noalias nocapture readonly %"sub_$N") local_unnamed_addr #0 {
alloca:
  %"var$1" = alloca [8 x i64], align 16
  %addressof = alloca [4 x i8], align 1
  %ARGBLOCK_0 = alloca { float }, align 8
  %"sub_$N_fetch" = load i32, i32* %"sub_$N", align 4
  %int_sext = sext i32 %"sub_$N_fetch" to i64
  %rel = icmp sgt i64 %int_sext, 0
  %slct = select i1 %rel, i64 %int_sext, i64 0
  %mul = shl nuw nsw i64 %slct, 3
  %mul10 = mul nsw i64 %mul, %slct
  %div = lshr exact i64 %mul10, 3
  %"sub_$C" = alloca double, i64 %div, align 8
  %mul13 = shl nsw i64 %int_sext, 3
  %rel48126 = icmp slt i32 %"sub_$N_fetch", 1
  br i1 %rel48126, label %bb32.preheader, label %bb24.preheader.preheader

bb24.preheader.preheader:                         ; preds = %alloca
  %0 = add nsw i64 %int_sext, 1
  br label %bb24.preheader

bb19:                                             ; preds = %bb24.preheader, %bb19
  %"var$3.0125" = phi i64 [ 1, %bb24.preheader ], [ %add44, %bb19 ]
  %"sub_$B[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"sub_$B[]", i64 %"var$3.0125")
  %"sub_$B[][]_fetch" = load double, double* %"sub_$B[][]", align 8
  %add35 = fadd double %"sub_$B[][]_fetch", 1.000000e+00
  %"sub_$C[][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub_$C[]", i64 %"var$3.0125")
  store double %add35, double* %"sub_$C[][]", align 8
  %add44 = add nuw nsw i64 %"var$3.0125", 1
  %exitcond132 = icmp eq i64 %add44, %0
  br i1 %exitcond132, label %bb26, label %bb19

bb26:                                             ; preds = %bb19
  %add52 = add nuw nsw i64 %"var$4.0127", 1
  %exitcond133 = icmp eq i64 %add52, %0
  br i1 %exitcond133, label %bb32.preheader.loopexit, label %bb24.preheader

bb24.preheader:                                   ; preds = %bb24.preheader.preheader, %bb26
  %"var$4.0127" = phi i64 [ %add52, %bb26 ], [ 1, %bb24.preheader.preheader ]
  %"sub_$B[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul13, double* elementtype(double) %"sub_$B", i64 %"var$4.0127")
  %"sub_$C[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul13, double* elementtype(double) nonnull %"sub_$C", i64 %"var$4.0127")
  br label %bb19

bb32.preheader.loopexit:                          ; preds = %bb26
  br label %bb32.preheader

bb32.preheader:                                   ; preds = %bb32.preheader.loopexit, %alloca
  br label %bb32

bb32:                                             ; preds = %bb37, %bb32.preheader
  %indvars.iv129 = phi i64 [ 1, %bb32.preheader ], [ %indvars.iv.next130, %bb37 ]
  %"sub_$S.0" = phi float [ 0.000000e+00, %bb32.preheader ], [ %float_cast.lcssa, %bb37 ]
  %"sub_$C[]71" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul13, double* elementtype(double) nonnull %"sub_$C", i64 %indvars.iv129)
  br label %bb36

bb36:                                             ; preds = %bb36, %bb32
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb36 ], [ 1, %bb32 ]
  %"sub_$S.1" = phi float [ %float_cast, %bb36 ], [ %"sub_$S.0", %bb32 ]
  %float_cast76 = fpext float %"sub_$S.1" to double
  %"sub_$C[]71[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub_$C[]71", i64 %indvars.iv)
  %"sub_$C[]71[]_fetch" = load double, double* %"sub_$C[]71[]", align 8
  %add75 = fadd double %"sub_$C[]71[]_fetch", %float_cast76
  %float_cast = fptrunc double %add75 to float
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond, label %bb37, label %bb36

bb37:                                             ; preds = %bb36
  %float_cast.lcssa = phi float [ %float_cast, %bb36 ]
  %indvars.iv.next130 = add nuw nsw i64 %indvars.iv129, 1
  %exitcond131 = icmp eq i64 %indvars.iv.next130, 6
  br i1 %exitcond131, label %bb46, label %bb32

bb46:                                             ; preds = %bb37
  %float_cast.lcssa.lcssa = phi float [ %float_cast.lcssa, %bb37 ]
  %.fca.0.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 0
  store i8 26, i8* %.fca.0.gep, align 1
  %.fca.1.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 1
  store i8 1, i8* %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 2
  store i8 1, i8* %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 3
  store i8 0, i8* %.fca.3.gep, align 1
  %BLKFIELD_ = getelementptr inbounds { float }, { float }* %ARGBLOCK_0, i64 0, i32 0
  store float %float_cast.lcssa.lcssa, float* %BLKFIELD_, align 8
  %ptr_cast = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast100 = bitcast { float }* %ARGBLOCK_0 to i8*
  %func_result102 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %ptr_cast, i32 -1, i64 1239157112576, i8* nonnull %.fca.0.gep, i8* nonnull %ptr_cast100)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr

attributes #0 = { "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}

