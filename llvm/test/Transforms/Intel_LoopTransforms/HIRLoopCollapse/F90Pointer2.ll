
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-collapse -hir-details-dims  -print-after=hir-loop-collapse  -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -hir-details-dims -disable-output < %s 2>&1 | FileCheck %s
;
;  Innermost 2 levels cab collaped 
;
;  real*8, target  ::   A(5,5,5,5)  
;  real*8, pointer  ::  P(:,:,:,:)
;  P => A(:,:,1:5:N,:) 
;  P = 1 

;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***
;               BEGIN REGION { }
;                 + DO i1 = 0, 4, 1   <DO_LOOP>
;                 |   + DO i2 = 0, %0 + -1, 1   <DO_LOOP>
;                 |   |   + DO i3 = 0, 4, 1   <DO_LOOP>
;                 |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
;                 |   |   |   |   (%"aer_$A")[0:i1:1000(double*:0)][0:i2:200 * sext.i32.i64(%"aer_$N_fetch.1")(double*:0)][0:i3:40(double*:0)][0:i4:8(double*:5)] = 1.000000e+00;
;                 |   |   |   + END LOOP
;                 |   |   + END LOOP
;                 |   + END LOOP
;                 + END LOOP
;               END REGION
;

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***


; CHECK:         BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, %0 + -1, 1   <DO_LOOP>
; CHECK:           |   |   + DO i3 = 0, 24, 1   <DO_LOOP>
; CHECK:           |   |   |   (%"aer_$A")[0:i1:1000(double*:0)][0:i2:200 * sext.i32.i64(%"aer_$N_fetch.1")(double*:0)][0:0:40(double*:0)][0:i3:8(double*:5)] = 1.000000e+00;
; CHECK:           |   |   + END LOOP
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:         END REGION


;Module Before HIR
; ModuleID = 'F90Pointer2.f90'
source_filename = "F90Pointer2.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$double*$rank4$" = type { double*, i64, i64, i64, i64, i64, [4 x { i64, i64, i64 }] }

; Function Attrs: argmemonly nofree nosync nounwind uwtable
define void @aer_(double* dereferenceable(8) %"aer_$A", i32* noalias nocapture readonly dereferenceable(4) %"aer_$N") local_unnamed_addr #0 {
alloca_0:
  %"aer_$P" = alloca %"QNCA_a0$double*$rank4$", align 8, !llfort.type_idx !0
  %"aer_$P.addr_a0$" = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %"aer_$P", i64 0, i32 0
  %"aer_$P.flags$" = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %"aer_$P", i64 0, i32 3
  %"aer_$N_fetch.1" = load i32, i32* %"aer_$N", align 1, !tbaa !1
  %int_sext = sext i32 %"aer_$N_fetch.1" to i64
  %add.1 = add nsw i64 %int_sext, 4
  %div.1 = sdiv i64 %add.1, %int_sext
  %0 = tail call i64 @llvm.smax.i64(i64 %div.1, i64 0)
  store i64 3, i64* %"aer_$P.flags$", align 8, !tbaa !5
  %"aer_$P.addr_length$" = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %"aer_$P", i64 0, i32 1
  store i64 8, i64* %"aer_$P.addr_length$", align 8, !tbaa !8
  %"aer_$P.dim$" = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %"aer_$P", i64 0, i32 4
  store i64 4, i64* %"aer_$P.dim$", align 8, !tbaa !9
  %"aer_$P.codim$" = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %"aer_$P", i64 0, i32 2
  store i64 0, i64* %"aer_$P.codim$", align 8, !tbaa !10
  %"aer_$P.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %"aer_$P", i64 0, i32 6, i64 0, i32 1
  %"aer_$P.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.spacing$", i32 0)
  store i64 8, i64* %"aer_$P.dim_info$.spacing$[]", align 1, !tbaa !11
  %"aer_$P.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %"aer_$P", i64 0, i32 6, i64 0, i32 2
  %"aer_$P.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.lower_bound$", i32 0)
  store i64 1, i64* %"aer_$P.dim_info$.lower_bound$[]", align 1, !tbaa !12
  %"aer_$P.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$double*$rank4$", %"QNCA_a0$double*$rank4$"* %"aer_$P", i64 0, i32 6, i64 0, i32 0
  %"aer_$P.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.extent$", i32 0)
  store i64 5, i64* %"aer_$P.dim_info$.extent$[]", align 1, !tbaa !13
  %"aer_$P.dim_info$.spacing$[]6" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.spacing$", i32 1)
  store i64 40, i64* %"aer_$P.dim_info$.spacing$[]6", align 1, !tbaa !11
  %"aer_$P.dim_info$.lower_bound$[]9" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.lower_bound$", i32 1)
  store i64 1, i64* %"aer_$P.dim_info$.lower_bound$[]9", align 1, !tbaa !12
  %"aer_$P.dim_info$.extent$[]12" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.extent$", i32 1)
  store i64 5, i64* %"aer_$P.dim_info$.extent$[]12", align 1, !tbaa !13
  %mul.6 = mul nsw i64 %int_sext, 200
  %"aer_$P.dim_info$.spacing$[]16" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.spacing$", i32 2)
  store i64 %mul.6, i64* %"aer_$P.dim_info$.spacing$[]16", align 1, !tbaa !11
  %"aer_$P.dim_info$.lower_bound$[]19" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.lower_bound$", i32 2)
  store i64 1, i64* %"aer_$P.dim_info$.lower_bound$[]19", align 1, !tbaa !12
  %"aer_$P.dim_info$.extent$[]22" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.extent$", i32 2)
  store i64 %0, i64* %"aer_$P.dim_info$.extent$[]22", align 1, !tbaa !13
  %"aer_$P.dim_info$.spacing$[]26" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.spacing$", i32 3)
  store i64 1000, i64* %"aer_$P.dim_info$.spacing$[]26", align 1, !tbaa !11
  %"aer_$P.dim_info$.lower_bound$[]29" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.lower_bound$", i32 3)
  store i64 1, i64* %"aer_$P.dim_info$.lower_bound$[]29", align 1, !tbaa !12
  %"aer_$P.dim_info$.extent$[]32" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.extent$", i32 3)
  store i64 5, i64* %"aer_$P.dim_info$.extent$[]32", align 1, !tbaa !13
  store double* %"aer_$A", double** %"aer_$P.addr_a0$", align 8, !tbaa !14
  %rel.4.not147 = icmp slt i64 %div.1, 1
  %1 = add nuw nsw i64 %0, 1
  br label %loop_test13.preheader

