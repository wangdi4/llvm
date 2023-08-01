; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; *** Source Code ***
;subroutine compute_rhs (N1,N2,N3)
;  common /x/rhs, forcing
;  real*8 rhs(5,0: 12, 0:12, 0:12)
;  real*8 forcing(5,0: 12, 0:12, 0:12)

;  do k = 0, N1
;    do j = 0, N2
;      do i = 0,N3
;        do m = 1, 5
;           rhs(m,i,j,k) = forcing(m,i,j,k) + rhs(m,i,j,k) +1
;        enddo
;      enddo
;    enddo
;  enddo
; end
;
; [Note]
; - The UB is in form of %Blob, not %Blob -1.
;   The collapser needs to understand this form of UB and handle the UBTC properly.
;


;*** IR Dump Before HIR Loop Collapse (hir-loop-collapse) ***
;Function: compute_rhs_

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, zext.i32.i64(%"compute_rhs_$N1_fetch.1"), 1   <DO_LOOP>  <MAX_TC_EST = 175760>  <LEGAL_MAX_TC = 2147483648>
; CHECK:               |   + DO i2 = 0, sext.i32.i64(%"compute_rhs_$N2_fetch.3"), 1   <DO_LOOP>  <MAX_TC_EST = 13>  <LEGAL_MAX_TC = 2147483648>
; CHECK:               |   |   + DO i3 = 0, sext.i32.i64(%"compute_rhs_$N3_fetch.5"), 1   <DO_LOOP>  <MAX_TC_EST = 13>  <LEGAL_MAX_TC = 2147483648>
; CHECK:               |   |   |   + DO i4 = 0, 4, 1   <DO_LOOP>
; CHECK:               |   |   |   |   %add.1 = (getelementptr inbounds ([175760 x i8], ptr @x_, i64 0, i64 87880))[i1][i2][i3][i4]  +  (@x_)[i1][i2][i3][i4];
; CHECK:               |   |   |   |   %add.2 = %add.1  +  1.000000e+00;
; CHECK:               |   |   |   |   (@x_)[i1][i2][i3][i4] = %add.2;
; CHECK:               |   |   |   + END LOOP
; CHECK:               |   |   + END LOOP
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION

;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
; CHECK: Function: compute_rhs_

; CHECK:         BEGIN REGION { modified }
; CHECK:               + DO i1 = 0, zext.i32.i64(%"compute_rhs_$N1_fetch.1"), 1   <DO_LOOP>  <MAX_TC_EST = 175760>  <LEGAL_MAX_TC = 2147483648>
; CHECK:               |   + DO i2 = 0, sext.i32.i64(%"compute_rhs_$N2_fetch.3"), 1   <DO_LOOP>  <MAX_TC_EST = 13>  <LEGAL_MAX_TC = 2147483648>
; CHECK:               |   |   + DO i3 = 0, 5 * (1 + sext.i32.i64(%"compute_rhs_$N3_fetch.5")) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 65>
; CHECK:               |   |   |   %add.1 = (getelementptr inbounds ([175760 x i8], ptr @x_, i64 0, i64 87880))[i1][i2][0][i3]  +  (@x_)[i1][i2][0][i3];
; CHECK:               |   |   |   %add.2 = %add.1  +  1.000000e+00;
; CHECK:               |   |   |   (@x_)[i1][i2][0][i3] = %add.2;
; CHECK:               |   |   + END LOOP
; CHECK:               |   + END LOOP
; CHECK:               + END LOOP
; CHECK:         END REGION


;Module Before HIR
; ModuleID = 'small.f'
source_filename = "small.f"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x_ = common unnamed_addr global [175760 x i8] zeroinitializer, align 32

; Function Attrs: nofree nosync nounwind uwtable
define void @compute_rhs_(ptr noalias nocapture readonly dereferenceable(4) %"compute_rhs_$N1", ptr noalias nocapture readonly dereferenceable(4) %"compute_rhs_$N2", ptr noalias nocapture readonly dereferenceable(4) %"compute_rhs_$N3") local_unnamed_addr #0 {
alloca_0:
  %"compute_rhs_$N1_fetch.1" = load i32, ptr %"compute_rhs_$N1", align 1
  %rel.1 = icmp slt i32 %"compute_rhs_$N1_fetch.1", 0
  br i1 %rel.1, label %bb3, label %bb2.preheader

