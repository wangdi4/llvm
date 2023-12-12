; REQUIRES: asserts
; RUN: opt -disable-hir-inter-loop-blocking=false -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-inter-loop-blocking,print<hir>" -aa-pipeline="basic-aa" -debug-only=hir-inter-loop-blocking,hir-inter-loop-blocking-profit -hir-inter-loop-blocking-print-info -hir-inter-loop-blocking-stripmine-size=2 -disable-output 2>&1 < %s | FileCheck %s

; Verify that this code is not inter-loop-blocked.
; Among four i2-innermost loops, first two i2-loops are found to be candidates for inter-loop-blocking around those two loops.
; On the other hands, the third i2-loops is not valid candidates.
; In the transformation phase, inter loop blocking's regards the lowest common ancestor loop of the first two loops, i1-loop,
; as the node outside by-strip loops.
; Also, it uses if-stmt, the lowest common lexical ancestor of the two loops as an anchor.
; Before that anchor by-strip loops are placed. That will make the whole if-stmt into the body of the by-strip loop, which is not correct.
;
; Thus, inter loop blocking should bailout upon a presence of "the lowest common ancestor deeper than LCA loop" of
; a candidate loop and a non-candidate loop. Here, a candidate loop is the second i2-loop and a non-candidate loop is the third i2-loop.
; LCA loop of the two is i1-loop and the lowest common lexical ancestor of the second and the third i2-loops is if-stmt.

; CHECK: Function: foo_

; CHECK:         BEGIN REGION { }
; CHECK:               + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:               |   if (%"foo_$N_fetch.1" < %"foo_$M_fetch.2")
; CHECK:               |   {
; CHECK:               |      + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK:               |      |   (%"foo_$FIVE")[i2] = (%"foo_$ZERO")[i2][%"foo_$N_fetch.1"];
; CHECK:               |      + END LOOP
; CHECK:               |
; CHECK:               |
; CHECK:               |      + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK:               |      |   (%"foo_$TEN")[i2] = (%"foo_$ZERO")[i2][%"foo_$M_fetch.2"];
; CHECK:               |      + END LOOP
; CHECK:               |
; CHECK:               |
; CHECK:               |      + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK:               |      |   (%"foo_$ZERO")[i2][%"foo_$M_fetch.2"] = (%"foo_$TEN")[i2];
; CHECK:               |      + END LOOP
; CHECK:               |
; CHECK:               |
; CHECK:               |      + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK:               |      |   (%"foo_$ZERO")[i2][%"foo_$N_fetch.1"] = (%"foo_$FIVE")[i2];
; CHECK:               |      + END LOOP
; CHECK:               |   }
; CHECK:               + END LOOP
; CHECK:         END REGION

; CHECK: Member loop and non-member loop cannot be cut.
; CHECK-NOT: Found Legal & Profitable Cand:false

; CHECK: Function: foo_

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:              |   if (%"foo_$N_fetch.1" < %"foo_$M_fetch.2")
; CHECK:              |   {
; CHECK:              |      + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK:              |      |   (%"foo_$FIVE")[i2] = (%"foo_$ZERO")[i2][%"foo_$N_fetch.1"];
; CHECK:              |      + END LOOP
; CHECK:              |
; CHECK:              |
; CHECK:              |      + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK:              |      |   (%"foo_$TEN")[i2] = (%"foo_$ZERO")[i2][%"foo_$M_fetch.2"];
; CHECK:              |      + END LOOP
; CHECK:              |
; CHECK:              |
; CHECK:              |      + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK:              |      |   (%"foo_$ZERO")[i2][%"foo_$M_fetch.2"] = (%"foo_$TEN")[i2];
; CHECK:              |      + END LOOP
; CHECK:              |
; CHECK:              |
; CHECK:              |      + DO i2 = 0, 8, 1   <DO_LOOP>
; CHECK:              |      |   (%"foo_$ZERO")[i2][%"foo_$N_fetch.1"] = (%"foo_$FIVE")[i2];
; CHECK:              |      + END LOOP
; CHECK:              |   }
; CHECK:              + END LOOP
; CHECK:        END REGION