loop_body6:                                       ; preds = %loop_test5.preheader, %loop_body6
  %"var$7.0144" = phi i64 [ 1, %loop_test5.preheader ], [ %add.12, %loop_body6 ]
  %"aer_$P.addr_a0$_fetch.2[][][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"aer_$P.addr_a0$_fetch.2[][][]", i64 %"var$7.0144")
  store double 1.000000e+00, double* %"aer_$P.addr_a0$_fetch.2[][][][]", align 1, !tbaa !15
  %add.12 = add nuw nsw i64 %"var$7.0144", 1
  %exitcond = icmp eq i64 %add.12, 6
  br i1 %exitcond, label %loop_exit7, label %loop_body6

loop_exit7:                                       ; preds = %loop_body6
  %add.14 = add nuw nsw i64 %"var$8.0146", 1
  %exitcond152 = icmp eq i64 %add.14, 6
  br i1 %exitcond152, label %loop_exit11, label %loop_test5.preheader

loop_test5.preheader:                             ; preds = %loop_test9.preheader, %loop_exit7
  %"var$8.0146" = phi i64 [ 1, %loop_test9.preheader ], [ %add.14, %loop_exit7 ]
  %"aer_$P.addr_a0$_fetch.2[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* nonnull elementtype(double) %"aer_$P.addr_a0$_fetch.2[][]", i64 %"var$8.0146")
  br label %loop_body6

loop_exit11:                                      ; preds = %loop_exit7
  %add.16 = add nuw nsw i64 %"var$9.0149", 1
  %exitcond153 = icmp eq i64 %add.16, %1
  br i1 %exitcond153, label %loop_exit15.loopexit, label %loop_test9.preheader

loop_test9.preheader:                             ; preds = %loop_test9.preheader.lr.ph, %loop_exit11
  %"var$9.0149" = phi i64 [ 1, %loop_test9.preheader.lr.ph ], [ %add.16, %loop_exit11 ]
  %"aer_$P.addr_a0$_fetch.2[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul.6, double* nonnull elementtype(double) %"aer_$P.addr_a0$_fetch.2[]", i64 %"var$9.0149")
  br label %loop_test5.preheader

loop_exit15.loopexit:                             ; preds = %loop_exit11
  br label %loop_exit15

loop_exit15:                                      ; preds = %loop_exit15.loopexit, %loop_test13.preheader
  %add.18 = add nuw nsw i64 %"var$10.0151", 1
  %exitcond154 = icmp eq i64 %add.18, 6
  br i1 %exitcond154, label %loop_exit19, label %loop_test13.preheader

loop_test13.preheader:                            ; preds = %alloca_0, %loop_exit15
  %"var$10.0151" = phi i64 [ 1, %alloca_0 ], [ %add.18, %loop_exit15 ]
  br i1 %rel.4.not147, label %loop_exit15, label %loop_test9.preheader.lr.ph

loop_test9.preheader.lr.ph:                       ; preds = %loop_test13.preheader
  %"aer_$P.addr_a0$_fetch.2[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 1000, double* nonnull elementtype(double) %"aer_$A", i64 %"var$10.0151")
  br label %loop_test9.preheader

loop_exit19:                                      ; preds = %loop_exit15
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare i64 @llvm.smax.i64(i64, i64) #2

attributes #0 = { argmemonly nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }
attributes #2 = { mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn }

!omp_offload.info = !{}

!0 = !{i64 28}
!1 = !{!2, !2, i64 0}
!2 = !{!"ifx$unique_sym$1", !3, i64 0}
!3 = !{!"Generic Fortran Symbol", !4, i64 0}
!4 = !{!"ifx$root$1$aer_"}
!5 = !{!6, !7, i64 24}
!6 = !{!"ifx$descr$1", !7, i64 0, !7, i64 8, !7, i64 16, !7, i64 24, !7, i64 32, !7, i64 40, !7, i64 48, !7, i64 56, !7, i64 64, !7, i64 72, !7, i64 80, !7, i64 88, !7, i64 96, !7, i64 104, !7, i64 112, !7, i64 120, !7, i64 128, !7, i64 136}
!7 = !{!"ifx$descr$field", !3, i64 0}
!8 = !{!6, !7, i64 8}
!9 = !{!6, !7, i64 32}
!10 = !{!6, !7, i64 16}
!11 = !{!6, !7, i64 56}
!12 = !{!6, !7, i64 64}
!13 = !{!6, !7, i64 48}
!14 = !{!6, !7, i64 0}
!15 = !{!3, !3, i64 0}

