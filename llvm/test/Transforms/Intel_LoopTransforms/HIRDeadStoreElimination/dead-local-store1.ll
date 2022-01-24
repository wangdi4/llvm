;
; RUN: opt -hir-ssa-deconstruction -hir-create-function-level-region -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
;
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" -hir-create-function-level-region -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination 2>&1 < %s | FileCheck %s
;
; FORTRAN Source Code:
;subroutine sub(A, B)
;  real*8 B(100,100)
;  real*8 C(100,100)
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

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   (@"sub_$C")[0][i1][i2] = 2.000000e+00;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
;
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   %"sub_$B_entry[][]_fetch" = (%"sub_$B")[i1][i2];
; CHECK:           |   |   %add17 = %"sub_$B_entry[][]_fetch"  +  1.000000e+00;
; CHECK:           |   |   (@"sub_$C")[0][i1][i2] = %add17;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
;
; CHECK:           %"sub_$S.0" = 0.000000e+00;
;
; CHECK:           + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   |   %float_cast38 = fpext.float.double(%"sub_$S.0");
; CHECK:           |   |   %"sub_$C[]35[]_fetch" = (@"sub_$C")[0][i1][i2];
; CHECK:           |   |   %add37 = %"sub_$C[]35[]_fetch"  +  %float_cast38;
; CHECK:           |   |   %"sub_$S.0" = fptrunc.double.float(%add37);
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
; CHECK:           %func_result = @for_write_seq_lis(&((i8*)(%"var$1")[0]),  -1,  1239157112576,  &((%addressof)[0][0]),  &((i8*)(%ARGBLOCK_0)[0]));
; CHECK:           ret ;
; CHECK:     END REGION

;*** IR Dump After HIR Dead Store Elimination ***

; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   |   %"sub_$B_entry[][]_fetch" = (%"sub_$B")[i1][i2];
; CHECK:           |   |   %add17 = %"sub_$B_entry[][]_fetch"  +  1.000000e+00;
; CHECK:           |   |   (@"sub_$C")[0][i1][i2] = %add17;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
;
; CHECK:           %"sub_$S.0" = 0.000000e+00;
;
; CHECK:           + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   |   %float_cast38 = fpext.float.double(%"sub_$S.0");
; CHECK:           |   |   %"sub_$C[]35[]_fetch" = (@"sub_$C")[0][i1][i2];
; CHECK:           |   |   %add37 = %"sub_$C[]35[]_fetch"  +  %float_cast38;
; CHECK:           |   |   %"sub_$S.0" = fptrunc.double.float(%add37);
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
; CHECK:           %func_result = @for_write_seq_lis(&((i8*)(%"var$1")[0]),  -1,  1239157112576,  &((%addressof)[0][0]),  &((i8*)(%ARGBLOCK_0)[0]));
; CHECK:           ret ;
; CHECK:     END REGION

;Module Before HIR
; ModuleID = 'dead-local-store1.f90'
source_filename = "dead-local-store1.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"sub_$C" = internal unnamed_addr global [100 x [100 x double]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub_(float* noalias nocapture readnone "ptrnoalias" %"sub_$A", double* noalias nocapture readonly "ptrnoalias" %"sub_$B") local_unnamed_addr #0 {
alloca_0:
  %"var$1" = alloca [8 x i64], align 16
  %addressof = alloca [4 x i8], align 1
  %ARGBLOCK_0 = alloca { float }, align 8
  br label %bb14.preheader

bb13:                                             ; preds = %bb13, %bb14.preheader
  %"var$2.077" = phi i64 [ 1, %bb14.preheader ], [ %add, %bb13 ]
  %"sub_$C[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"sub_$C[]", i64 %"var$2.077")
  store double 2.000000e+00, double* %"sub_$C[][]", align 1
  %add = add nuw nsw i64 %"var$2.077", 1
  %exitcond84 = icmp eq i64 %add, 101
  br i1 %exitcond84, label %bb16, label %bb13

bb16:                                             ; preds = %bb13
  %add12 = add nuw nsw i64 %"var$3.078", 1
  %exitcond85 = icmp eq i64 %add12, 101
  br i1 %exitcond85, label %bb36.preheader.preheader, label %bb14.preheader

bb36.preheader.preheader:                         ; preds = %bb16
  br label %bb36.preheader

