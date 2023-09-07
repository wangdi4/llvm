; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output < %s 2>&1 | FileCheck %s

; Test checks that the distance of 4 (the original inner loop TC) is assigned to the DV innermost dimension for collapsed refs.

; Original HIR:
;<0>          BEGIN REGION { }
;<46>               + DO i1 = 0, 999, 1   <DO_LOOP>
;<47>               |   + DO i2 = 0, 23, 1   <DO_LOOP>
;<48>               |   |   + DO i3 = 0, 3, 1   <DO_LOOP>
;<14>               |   |   |   %fetch3 = (%MD)[i2][i3];
;<16>               |   |   |   if (%fetch3 < 1.000000e+03)
;<16>               |   |   |   {
;<22>               |   |   |      %mul.4 = (%COND)[i2 + 1][i3]  *  %fetch3;
;<24>               |   |   |      (%COND)[i2 + 2][i3] = %mul.4;
;<16>               |   |   |   }
;<48>               |   |   + END LOOP
;<47>               |   + END LOOP
;<46>               + END LOOP
;<0>          END REGION

; HIR After Collapse:
;<0>          BEGIN REGION { modified }
;<46>               + DO i1 = 0, 999, 1   <DO_LOOP>
;<48>               |   + DO i2 = 0, 95, 1   <DO_LOOP>  <MAX_TC_EST = 96>  <LEGAL_MAX_TC = 96>
;<14>               |   |   %fetch3 = (%MD)[0][i2];
;<16>               |   |   if (%fetch3 < 1.000000e+03)
;<16>               |   |   {
;<22>               |   |      %mul.4 = (%COND)[1][i2]  *  %fetch3;
;<24>               |   |      (%COND)[2][i2] = %mul.4;
;<16>               |   |   }
;<48>               |   + END LOOP
;<46>               + END LOOP
;<0>          END REGION

; CHECK-DAG: 24:22 (%COND)[2][i2] --> (%COND)[1][i2] FLOW (* <) (? 4)
; CHECK-DAG: 24:24 (%COND)[2][i2] --> (%COND)[2][i2] OUTPUT (* <) (? 4)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @cam4_(ptr noalias nocapture dereferenceable(8) %COND, ptr noalias nocapture readonly dereferenceable(8) %MD) local_unnamed_addr {
alloca_0:
  br label %do.body2

do.body2:                                         ; preds = %do.epilog9, %alloca_0
  %M0 = phi i32 [ 1, %alloca_0 ], [ %add.5, %do.epilog9 ]
  br label %do.body6

do.body6:                                         ; preds = %do.epilog13, %do.body2
  %indvars.iv24 = phi i64 [ %indvars.iv.next25, %do.epilog13 ], [ 0, %do.body2 ]
  %MD10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 32, ptr nonnull elementtype(double) %MD, i64 %indvars.iv24), !llfort.type_idx !0
  %indvars.iv.next25 = add nuw nsw i64 %indvars.iv24, 1
  %COND_sub1 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 32, ptr nonnull elementtype(double) %COND, i64 %indvars.iv.next25)
  %0 = add nuw nsw i64 %indvars.iv24, 2
  %COND6 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 32, ptr nonnull elementtype(double) %COND, i64 %0)
  br label %do.body10

do.body10:                                        ; preds = %bb3_endif, %do.body6
  %indvars.iv = phi i64 [ %indvars.iv.next, %bb3_endif ], [ 0, %do.body6 ]
  %MD11 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %MD10, i64 %indvars.iv), !llfort.type_idx !0
  %fetch3 = load double, ptr %MD11, align 1, !tbaa !1
  %rel.1 = fcmp reassoc ninf nsz arcp contract afn olt double %fetch3, 1.000000e+03
  br i1 %rel.1, label %bb_new15_then, label %bb3_endif

bb_new15_then:                                    ; preds = %do.body10
  %COND_sub2 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %COND_sub1, i64 %indvars.iv), !llfort.type_idx !0
  %COND_fetch9 = load double, ptr %COND_sub2, align 1, !tbaa !6, !llfort.type_idx !8
  %mul.4 = fmul reassoc ninf nsz arcp contract afn double %COND_fetch9, %fetch3
  %COND7 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(double) %COND6, i64 %indvars.iv), !llfort.type_idx !0
  store double %mul.4, ptr %COND7, align 1, !tbaa !6
  br label %bb3_endif

bb3_endif:                                        ; preds = %do.body10, %bb_new15_then
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 4
  br i1 %exitcond.not, label %do.epilog13, label %do.body10

do.epilog13:                                      ; preds = %bb3_endif
  %exitcond27.not = icmp eq i64 %indvars.iv.next25, 24
  br i1 %exitcond27.not, label %do.epilog9, label %do.body6

do.epilog9:                                       ; preds = %do.epilog13
  %add.5 = add nuw nsw i32 %M0, 1
  %exitcond28.not = icmp eq i32 %add.5, 1001
  br i1 %exitcond28.not, label %do.epilog5, label %do.body2

do.epilog5:                                       ; preds = %do.epilog9
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

!omp_offload.info = !{}

!0 = !{i64 6}
!1 = !{!2, !2, i64 0}
!2 = !{!"ifx$unique_sym$4", !3, i64 0}
!3 = !{!"Fortran Data Symbol", !4, i64 0}
!4 = !{!"Generic Fortran Symbol", !5, i64 0}
!5 = !{!"ifx$root$1$cam4_"}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$5", !3, i64 0}
!8 = !{i64 42}
