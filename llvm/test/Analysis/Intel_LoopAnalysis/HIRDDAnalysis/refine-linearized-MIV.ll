; RUN: opt -passes="hir-ssa-deconstruction,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output <%s 2>&1 | FileCheck %s

; Test checks that DD analysis refines the (* *) edge between <11> and <13> to (= >).
; But does not refine the edge (*) between <11> and <29>.

;            BEGIN REGION { }
;<44>               + DO i1 = 0, 80, 1   <DO_LOOP>
;<45>               |   + DO i2 = 0, i1, 1   <DO_LOOP>  <MAX_TC_EST = 81>  <LEGAL_MAX_TC = 81>
;<8>                |   |   %D_fetch3 = (%D)[i1 + -1 * i2 + 1];
;<11>               |   |   %D_fetch2 = (%D)[i1 + -1 * i2 + 2];
;<12>               |   |   %add.3 = %D_fetch2  +  %D_fetch3;
;<13>               |   |   (%D)[i1 + -1 * i2 + 1] = %add.3;
;<45>               |   + END LOOP
;<45>               |
;<46>               |
;<46>               |   + DO i2 = 0, i1, 1   <DO_LOOP>  <MAX_TC_EST = 81>  <LEGAL_MAX_TC = 81>
;<27>               |   |   %D_fetch = (%D)[i1 + -1 * i2 + 1];
;<28>               |   |   %add.7 = %D_fetch  +  1.000000e+00;
;<29>               |   |   (%D)[i1 + -1 * i2 + 1] = %add.7;
;<46>               |   + END LOOP
;<44>               + END LOOP
;<0>          END REGION

; CHECK: DD graph for function sub_:
; The edge between <11> and <13> was refined:
; CHECK-DAG : (%D)[i1 + -1 * i2 + 2] --> (%D)[i1 + -1 * i2 + 1] ANTI (* >) (? -1)
; The edge between <11> and <29> was not refined:
; CHECK-DAG : (%D)[i1 + -1 * i2 + 1] --> (%D)[i1 + -1 * i2 + 2] FLOW (*) (?)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @sub_(ptr noalias nocapture dereferenceable(8) %D, ptr noalias nocapture readnone dereferenceable(8) %"sub_$A", ptr noalias nocapture readonly dereferenceable(4) %"sub_$N") local_unnamed_addr {
alloca_0:
  %"sub_$N_fetch.1" = load i32, ptr %"sub_$N", align 1, !tbaa !0, !llfort.type_idx !5
  %sub.3 = sub i32 -2, %"sub_$N_fetch.1"
  %int_sext = sext i32 %sub.3 to i64, !llfort.type_idx !6
  br label %do.body3

do.body3:                                         ; preds = %do.end_do16, %alloca_0
  %indvars.iv55 = phi i64 [ %indvars.iv.next56, %do.end_do16 ], [ 0, %alloca_0 ]
  %indvars.iv53 = phi i64 [ %indvars.iv.next54, %do.end_do16 ], [ 1, %alloca_0 ]
  br label %do.body7

do.body7:                                         ; preds = %do.body3, %do.body7
  %indvars.iv = phi i64 [ 0, %do.body3 ], [ %indvars.iv.next, %do.body7 ]
  %0 = sub nsw i64 %indvars.iv55, %indvars.iv
  %1 = add nsw i64 %0, 1
  %"sub_$D[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext, i64 8, ptr nonnull elementtype(double) %D, i64 %1), !llfort.type_idx !7
  %D_fetch3 = load double, ptr %"sub_$D[]", align 1, !tbaa !8, !llfort.type_idx !10
  %2 = add nsw i64 %0, 2
  %"sub_$D[]5" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext, i64 8, ptr nonnull elementtype(double) %D, i64 %2), !llfort.type_idx !7
  %D_fetch2 = load double, ptr %"sub_$D[]5", align 1, !tbaa !8, !llfort.type_idx !11
  %add.3 = fadd reassoc ninf nsz arcp contract afn double %D_fetch2, %D_fetch3
  store double %add.3, ptr %"sub_$D[]", align 1, !tbaa !8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %indvars.iv53
  br i1 %exitcond, label %do.body15.preheader, label %do.body7

do.body15.preheader:                              ; preds = %do.body7
  %indvars.iv.next56 = add nuw nsw i64 %indvars.iv55, 1
  br label %do.body15

do.body15:                                        ; preds = %do.body15.preheader, %do.body15
  %indvars.iv48 = phi i64 [ 0, %do.body15.preheader ], [ %indvars.iv.next49, %do.body15 ]
  %3 = sub nsw i64 %indvars.iv.next56, %indvars.iv48
  %"sub_$D[]12" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext, i64 8, ptr nonnull elementtype(double) %D, i64 %3), !llfort.type_idx !7
  %D_fetch = load double, ptr %"sub_$D[]12", align 1, !tbaa !8, !llfort.type_idx !12
  %add.7 = fadd reassoc ninf nsz arcp contract afn double %D_fetch, 1.000000e+00
  store double %add.7, ptr %"sub_$D[]12", align 1, !tbaa !8
  %indvars.iv.next49 = add nuw nsw i64 %indvars.iv48, 1
  %exitcond52 = icmp eq i64 %indvars.iv.next49, %indvars.iv53
  br i1 %exitcond52, label %do.end_do16, label %do.body15

do.end_do16:                                      ; preds = %do.body15
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond57.not = icmp eq i64 %indvars.iv.next56, 81
  br i1 %exitcond57.not, label %do.epilog6, label %do.body3

do.epilog6:                                       ; preds = %do.end_do16
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

!omp_offload.info = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$1", !2, i64 0}
!2 = !{!"Fortran Data Symbol", !3, i64 0}
!3 = !{!"Generic Fortran Symbol", !4, i64 0}
!4 = !{!"ifx$root$1$sub_"}
!5 = !{i64 45}
!6 = !{i64 3}
!7 = !{i64 25}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$4", !2, i64 0}
!10 = !{i64 47}
!11 = !{i64 48}
!12 = !{i64 49}