bb14.preheader:                                   ; preds = %bb16, %alloca_0
  %"var$3.078" = phi i64 [ 1, %alloca_0 ], [ %add12, %bb16 ]
  %"sub_$C[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 800, double* elementtype(double) getelementptr inbounds ([100 x [100 x double]], [100 x [100 x double]]* @"sub_$C", i64 0, i64 0, i64 0), i64 %"var$3.078")
  br label %bb13

bb31:                                             ; preds = %bb31, %bb36.preheader
  %"var$4.075" = phi i64 [ 1, %bb36.preheader ], [ %add26, %bb31 ]
  %"sub_$B_entry[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"sub_$B_entry[]", i64 %"var$4.075")
  %"sub_$B_entry[][]_fetch" = load double, double* %"sub_$B_entry[][]", align 1
  %add17 = fadd double %"sub_$B_entry[][]_fetch", 1.000000e+00
  %"sub_$C[]13[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"sub_$C[]13", i64 %"var$4.075")
  store double %add17, double* %"sub_$C[]13[]", align 1
  %add26 = add nuw nsw i64 %"var$4.075", 1
  %exitcond82 = icmp eq i64 %add26, 101
  br i1 %exitcond82, label %bb38, label %bb31

bb38:                                             ; preds = %bb31
  %add34 = add nuw nsw i64 %"var$5.076", 1
  %exitcond83 = icmp eq i64 %add34, 101
  br i1 %exitcond83, label %bb44.preheader, label %bb36.preheader

bb44.preheader:                                   ; preds = %bb38
  br label %bb44

bb36.preheader:                                   ; preds = %bb36.preheader.preheader, %bb38
  %"var$5.076" = phi i64 [ %add34, %bb38 ], [ 1, %bb36.preheader.preheader ]
  %"sub_$B_entry[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 800, double* elementtype(double) %"sub_$B", i64 %"var$5.076")
  %"sub_$C[]13" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 800, double* elementtype(double) getelementptr inbounds ([100 x [100 x double]], [100 x [100 x double]]* @"sub_$C", i64 0, i64 0, i64 0), i64 %"var$5.076")
  br label %bb31

bb44:                                             ; preds = %bb44.preheader, %bb49
  %indvars.iv79 = phi i64 [ %indvars.iv.next80, %bb49 ], [ 1, %bb44.preheader ]
  %"sub_$S.0" = phi float [ %float_cast.lcssa, %bb49 ], [ 0.000000e+00, %bb44.preheader ]
  %"sub_$C[]35" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 800, double* elementtype(double) getelementptr inbounds ([100 x [100 x double]], [100 x [100 x double]]* @"sub_$C", i64 0, i64 0, i64 0), i64 %indvars.iv79)
  br label %bb48

bb48:                                             ; preds = %bb48, %bb44
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb48 ], [ 1, %bb44 ]
  %"sub_$S.1" = phi float [ %float_cast, %bb48 ], [ %"sub_$S.0", %bb44 ]
  %float_cast38 = fpext float %"sub_$S.1" to double
  %"sub_$C[]35[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"sub_$C[]35", i64 %indvars.iv)
  %"sub_$C[]35[]_fetch" = load double, double* %"sub_$C[]35[]", align 1
  %add37 = fadd double %"sub_$C[]35[]_fetch", %float_cast38
  %float_cast = fptrunc double %add37 to float
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond, label %bb49, label %bb48

bb49:                                             ; preds = %bb48
  %float_cast.lcssa = phi float [ %float_cast, %bb48 ]
  %indvars.iv.next80 = add nuw nsw i64 %indvars.iv79, 1
  %exitcond81 = icmp eq i64 %indvars.iv.next80, 6
  br i1 %exitcond81, label %bb58, label %bb44

bb58:                                             ; preds = %bb49
  %float_cast.lcssa.lcssa = phi float [ %float_cast.lcssa, %bb49 ]
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
  %ptr_cast57 = bitcast [8 x i64]* %"var$1" to i8*
  %ptr_cast61 = bitcast { float }* %ARGBLOCK_0 to i8*
  %func_result = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %ptr_cast57, i32 -1, i64 1239157112576, i8* nonnull %.fca.0.gep, i8* nonnull %ptr_cast61) #2
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr

attributes #0 = { nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }

!omp_offload.info = !{}
