
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-collapse -hir-details-dims -print-before=hir-loop-collapse -print-after=hir-loop-collapse  -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -hir-details-dims -disable-output < %s 2>&1 | FileCheck %s
;

; F90 pointer 
;subroutine aer (A) 
; real*8, target  :: A(5,5)  
; real*8, pointer  ::  P(:,:)
; P => A(1:5:2, 1:4) 
; P = 1 
;end subroutine aer

;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK:           |   |   (%"aer_$A")[0:i1:40(double*:0)][0:i2:16(double*:3)] = 1.000000e+00;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:           |   + DO i2 = 0, 2, 1   <DO_LOOP>
; CHECK:           |   |   (%"aer_$A")[0:i1:40(double*:0)][0:i2:16(double*:3)] = 1.000000e+00;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION

;Module Before HIR
; ModuleID = 'F90Pointer.f90'
source_filename = "F90Pointer.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"QNCA_a0$double*$rank2$" = type { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }

; Function Attrs: argmemonly nofree nosync nounwind writeonly uwtable
define void @aer_(double* dereferenceable(8) %"aer_$A") local_unnamed_addr #0 {
alloca_0:
  %"aer_$P" = alloca %"QNCA_a0$double*$rank2$", align 8, !llfort.type_idx !0
  %"aer_$P.addr_a0$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"aer_$P", i64 0, i32 0
  %"aer_$P.flags$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"aer_$P", i64 0, i32 3
  store i64 3, i64* %"aer_$P.flags$", align 8, !tbaa !1
  %"aer_$P.addr_length$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"aer_$P", i64 0, i32 1
  store i64 8, i64* %"aer_$P.addr_length$", align 8, !tbaa !6
  %"aer_$P.dim$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"aer_$P", i64 0, i32 4
  store i64 2, i64* %"aer_$P.dim$", align 8, !tbaa !7
  %"aer_$P.codim$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"aer_$P", i64 0, i32 2
  store i64 0, i64* %"aer_$P.codim$", align 8, !tbaa !8
  %"aer_$P.dim_info$.spacing$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"aer_$P", i64 0, i32 6, i64 0, i32 1
  %"aer_$P.dim_info$.spacing$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.spacing$", i32 0)
  store i64 16, i64* %"aer_$P.dim_info$.spacing$[]", align 1, !tbaa !9
  %"aer_$P.dim_info$.lower_bound$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"aer_$P", i64 0, i32 6, i64 0, i32 2
  %"aer_$P.dim_info$.lower_bound$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.lower_bound$", i32 0)
  store i64 1, i64* %"aer_$P.dim_info$.lower_bound$[]", align 1, !tbaa !10
  %"aer_$P.dim_info$.extent$" = getelementptr inbounds %"QNCA_a0$double*$rank2$", %"QNCA_a0$double*$rank2$"* %"aer_$P", i64 0, i32 6, i64 0, i32 0
  %"aer_$P.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.extent$", i32 0)
  store i64 3, i64* %"aer_$P.dim_info$.extent$[]", align 1, !tbaa !11
  %"aer_$P.dim_info$.spacing$[]6" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.spacing$", i32 1)
  store i64 40, i64* %"aer_$P.dim_info$.spacing$[]6", align 1, !tbaa !9
  %"aer_$P.dim_info$.lower_bound$[]9" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.lower_bound$", i32 1)
  store i64 1, i64* %"aer_$P.dim_info$.lower_bound$[]9", align 1, !tbaa !10
  %"aer_$P.dim_info$.extent$[]12" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"aer_$P.dim_info$.extent$", i32 1)
  store i64 4, i64* %"aer_$P.dim_info$.extent$[]12", align 1, !tbaa !11
  store double* %"aer_$A", double** %"aer_$P.addr_a0$", align 8, !tbaa !12
  br label %loop_test5.preheader

loop_body6:                                       ; preds = %loop_test5.preheader, %loop_body6
  %"var$5.070" = phi i64 [ 1, %loop_test5.preheader ], [ %add.6, %loop_body6 ]
  %"aer_$P.addr_a0$_fetch.1[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 16, double* nonnull elementtype(double) %"aer_$P.addr_a0$_fetch.1[]", i64 %"var$5.070")
  store double 1.000000e+00, double* %"aer_$P.addr_a0$_fetch.1[][]", align 1, !tbaa !13
  %add.6 = add nuw nsw i64 %"var$5.070", 1
  %exitcond = icmp eq i64 %add.6, 4
  br i1 %exitcond, label %loop_exit7, label %loop_body6

loop_exit7:                                       ; preds = %loop_body6
  %add.8 = add nuw nsw i64 %"var$6.072", 1
  %exitcond73 = icmp eq i64 %add.8, 5
  br i1 %exitcond73, label %loop_exit11, label %loop_test5.preheader

loop_test5.preheader:                             ; preds = %alloca_0, %loop_exit7
  %"var$6.072" = phi i64 [ 1, %alloca_0 ], [ %add.8, %loop_exit7 ]
  %"aer_$P.addr_a0$_fetch.1[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* nonnull elementtype(double) %"aer_$A", i64 %"var$6.072")
  br label %loop_body6

loop_exit11:                                      ; preds = %loop_exit7
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nofree nosync nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { argmemonly nofree nosync nounwind writeonly uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{i64 28}
!1 = !{!2, !3, i64 24}
!2 = !{!"ifx$descr$1", !3, i64 0, !3, i64 8, !3, i64 16, !3, i64 24, !3, i64 32, !3, i64 40, !3, i64 48, !3, i64 56, !3, i64 64, !3, i64 72, !3, i64 80, !3, i64 88}
!3 = !{!"ifx$descr$field", !4, i64 0}
!4 = !{!"Generic Fortran Symbol", !5, i64 0}
!5 = !{!"ifx$root$1$aer_"}
!6 = !{!2, !3, i64 8}
!7 = !{!2, !3, i64 32}
!8 = !{!2, !3, i64 16}
!9 = !{!2, !3, i64 56}
!10 = !{!2, !3, i64 64}
!11 = !{!2, !3, i64 48}
!12 = !{!2, !3, i64 0}
!13 = !{!4, !4, i64 0}
