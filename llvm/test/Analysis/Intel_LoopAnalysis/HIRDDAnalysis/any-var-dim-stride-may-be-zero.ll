; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region -disable-output 2>&1 < %s | FileCheck %s

; Fortran only. Test checks that we create all (*) edges for mem refs
; that have variable dimension stride which may be zero(see attribute list #4).
; This happens when SUM or PRODUCT functions are involved.
; (%"var$2011")[i1][i2] has a variable stride %"var$11.04875".


;     subroutine sub(n1)
;     REAL(8) RDA(2)
;     REAL(8) RDA1(2,2)
;     integer n1
;
;     RDA1(:,1) = (/1,2/)
;     RDA1(:,2) = (/3,4/)
;     RDA = 0.0
;     RDA = sum(RDA1,n1)
;     print *, RDA
;     END


; HIR:
;            BEGIN REGION { }
;                  + DO i1 = 0, 1, 1   <DO_LOOP>
;                  |   + DO i2 = 0, 1, 1   <DO_LOOP>
;                  |   |   %add.15 = (%"var$2011")[i1][i2]  +  (@"sub_$RDA1")[i1][i2];
;                  |   |   (%"var$2011")[i1][i2] = %add.15;
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; DD graph for function sub_:
; CHECK-DAG: 11:12 (%"var$2011")[i1][i2] --> (%"var$2011")[i1][i2] ANTI (* *) (? ?)
; CHECK-DAG: 12:11 (%"var$2011")[i1][i2] --> (%"var$2011")[i1][i2] FLOW (* *) (? ?)
; CHECK-DAG: 12:12 (%"var$2011")[i1][i2] --> (%"var$2011")[i1][i2] OUTPUT (* *) (? ?)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"sub_$RDA1" = internal unnamed_addr global [2 x [2 x double]] zeroinitializer, align 16, !llfort.type_idx !0
@"sub_$RDA" = internal global [2 x double] zeroinitializer, align 16, !llfort.type_idx !1

; Function Attrs: nofree nounwind uwtable
define void @sub_(ptr noalias nocapture readonly dereferenceable(4) %"sub_$N1") local_unnamed_addr #0 !llfort.type_idx !4 {
alloca_0:
  %"var$12" = alloca [1 x i64], align 16, !llfort.type_idx !6
  %"sub_$N1_fetch.15" = load i32, ptr %"sub_$N1", align 1, !tbaa !28, !llfort.type_idx !30
  %rel.4.not = icmp eq i32 %"sub_$N1_fetch.15", 1
  br i1 %rel.4.not, label %bb_true, label %bb_false

bb_true:                                          ; preds = %alloca_0
  %"var$12[]81" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"var$12", i64 0), !llfort.type_idx !32
  br label %loop34.preheader.preheader

bb_false:                                         ; preds = %alloca_0
  %"var$12[]80" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"var$12", i64 0)
  br label %loop34.preheader.preheader

loop34.preheader.preheader:                       ; preds = %bb_true, %bb_false
  %"var$11.04875" = phi i64 [ 0, %bb_true ], [ 4, %bb_false ]
  %"var$12[]8" = phi ptr [ %"var$12[]81", %bb_true ], [ %"var$12[]80", %bb_false ]
  %.pre66 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 0, i64 8, ptr nonnull elementtype(i64) %"var$12", i64 0), !llfort.type_idx !31
  store i64 2, ptr %"var$12[]8", align 8, !tbaa !12
  %"var$12[]9_fetch.33.pr" = load i64, ptr %.pre66, align 8, !tbaa !12
  %"var$2011" = alloca double, i64 %"var$12[]9_fetch.33.pr", align 8, !llfort.type_idx !17
  br label %loop34.preheader

loop34.preheader:                                 ; preds = %loop34.preheader.preheader, %loop_exit32
  %"$loop_ctr7.058" = phi i64 [ 1, %loop34.preheader.preheader ], [ 2, %loop_exit32 ]
  %"var$2011[]14" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8, ptr nonnull elementtype(double) %"var$2011", i64 %"$loop_ctr7.058") #4, !llfort.type_idx !37
  %"sub_$RDA1[]12" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 16, ptr nonnull elementtype(double) @"sub_$RDA1", i64 %"$loop_ctr7.058"), !llfort.type_idx !38
  br label %loop_body31

