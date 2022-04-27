; RUN: llc < %s -O3 -intel-opt-report=high -intel-opt-report-emitter=mir -opt-report-embed -enable-protobuf-opt-report=true -debug-only=opt-report-support-utils 2>&1 | FileCheck %s
; REQUIRES: asserts, proto_bor

; CHECK:      Global Mloop optimization report for : unroll_remainder
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN
; CHECK-NEXT:     remark #25439: Loop unrolled with remainder by 2
; CHECK-NEXT: LOOP END
; CHECK-NEXT: =================================================================
; CHECK-EMPTY:
; CHECK-NEXT: Global Mloop optimization report for : unroll_no_remainder
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN
; CHECK-NEXT:     remark #25438: Loop unrolled without remainder by 2
; CHECK-NEXT: LOOP END
; CHECK-NEXT: =================================================================

; CHECK-LABEL:    --- Start Protobuf Binary OptReport Printer ---
; CHECK-NEXT:     Version: 1.5
; CHECK-NEXT:     Property Message Map:
; CHECK-DAG:        C_LOOP_UNROLL_WITHOUT_REMAINDER --> Loop unrolled without remainder by %d
; CHECK-DAG:        C_LOOP_UNROLL_WITH_REMAINDER --> Loop unrolled with remainder by %d
; CHECK-NEXT:     Number of reports: 2

; CHECK-DAG:      === Loop Begin ===
; CHECK-DAG:      Anchor ID: 04ed492b015245a23465c13db2906995
; CHECK-DAG:      Number of remarks: 1
; CHECK-DAG:        Property: C_LOOP_UNROLL_WITH_REMAINDER, Remark ID: 25439, Remark Args: 2
; CHECK-DAG:      ==== Loop End ====

; CHECK-DAG:      === Loop Begin ===
; CHECK-DAG:      Anchor ID: 1b41c12c5f48cc862488f7d568b3309c
; CHECK-DAG:      Number of remarks: 1
; CHECK-DAG:        Property: C_LOOP_UNROLL_WITHOUT_REMAINDER, Remark ID: 25438, Remark Args: 2
; CHECK-DAG:      ==== Loop End ====
; CHECK:          --- End Protobuf Binary OptReport Printer ---

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @unroll_remainder(i32* noalias nocapture %A, i32 %N) local_unnamed_addr {
entry:
  %t8 = alloca i32, align 4
  %i1.i64 = alloca i64, align 8
  %t14 = alloca i64, align 8
  %cmp6 = icmp sgt i32 %N, 1
  br i1 %cmp6, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  %0 = sext i32 %N to i64
  %1 = add i64 %0, -1
  %2 = udiv i64 %1, 2
  store i64 %2, i64* %t14, align 8
  %t14. = load i64, i64* %t14, align 8
  %hir.cmp.16 = icmp ult i64 0, %t14.
  br i1 %hir.cmp.16, label %then.16, label %ifmerge.16

for.cond.cleanup:                                 ; preds = %ifmerge.16, %then.17, %entry
  ret void

then.16:                                          ; preds = %for.body.lr.ph
  store i64 0, i64* %i1.i64, align 8
  %t14.1 = load i64, i64* %t14, align 8
  %3 = add i64 %t14.1, -1
  br label %loop.15

loop.15:                                          ; preds = %loop.15, %then.16
  %4 = load i64, i64* %i1.i64, align 8
  %5 = mul i64 2, %4
  %6 = getelementptr inbounds i32, i32* %A, i64 %5
  %gepload = load i32, i32* %6, align 4, !tbaa !1
  store i32 %gepload, i32* %t8, align 4
  %7 = load i64, i64* %i1.i64, align 8
  %8 = mul i64 2, %7
  %9 = add i64 %8, 1
  %10 = getelementptr inbounds i32, i32* %A, i64 %9
  %t8. = load i32, i32* %t8, align 4
  store i32 %t8., i32* %10, align 4, !tbaa !1
  %11 = load i64, i64* %i1.i64, align 8
  %12 = mul i64 2, %11
  %13 = add i64 %12, 1
  %14 = getelementptr inbounds i32, i32* %A, i64 %13
  %gepload2 = load i32, i32* %14, align 4, !tbaa !1
  store i32 %gepload2, i32* %t8, align 4
  %15 = load i64, i64* %i1.i64, align 8
  %16 = mul i64 2, %15
  %17 = add i64 %16, 2
  %18 = getelementptr inbounds i32, i32* %A, i64 %17
  %t8.3 = load i32, i32* %t8, align 4
  store i32 %t8.3, i32* %18, align 4, !tbaa !1
  %19 = load i64, i64* %i1.i64, align 8
  %nextivloop.15 = add nuw nsw i64 %19, 1
  store i64 %nextivloop.15, i64* %i1.i64, align 8
  %condloop.15 = icmp ne i64 %19, %3
  br i1 %condloop.15, label %loop.15, label %ifmerge.16, !llvm.loop !5

ifmerge.16:                                       ; preds = %loop.15, %for.body.lr.ph
  %t14.4 = load i64, i64* %t14, align 8
  %20 = shl i64 %t14.4, 1
  %21 = sext i32 %N to i64
  %22 = add i64 %21, -1
  %hir.cmp.17 = icmp ult i64 %20, %22
  br i1 %hir.cmp.17, label %then.17, label %for.cond.cleanup

then.17:                                          ; preds = %ifmerge.16
  %t14.5 = load i64, i64* %t14, align 8
  %23 = shl i64 %t14.5, 1
  %24 = getelementptr inbounds i32, i32* %A, i64 %23
  %gepload6 = load i32, i32* %24, align 4, !tbaa !1
  store i32 %gepload6, i32* %t8, align 4
  %t14.7 = load i64, i64* %t14, align 8
  %25 = shl i64 %t14.7, 1
  %26 = add i64 %25, 1
  %27 = getelementptr inbounds i32, i32* %A, i64 %26
  %t8.8 = load i32, i32* %t8, align 4
  store i32 %t8.8, i32* %27, align 4, !tbaa !1
  br label %for.cond.cleanup
}

