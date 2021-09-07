; XFAIL: *
;
; RUN: opt -hir-ssa-deconstruction -hir-create-function-level-region -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
; [Note]
; - This LIT testcase is marked XFAIL due to JIRA-19686.
;   Once this JIRA closes, will need to revisit this LIT case.
;
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" -hir-create-function-level-region -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination 2>&1 < %s | FileCheck %s
;
; FORTRAN Source Code:
;subroutine sub(A, B, N)
;  real*8 B(N,N)
;  real*8 C(N,N)
;
;  C = 2.0    !  C   not needed
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
; CHECK:           if (%"sub_$N_fetch" >= 1)
; CHECK:           {
; CHECK:              + DO i1 = 0, sext.i32.i64(%"sub_$N_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%"sub_$N_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   |   (%"sub_$C")[i1][i2] = 2.000000e+00;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
;
; CHECK:              + DO i1 = 0, sext.i32.i64(%"sub_$N_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%"sub_$N_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   |   %"sub_$B[][]_fetch" = (%"sub_$B")[i1][i2];
; CHECK:              |   |   %add64 = %"sub_$B[][]_fetch"  +  1.000000e+00;
; CHECK:              |   |   (%"sub_$C")[i1][i2] = %add64;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:           %"sub_$S.0" = 0.000000e+00;
;
; CHECK:           + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   %"sub_$S.1" = %"sub_$S.0";
; CHECK:           |
; CHECK:           |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   |   %float_cast107 = fpext.float.double(%"sub_$S.1");
; CHECK:           |   |   %"sub_$C[]102[]_fetch" = (%"sub_$C")[i1][i2];
; CHECK:           |   |   %add106 = %"sub_$C[]102[]_fetch"  +  %float_cast107;
; CHECK:           |   |   %float_cast = fptrunc.double.float(%add106);
; CHECK:           |   |   %"sub_$S.1" = %float_cast;
; CHECK:           |   + END LOOP
; CHECK:           |
; CHECK:           |   %"sub_$S.0" = %float_cast;
; CHECK:           + END LOOP
;
; CHECK:           (%addressof)[0][0] = 26;
; CHECK:           (%addressof)[0][1] = 1;
; CHECK:           (%addressof)[0][2] = 1;
; CHECK:           (%addressof)[0][3] = 0;
; CHECK:           (%ARGBLOCK_0)[0].0 = %float_cast;
; CHECK:           %func_result133 = @for_write_seq_lis(&((i8*)(%"var$1")[0]),  -1,  1239157112576,  &((%addressof)[0][0]),  &((i8*)(%ARGBLOCK_0)[0]));
; CHECK:           ret ;
; CHECK:     END REGION

;*** IR Dump After HIR Dead Store Elimination ***

; CHECK:     BEGIN REGION { modified }
; CHECK:           if (%"sub_$N_fetch" >= 1)
; CHECK:           {
; CHECK:              + DO i1 = 0, sext.i32.i64(%"sub_$N_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%"sub_$N_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   |   %"sub_$B[][]_fetch" = (%"sub_$B")[i1][i2];
; CHECK:              |   |   %add64 = %"sub_$B[][]_fetch"  +  1.000000e+00;
; CHECK:              |   |   (%"sub_$C")[i1][i2] = %add64;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:           }
; CHECK:           %"sub_$S.0" = 0.000000e+00;
;
; CHECK:           + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   %"sub_$S.1" = %"sub_$S.0";
; CHECK:           |
; CHECK:           |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   |   %float_cast107 = fpext.float.double(%"sub_$S.1");
; CHECK:           |   |   %"sub_$C[]102[]_fetch" = (%"sub_$C")[i1][i2];
; CHECK:           |   |   %add106 = %"sub_$C[]102[]_fetch"  +  %float_cast107;
; CHECK:           |   |   %float_cast = fptrunc.double.float(%add106);
; CHECK:           |   |   %"sub_$S.1" = %float_cast;
; CHECK:           |   + END LOOP
; CHECK:           |
; CHECK:           |   %"sub_$S.0" = %float_cast;
; CHECK:           + END LOOP
;
; CHECK:           (%addressof)[0][0] = 26;
; CHECK:           (%addressof)[0][1] = 1;
; CHECK:           (%addressof)[0][2] = 1;
; CHECK:           (%addressof)[0][3] = 0;
; CHECK:           (%ARGBLOCK_0)[0].0 = %float_cast;
; CHECK:           %func_result133 = @for_write_seq_lis(&((i8*)(%"var$1")[0]),  -1,  1239157112576,  &((%addressof)[0][0]),  &((i8*)(%ARGBLOCK_0)[0]));
; CHECK:           ret ;
; CHECK:     END REGION

;Module Before HIR
; ModuleID = 'dead-local-store.f90'
source_filename = "dead-local-store.f90"
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
  %rel32169 = icmp slt i32 %"sub_$N_fetch", 1
  br i1 %rel32169, label %bb50.preheader, label %bb18.preheader.preheader

bb18.preheader.preheader:                         ; preds = %alloca
  %0 = add nsw i64 %int_sext, 1
  br label %bb18.preheader

bb17:                                             ; preds = %bb18.preheader, %bb17
  %"var$3.0168" = phi i64 [ 1, %bb18.preheader ], [ %add28, %bb17 ]
  %"sub_$C[][]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub_$C[]", i64 %"var$3.0168")
  store double 2.000000e+00, double* %"sub_$C[][]", align 8
  %add28 = add nuw nsw i64 %"var$3.0168", 1
  %exitcond177 = icmp eq i64 %add28, %0
  br i1 %exitcond177, label %bb20, label %bb17

bb20:                                             ; preds = %bb17
  %add36 = add nuw nsw i64 %"var$4.0170", 1
  %exitcond178 = icmp eq i64 %add36, %0
  br i1 %exitcond178, label %bb46.preheader, label %bb18.preheader

bb18.preheader:                                   ; preds = %bb18.preheader.preheader, %bb20
  %"var$4.0170" = phi i64 [ %add36, %bb20 ], [ 1, %bb18.preheader.preheader ]
  %"sub_$C[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul13, double* elementtype(double) nonnull %"sub_$C", i64 %"var$4.0170")
  br label %bb17

bb46.preheader:                                   ; preds = %bb20
  br i1 %rel32169, label %bb50.preheader, label %bb42.preheader.preheader

bb42.preheader.preheader:                         ; preds = %bb46.preheader
  %1 = add nsw i64 %int_sext, 1
  br label %bb42.preheader

bb37:                                             ; preds = %bb42.preheader, %bb37
  %"var$7.0163" = phi i64 [ 1, %bb42.preheader ], [ %add73, %bb37 ]
  %"sub_$B[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"sub_$B[]", i64 %"var$7.0163")
  %"sub_$B[][]_fetch" = load double, double* %"sub_$B[][]", align 8
  %add64 = fadd double %"sub_$B[][]_fetch", 1.000000e+00
  %"sub_$C[]47[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub_$C[]47", i64 %"var$7.0163")
  store double %add64, double* %"sub_$C[]47[]", align 8
  %add73 = add nuw nsw i64 %"var$7.0163", 1
  %exitcond175 = icmp eq i64 %add73, %1
  br i1 %exitcond175, label %bb44, label %bb37

bb44:                                             ; preds = %bb37
  %add81 = add nuw nsw i64 %"var$8.0165", 1
  %exitcond176 = icmp eq i64 %add81, %1
  br i1 %exitcond176, label %bb50.preheader.loopexit, label %bb42.preheader

bb42.preheader:                                   ; preds = %bb42.preheader.preheader, %bb44
  %"var$8.0165" = phi i64 [ %add81, %bb44 ], [ 1, %bb42.preheader.preheader ]
  %"sub_$B[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul13, double* elementtype(double) %"sub_$B", i64 %"var$8.0165")
  %"sub_$C[]47" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul13, double* elementtype(double) nonnull %"sub_$C", i64 %"var$8.0165")
  br label %bb37

bb50.preheader.loopexit:                          ; preds = %bb44
  br label %bb50.preheader

bb50.preheader:                                   ; preds = %bb50.preheader.loopexit, %alloca, %bb46.preheader
  br label %bb50

bb50:                                             ; preds = %bb55, %bb50.preheader
  %indvars.iv172 = phi i64 [ 1, %bb50.preheader ], [ %indvars.iv.next173, %bb55 ]
  %"sub_$S.0" = phi float [ 0.000000e+00, %bb50.preheader ], [ %float_cast.lcssa, %bb55 ]
  %"sub_$C[]102" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 %mul13, double* elementtype(double) nonnull %"sub_$C", i64 %indvars.iv172)
  br label %bb54