loop_body31:                                      ; preds = %loop34.preheader, %loop_body31
  %"$loop_ctr6.057" = phi i64 [ 1, %loop34.preheader ], [ %add.16, %loop_body31 ]
  %"var$2011[][]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %"var$11.04875", ptr nonnull elementtype(double) %"var$2011[]14", i64 %"$loop_ctr6.057") #4, !llfort.type_idx !34
  %"sub_$RDA1[][]13" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"sub_$RDA1[]12", i64 %"$loop_ctr6.057"), !llfort.type_idx !35
  %"sub_$RDA1[][]_fetch.18" = load double, ptr %"sub_$RDA1[][]13", align 8, !tbaa !19, !llfort.type_idx !36
  %"var$2011[][]_fetch.40" = load double, ptr %"var$2011[][]", align 8, !tbaa !12, !llfort.type_idx !34
  %add.15 = fadd reassoc ninf nsz arcp contract afn double %"var$2011[][]_fetch.40", %"sub_$RDA1[][]_fetch.18"
  store double %add.15, ptr %"var$2011[][]", align 8, !tbaa !12
  %add.16 = add nuw nsw i64 %"$loop_ctr6.057", 1
  %exitcond63.not = icmp eq i64 %add.16, 3
  br i1 %exitcond63.not, label %loop_exit32, label %loop_body31

loop_exit32:                                      ; preds = %loop_body31
  %add.17 = add nuw nsw i64 %"$loop_ctr7.058", 1
  %exitcond64.not = icmp eq i64 %add.17, 3
  br i1 %exitcond64.not, label %loop_exit40, label %loop34.preheader

loop_exit40:                                      ; preds = %loop_exit32
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "loopopt-pipeline"="full" }
attributes #1 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #4 = { "stride-may-be-zero" }

!omp_offload.info = !{}

!0 = !{i64 17}
!1 = !{i64 15}
!2 = !{i64 36}
!3 = !{i64 43}
!4 = !{i64 31}
!5 = !{i64 23}
!6 = !{i64 54}
!7 = !{i64 65}
!8 = !{i64 67}
!9 = !{i64 33}
!10 = !{i64 40}
!11 = !{i64 37}
!12 = !{!13, !13, i64 0}
!13 = !{!"Fortran Data Symbol", !14, i64 0}
!14 = !{!"Generic Fortran Symbol", !15, i64 0}
!15 = !{!"ifx$root$1$sub_"}
!16 = !{i64 38}
!17 = !{i64 6}
!18 = !{i64 34}
!19 = !{!20, !20, i64 0}
!20 = !{!"ifx$unique_sym$1", !13, i64 0}
!21 = !{i64 44}
!22 = !{i64 45}
!23 = !{i64 41}
!24 = !{i64 46}
!25 = !{!26, !26, i64 0}
!26 = !{!"ifx$unique_sym$2", !13, i64 0}
!27 = !{i64 11}
!28 = !{!29, !29, i64 0}
!29 = !{!"ifx$unique_sym$3", !13, i64 0}
!30 = !{i64 50}
!31 = !{i64 57}
!32 = !{i64 56}
!33 = !{i64 58}
!34 = !{i64 60}
!35 = !{i64 52}
!36 = !{i64 53}
!37 = !{i64 59}
!38 = !{i64 51}
!39 = !{i64 61}
!40 = !{i64 48}
!41 = !{i64 20}
!42 = !{i64 68}
!43 = !{!44, !44, i64 0}
!44 = !{!"ifx$unique_sym$4", !13, i64 0}
!45 = !{i64 69}
!46 = !{!47, !47, i64 0}
!47 = !{!"ifx$unique_sym$5", !13, i64 0}
!48 = !{i64 2}
!49 = !{i64 62}
!50 = !{i64 63}
!51 = !{i32 334}
!52 = !{i64 66}