define void @unroll_no_remainder(i32* noalias nocapture %A) local_unnamed_addr {
entry:
  %t7 = alloca i32, align 4
  %i1.i64 = alloca i64, align 8
  store i64 0, i64* %i1.i64, align 8
  br label %loop.14

loop.14:                                          ; preds = %loop.14, %entry
  %0 = load i64, i64* %i1.i64, align 8
  %1 = mul i64 2, %0
  %2 = add i64 %1, -1
  %3 = getelementptr inbounds i32, i32* %A, i64 %2
  %gepload = load i32, i32* %3, align 4, !tbaa !1
  store i32 %gepload, i32* %t7, align 4
  %4 = load i64, i64* %i1.i64, align 8
  %5 = mul i64 2, %4
  %6 = getelementptr inbounds i32, i32* %A, i64 %5
  %t7. = load i32, i32* %t7, align 4
  store i32 %t7., i32* %6, align 4, !tbaa !1
  %7 = load i64, i64* %i1.i64, align 8
  %8 = mul i64 2, %7
  %9 = getelementptr inbounds i32, i32* %A, i64 %8
  %gepload1 = load i32, i32* %9, align 4, !tbaa !1
  store i32 %gepload1, i32* %t7, align 4
  %10 = load i64, i64* %i1.i64, align 8
  %11 = mul i64 2, %10
  %12 = add i64 %11, 1
  %13 = getelementptr inbounds i32, i32* %A, i64 %12
  %t7.2 = load i32, i32* %t7, align 4
  store i32 %t7.2, i32* %13, align 4, !tbaa !1
  %14 = load i64, i64* %i1.i64, align 8
  %nextivloop.14 = add nuw nsw i64 %14, 1
  store i64 %nextivloop.14, i64* %i1.i64, align 8
  %condloop.14 = icmp ne i64 %14, 511
  br i1 %condloop.14, label %loop.14, label %afterloop.14, !llvm.loop !11

afterloop.14:                                     ; preds = %loop.14
  ret void
}

; Function Attrs: nofree nosync nounwind readnone willreturn
declare i64 @llvm.ssa.copy.i64(i64 returned) #0

attributes #0 = { nofree nosync nounwind readnone willreturn }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = distinct !{!5, !6, !7}
!6 = !{!"llvm.loop.unroll.disable"}
!7 = distinct !{!"intel.optreport.rootnode", !8}
!8 = distinct !{!"intel.optreport", !9}
!9 = !{!"intel.optreport.remarks", !10}
!10 = !{!"intel.optreport.remark", i32 25439, !"Loop unrolled with remainder by %d", i32 2}
!11 = distinct !{!11, !6, !12}
!12 = distinct !{!"intel.optreport.rootnode", !13}
!13 = distinct !{!"intel.optreport", !14}
!14 = !{!"intel.optreport.remarks", !15}
!15 = !{!"intel.optreport.remark", i32 25438, !"Loop unrolled without remainder by %d", i32 2}