bb54:                                             ; preds = %bb54, %bb50
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb54 ], [ 1, %bb50 ]
  %"sub_$S.1" = phi float [ %float_cast, %bb54 ], [ %"sub_$S.0", %bb50 ]
  %float_cast107 = fpext float %"sub_$S.1" to double
  %"sub_$C[]102[]" = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) nonnull %"sub_$C[]102", i64 %indvars.iv)
  %"sub_$C[]102[]_fetch" = load double, double* %"sub_$C[]102[]", align 8
  %add106 = fadd double %"sub_$C[]102[]_fetch", %float_cast107
  %float_cast = fptrunc double %add106 to float
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond, label %bb55, label %bb54

bb55:                                             ; preds = %bb54
  %float_cast.lcssa = phi float [ %float_cast, %bb54 ]
  %indvars.iv.next173 = add nuw nsw i64 %indvars.iv172, 1
  %exitcond174 = icmp eq i64 %indvars.iv.next173, 6
  br i1 %exitcond174, label %bb64, label %bb50

bb64:                                             ; preds = %bb55
  %float_cast.lcssa.lcssa = phi float [ %float_cast.lcssa, %bb55 ]
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
  %ptr_cast131 = bitcast { float }* %ARGBLOCK_0 to i8*
  %func_result133 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %ptr_cast, i32 -1, i64 1239157112576, i8* nonnull %.fca.0.gep, i8* nonnull %ptr_cast131)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr

attributes #0 = { "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