;Module Before HIR
; ModuleID = 'exchange.f90'
source_filename = "exchange.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define void @foo_(ptr noalias nocapture dereferenceable(8) %"foo_$FIVE", ptr noalias nocapture dereferenceable(8) %"foo_$ZERO", ptr noalias nocapture dereferenceable(8) %"foo_$TEN", ptr noalias nocapture readonly dereferenceable(4) %"foo_$N", ptr noalias nocapture readonly dereferenceable(4) %"foo_$M") local_unnamed_addr #0 {
alloca_0:
  %"foo_$N_fetch.1" = load i32, ptr %"foo_$N", align 1, !tbaa !0
  %"foo_$M_fetch.2" = load i32, ptr %"foo_$M", align 1, !tbaa !5
  %rel.1 = icmp slt i32 %"foo_$N_fetch.1", %"foo_$M_fetch.2"
  %int_sext = sext i32 %"foo_$N_fetch.1" to i64
  %int_sext3 = sext i32 %"foo_$M_fetch.2" to i64
  br label %do.body2

do.body2:                                         ; preds = %bb3_endif, %alloca_0
  %"foo_$I.0" = phi i32 [ 0, %alloca_0 ], [ %add.13, %bb3_endif ]
  br i1 %rel.1, label %do.body7.preheader, label %bb3_endif

do.body7.preheader:                               ; preds = %do.body2
  br label %do.body7

do.body7:                                         ; preds = %do.body7.preheader, %do.body7
  %indvars.iv = phi i64 [ %indvars.iv.next, %do.body7 ], [ 0, %do.body7.preheader ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %"foo_$ZERO[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8000, ptr nonnull elementtype(double) %"foo_$ZERO", i64 %indvars.iv.next), !llfort.type_idx !7, !ifx.array_extent !8
  %"foo_$ZERO[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"foo_$ZERO[]", i64 %int_sext), !llfort.type_idx !7
  %"foo_$ZERO[][]_fetch.5" = load double, ptr %"foo_$ZERO[][]", align 1, !tbaa !9, !llfort.type_idx !11
  %"foo_$FIVE[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"foo_$FIVE", i64 %indvars.iv.next), !llfort.type_idx !7, !ifx.array_extent !8
  store double %"foo_$ZERO[][]_fetch.5", ptr %"foo_$FIVE[]", align 1, !tbaa !12
  %exitcond.not = icmp eq i64 %indvars.iv.next, 9
  br i1 %exitcond.not, label %do.body13.preheader, label %do.body7

do.body13.preheader:                              ; preds = %do.body7
  br label %do.body13

do.body13:                                        ; preds = %do.body13.preheader, %do.body13
  %indvars.iv37 = phi i64 [ %indvars.iv.next38, %do.body13 ], [ 0, %do.body13.preheader ]
  %indvars.iv.next38 = add nuw nsw i64 %indvars.iv37, 1
  %"foo_$ZERO[]5" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8000, ptr nonnull elementtype(double) %"foo_$ZERO", i64 %indvars.iv.next38), !llfort.type_idx !7, !ifx.array_extent !8
  %"foo_$ZERO[][]6" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"foo_$ZERO[]5", i64 %int_sext3), !llfort.type_idx !7
  %"foo_$ZERO[][]_fetch.11" = load double, ptr %"foo_$ZERO[][]6", align 1, !tbaa !9, !llfort.type_idx !14
  %"foo_$TEN[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"foo_$TEN", i64 %indvars.iv.next38), !llfort.type_idx !7, !ifx.array_extent !8
  store double %"foo_$ZERO[][]_fetch.11", ptr %"foo_$TEN[]", align 1, !tbaa !15
  %exitcond39.not = icmp eq i64 %indvars.iv.next38, 9
  br i1 %exitcond39.not, label %do.body18.preheader, label %do.body13

do.body18.preheader:                              ; preds = %do.body13
  br label %do.body18

