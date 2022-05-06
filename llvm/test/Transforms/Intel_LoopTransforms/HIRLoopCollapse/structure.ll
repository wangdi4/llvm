
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-collapse -hir-details-dims  -print-after=hir-loop-collapse  -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -hir-details-dims -disable-output < %s 2>&1 | FileCheck %s
;
;
; Test arrays inside a structure for collapsing 
; Notice the stride on leftmost dim is not a multiple of the next 
;  type S1
;   real*8 A(10,20,30)
;   real*4 B 
;  end type S1
;  type (S1)  Structure
;   Structure%A = 1.0 
;  (%"sub_$STRUCTURE")[0:0:48008(%"SUB$.btS1"*:0)].0
;                     [0:i1:1600([30 x [20 x [10 x double]]]:30)]
;                     [0:i2:80([20 x [10 x double]]:20)]
;                     [0:i3:8([10 x double]:10)] 
;
;*** IR Dump before HIR Loop Collapse (hir-loop-collapse) ***
;
;          BEGIN REGION { }
;              + DO i1 = 0, 29, 1   <DO_LOOP>
;              |   + DO i2 = 0, 19, 1   <DO_LOOP>
;              |   |   + DO i3 = 0, 9, 1   <DO_LOOP>
;              |   |   |   (%"sub_$STRUCTURE")[0:0:48008(%"SUB$.btS1"*:0)].0[0:i1:1600([30 x [20 x [10 x double]]]:30)][0:i2:80([20 x [10 x double]]:20)][0:i3:8([10 x double]:10)] = 1.000000e+00;
;              |   |   + END LOOP
;              |   + END LOOP
;              + END LOOP
;          END REGION
;
;*** IR Dump After HIR Loop Collapse (hir-loop-collapse) ***
;
; CHECK:        BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 5999, 1   <DO_LOOP>
; CHECK:           |   (%"sub_$STRUCTURE")[0:0:48008(%"SUB$.btS1"*:0)].0[0:0:1600([30 x [20 x [10 x double]]]:30)][0:0:80([20 x [10 x double]]:20)][0:i1:8([10 x double]:10)] = 1.000000e+00;
; CHECK:           + END LOOP
; CHECK:        END REGION
;
;Module Before HIR
; ModuleID = 'structure.f90'
source_filename = "structure.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"SUB$.btS1" = type <{ [30 x [20 x [10 x double]]], float, [4 x i8] }>

; Function Attrs: argmemonly nofree nosync nounwind writeonly uwtable
define void @sub_(%"SUB$.btS1"* noalias nocapture writeonly dereferenceable(48008) %"sub_$STRUCTURE") local_unnamed_addr #0 {
alloca_0:
  %"sub_$STRUCTURE.A$" = getelementptr inbounds %"SUB$.btS1", %"SUB$.btS1"* %"sub_$STRUCTURE", i64 0, i32 0, i64 0, i64 0, i64 0
  br label %loop_test7.preheader

loop_body4:                                       ; preds = %loop_test3.preheader, %loop_body4
  %"$loop_ctr.09" = phi i64 [ 1, %loop_test3.preheader ], [ %add.1, %loop_body4 ]
  %"sub_$STRUCTURE.A$[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %"sub_$STRUCTURE.A$[][]", i64 %"$loop_ctr.09")
  store double 1.000000e+00, double* %"sub_$STRUCTURE.A$[][][]", align 1, !tbaa !0
  %add.1 = add nuw nsw i64 %"$loop_ctr.09", 1
  %exitcond.not = icmp eq i64 %add.1, 11
  br i1 %exitcond.not, label %loop_exit5, label %loop_body4

loop_exit5:                                       ; preds = %loop_body4
  %add.2 = add nuw nsw i64 %"$loop_ctr1.010", 1
  %exitcond12.not = icmp eq i64 %add.2, 21
  br i1 %exitcond12.not, label %loop_exit9, label %loop_test3.preheader

loop_test3.preheader:                             ; preds = %loop_test7.preheader, %loop_exit5
  %"$loop_ctr1.010" = phi i64 [ 1, %loop_test7.preheader ], [ %add.2, %loop_exit5 ]
  %"sub_$STRUCTURE.A$[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 80, double* nonnull elementtype(double) %"sub_$STRUCTURE.A$[]", i64 %"$loop_ctr1.010")
  br label %loop_body4

loop_exit9:                                       ; preds = %loop_exit5
  %add.3 = add nuw nsw i64 %"$loop_ctr2.011", 1
  %exitcond13.not = icmp eq i64 %add.3, 31
  br i1 %exitcond13.not, label %loop_exit13, label %loop_test7.preheader

loop_test7.preheader:                             ; preds = %alloca_0, %loop_exit9
  %"$loop_ctr2.011" = phi i64 [ 1, %alloca_0 ], [ %add.3, %loop_exit9 ]
  %"sub_$STRUCTURE.A$[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 1600, double* nonnull elementtype(double) %"sub_$STRUCTURE.A$", i64 %"$loop_ctr2.011")
  br label %loop_test3.preheader

loop_exit13:                                      ; preds = %loop_exit9
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

attributes #0 = { argmemonly nofree nosync nounwind writeonly uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="nocona" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sse,+sse2,+sse3,+x87" }
attributes #1 = { nofree nosync nounwind readnone speculatable }

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Generic Fortran Symbol", !3, i64 0}
!3 = !{!"ifx$root$1$sub_"}