bb2.preheader:                                    ; preds = %alloca_0
  %"compute_rhs_$N2_fetch.3" = load i32, ptr %"compute_rhs_$N2", align 1
  %rel.2 = icmp slt i32 %"compute_rhs_$N2_fetch.3", 0
  %"compute_rhs_$N3_fetch.5" = load i32, ptr %"compute_rhs_$N3", align 1
  %rel.3 = icmp slt i32 %"compute_rhs_$N3_fetch.5", 0
  %0 = add nuw nsw i32 %"compute_rhs_$N3_fetch.5", 1
  %1 = add nuw nsw i32 %"compute_rhs_$N2_fetch.3", 1
  %2 = add nuw nsw i32 %"compute_rhs_$N1_fetch.1", 1
  %wide.trip.count4850 = zext i32 %2 to i64
  %wide.trip.count44 = sext i32 %1 to i64
  %wide.trip.count = sext i32 %0 to i64
  br label %bb2

bb2:                                              ; preds = %bb2.preheader, %bb7
  %indvars.iv46 = phi i64 [ 0, %bb2.preheader ], [ %indvars.iv.next47, %bb7 ]
  br i1 %rel.2, label %bb7, label %bb6.preheader

bb6.preheader:                                    ; preds = %bb2
  %"val$[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 0, i64 6760, ptr elementtype(double) getelementptr inbounds ([175760 x i8], ptr @x_, i64 0, i64 87880), i64 %indvars.iv46)
  %"val$[]8" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 0, i64 6760, ptr elementtype(double) @x_, i64 %indvars.iv46)
  br label %bb6

bb6:                                              ; preds = %bb6.preheader, %bb11
  %indvars.iv42 = phi i64 [ 0, %bb6.preheader ], [ %indvars.iv.next43, %bb11 ]
  br i1 %rel.3, label %bb11, label %bb10.preheader

bb10.preheader:                                   ; preds = %bb6
  %"val$[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 0, i64 520, ptr elementtype(double) %"val$[]", i64 %indvars.iv42)
  %"val$[][]9" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 0, i64 520, ptr elementtype(double) %"val$[]8", i64 %indvars.iv42)
  br label %bb10

bb10:                                             ; preds = %bb10.preheader, %bb17
  %indvars.iv39 = phi i64 [ 0, %bb10.preheader ], [ %indvars.iv.next40, %bb17 ]
  %"val$[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 40, ptr elementtype(double) %"val$[][]", i64 %indvars.iv39)
  %"val$[][][]10" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 40, ptr elementtype(double) %"val$[][]9", i64 %indvars.iv39)
  br label %bb14

bb14:                                             ; preds = %bb14, %bb10
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb14 ], [ 1, %bb10 ]
  %"val$[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"val$[][][]", i64 %indvars.iv)
  %"val$[][][][]_fetch.11" = load double, ptr %"val$[][][][]", align 1
  %"val$[][][][]11" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"val$[][][]10", i64 %indvars.iv)
  %"val$[][][][]_fetch.16" = load double, ptr %"val$[][][][]11", align 1
  %add.1 = fadd reassoc ninf nsz arcp contract afn double %"val$[][][][]_fetch.11", %"val$[][][][]_fetch.16"
  %add.2 = fadd reassoc ninf nsz arcp contract afn double %add.1, 1.000000e+00
  store double %add.2, ptr %"val$[][][][]11", align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 6
  br i1 %exitcond.not, label %bb17, label %bb14

bb17:                                             ; preds = %bb14
  %indvars.iv.next40 = add nuw nsw i64 %indvars.iv39, 1
  %exitcond41 = icmp eq i64 %indvars.iv.next40, %wide.trip.count
  br i1 %exitcond41, label %bb11.loopexit, label %bb10

bb11.loopexit:                                    ; preds = %bb17
  br label %bb11

bb11:                                             ; preds = %bb11.loopexit, %bb6
  %indvars.iv.next43 = add nuw nsw i64 %indvars.iv42, 1
  %exitcond45 = icmp eq i64 %indvars.iv.next43, %wide.trip.count44
  br i1 %exitcond45, label %bb7.loopexit, label %bb6

bb7.loopexit:                                     ; preds = %bb11
  br label %bb7

bb7:                                              ; preds = %bb7.loopexit, %bb2
  %indvars.iv.next47 = add nuw nsw i64 %indvars.iv46, 1
  %exitcond49 = icmp eq i64 %indvars.iv.next47, %wide.trip.count4850
  br i1 %exitcond49, label %bb3.loopexit, label %bb2

bb3.loopexit:                                     ; preds = %bb7
  br label %bb3

bb3:                                              ; preds = %bb3.loopexit, %alloca_0
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nosync nounwind uwtable "frame-pointer"="none" "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