do.body18:                                        ; preds = %do.body18.preheader, %do.body18
  %indvars.iv40 = phi i64 [ %indvars.iv.next41, %do.body18 ], [ 0, %do.body18.preheader ]
  %indvars.iv.next41 = add nuw nsw i64 %indvars.iv40, 1
  %"foo_$TEN[]9" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"foo_$TEN", i64 %indvars.iv.next41), !llfort.type_idx !7, !ifx.array_extent !8
  %"foo_$TEN[]_fetch.16" = load double, ptr %"foo_$TEN[]9", align 1, !tbaa !15, !llfort.type_idx !17
  %"foo_$ZERO[]12" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8000, ptr nonnull elementtype(double) %"foo_$ZERO", i64 %indvars.iv.next41), !llfort.type_idx !7, !ifx.array_extent !8
  %"foo_$ZERO[][]13" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"foo_$ZERO[]12", i64 %int_sext3), !llfort.type_idx !7
  store double %"foo_$TEN[]_fetch.16", ptr %"foo_$ZERO[][]13", align 1, !tbaa !9
  %exitcond42.not = icmp eq i64 %indvars.iv.next41, 9
  br i1 %exitcond42.not, label %do.body23.preheader, label %do.body18

do.body23.preheader:                              ; preds = %do.body18
  br label %do.body23

do.body23:                                        ; preds = %do.body23.preheader, %do.body23
  %indvars.iv43 = phi i64 [ %indvars.iv.next44, %do.body23 ], [ 0, %do.body23.preheader ]
  %indvars.iv.next44 = add nuw nsw i64 %indvars.iv43, 1
  %"foo_$FIVE[]15" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"foo_$FIVE", i64 %indvars.iv.next44), !llfort.type_idx !7, !ifx.array_extent !8
  %"foo_$FIVE[]_fetch.22" = load double, ptr %"foo_$FIVE[]15", align 1, !tbaa !12, !llfort.type_idx !18
  %"foo_$ZERO[]18" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 8000, ptr nonnull elementtype(double) %"foo_$ZERO", i64 %indvars.iv.next44), !llfort.type_idx !7, !ifx.array_extent !8
  %"foo_$ZERO[][]19" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %"foo_$ZERO[]18", i64 %int_sext), !llfort.type_idx !7
  store double %"foo_$FIVE[]_fetch.22", ptr %"foo_$ZERO[][]19", align 1, !tbaa !9
  %exitcond45.not = icmp eq i64 %indvars.iv.next44, 9
  br i1 %exitcond45.not, label %bb3_endif.loopexit, label %do.body23

bb3_endif.loopexit:                               ; preds = %do.body23
  br label %bb3_endif

bb3_endif:                                        ; preds = %bb3_endif.loopexit, %do.body2
  %add.13 = add nuw nsw i32 %"foo_$I.0", 1
  %exitcond46.not = icmp eq i32 %add.13, 3
  br i1 %exitcond46.not, label %do.epilog5, label %do.body2

do.epilog5:                                       ; preds = %bb3_endif
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "denormal-fp-math"="preserve-sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }

!omp_offload.info = !{}
!ifx.types.dv = !{}

!0 = !{!1, !1, i64 0}
!1 = !{!"ifx$unique_sym$2", !2, i64 0}
!2 = !{!"Fortran Data Symbol", !3, i64 0}
!3 = !{!"Generic Fortran Symbol", !4, i64 0}
!4 = !{!"ifx$root$1$foo_"}
!5 = !{!6, !6, i64 0}
!6 = !{!"ifx$unique_sym$3", !2, i64 0}
!7 = !{i64 6}
!8 = !{i64 1000}
!9 = !{!10, !10, i64 0}
!10 = !{!"ifx$unique_sym$5", !2, i64 0}
!11 = !{i64 56}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$6", !2, i64 0}
!14 = !{i64 59}
!15 = !{!16, !16, i64 0}
!16 = !{!"ifx$unique_sym$7", !2, i64 0}
!17 = !{i64 61}
!18 = !{i64 64}
