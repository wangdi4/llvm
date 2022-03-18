; RUN: llc < %s -O3 -intel-opt-report=high -intel-opt-report-emitter=mir -opt-report-embed -enable-protobuf-opt-report=true -debug-only=opt-report-support-utils 2>&1 | FileCheck %s
; REQUIRES: asserts, proto_bor

; CHECK:      Global Mloop optimization report for : foo
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN
; CHECK-NEXT:     remark #25540: Loop unrolled and jammed by 2
; CHECK-EMPTY:
; CHECK-NEXT:     LOOP BEGIN
; CHECK-NEXT:     LOOP END
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN
; CHECK-NEXT: <Remainder loop>
; CHECK-EMPTY:
; CHECK-NEXT:     LOOP BEGIN
; CHECK-NEXT:     LOOP END
; CHECK-NEXT: LOOP END
; CHECK-NEXT: =================================================================

; CHECK-LABEL:    --- Start Protobuf Binary OptReport Printer ---
; CHECK-NEXT:     Version: 1.5
; CHECK-NEXT:     Property Message Map:
; CHECK-DAG:        C_LOOP_UNROLL_AND_JAM --> Loop unrolled and jammed by %d
; CHECK-DAG:        C_LOOP_REMAINDER --> Remainder loop
; CHECK-NEXT:     Number of reports: 3

; CHECK-DAG:      === Loop Begin ===
; CHECK-DAG:      Anchor ID: acaeb5319062b23bfbf47f58b6360c4b
; CHECK-DAG:      Number of remarks: 0
; CHECK-DAG:      ==== Loop End ====

; CHECK-DAG:      === Loop Begin ===
; CHECK-DAG:      Anchor ID: e9a3f5d79e7ea2f824577e5b6b1e4d85
; CHECK-DAG:      Number of remarks: 1
; CHECK-DAG:        Property: C_LOOP_UNROLL_AND_JAM, Remark ID: 25540, Remark Args: 2
; CHECK-DAG:      ==== Loop End ====

; CHECK-DAG:      === Loop Begin ===
; CHECK-DAG:      Anchor ID: 486251fe7d339baf8de26afca68b1e6f
; CHECK-DAG:      Number of remarks: 1
; CHECK-DAG:        Property: C_LOOP_REMAINDER, Remark ID: 25491, Remark Args:
; CHECK-DAG:      ==== Loop End ====
; CHECK:          --- End Protobuf Binary OptReport Printer ---

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = common local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define void @foo(i32 %n) local_unnamed_addr {
entry:
  %t14 = alloca i32, align 4
  %t10 = alloca i32, align 4
  %i2.i32 = alloca i32, align 4
  %i1.i32 = alloca i32, align 4
  %t23 = alloca i32, align 4
  %cmp3 = icmp slt i32 0, %n
  br i1 %cmp3, label %region.0, label %for.end8

for.end8:                                         ; preds = %ifmerge.37, %afterloop.34, %entry
  ret void

region.0:                                         ; preds = %entry
  %0 = udiv i32 %n, 2
  store i32 %0, i32* %t23, align 4
  %t23. = load i32, i32* %t23, align 4
  %hir.cmp.37 = icmp ult i32 0, %t23.
  br i1 %hir.cmp.37, label %then.37, label %ifmerge.37

then.37:                                          ; preds = %region.0
  store i32 0, i32* %i1.i32, align 4
  %t23.2 = load i32, i32* %t23, align 4
  %1 = add i32 %t23.2, -1
  br label %loop.36

loop.36:                                          ; preds = %afterloop.40, %then.37
  store i32 0, i32* %i2.i32, align 4
  %2 = add i32 %n, -1
  br label %loop.40

loop.40:                                          ; preds = %loop.40, %loop.36
  %3 = load i32, i32* %i2.i32, align 4
  %4 = sext i32 %3 to i64
  %5 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %4
  %gepload = load i32, i32* %5, align 4
  store i32 %gepload, i32* %t10, align 4
  %6 = load i32, i32* %i1.i32, align 4
  %7 = mul i32 2, %6
  %8 = sext i32 %7 to i64
  %9 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %8
  %gepload3 = load i32, i32* %9, align 4
  store i32 %gepload3, i32* %t14, align 4
  %10 = load i32, i32* %i1.i32, align 4
  %11 = mul i32 2, %10
  %12 = sext i32 %11 to i64
  %13 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %12
  %t10. = load i32, i32* %t10, align 4
  %t14. = load i32, i32* %t14, align 4
  %14 = add i32 %t10., %t14.
  store i32 %14, i32* %13, align 4
  %15 = load i32, i32* %i2.i32, align 4
  %16 = sext i32 %15 to i64
  %17 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %16
  %gepload4 = load i32, i32* %17, align 4
  store i32 %gepload4, i32* %t10, align 4
  %18 = load i32, i32* %i1.i32, align 4
  %19 = mul i32 2, %18
  %20 = add i32 %19, 1
  %21 = sext i32 %20 to i64
  %22 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %21
  %gepload5 = load i32, i32* %22, align 4
  store i32 %gepload5, i32* %t14, align 4
  %23 = load i32, i32* %i1.i32, align 4
  %24 = mul i32 2, %23
  %25 = add i32 %24, 1
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %26
  %t10.6 = load i32, i32* %t10, align 4
  %t14.7 = load i32, i32* %t14, align 4
  %28 = add i32 %t10.6, %t14.7
  store i32 %28, i32* %27, align 4
  %29 = load i32, i32* %i2.i32, align 4
  %nextivloop.40 = add nuw nsw i32 %29, 1
  store i32 %nextivloop.40, i32* %i2.i32, align 4
  %condloop.40 = icmp ne i32 %29, %2
  br i1 %condloop.40, label %loop.40, label %afterloop.40, !llvm.loop !0

afterloop.40:                                     ; preds = %loop.40
  %30 = load i32, i32* %i1.i32, align 4
  %nextivloop.36 = add nuw nsw i32 %30, 1
  store i32 %nextivloop.36, i32* %i1.i32, align 4
  %condloop.36 = icmp ne i32 %30, %1
  br i1 %condloop.36, label %loop.36, label %ifmerge.37, !llvm.loop !3

ifmerge.37:                                       ; preds = %afterloop.40, %region.0
  %t23.8 = load i32, i32* %t23, align 4
  %31 = shl i32 %t23.8, 1
  %hir.cmp.38 = icmp ult i32 %31, %n
  br i1 %hir.cmp.38, label %then.38, label %for.end8

then.38:                                          ; preds = %ifmerge.37
  %t23.9 = load i32, i32* %t23, align 4
  %32 = shl i32 %t23.9, 1
  store i32 %32, i32* %i1.i32, align 4
  %33 = add i32 %n, -1
  br label %loop.33

loop.33:                                          ; preds = %afterloop.34, %then.38
  store i32 0, i32* %i2.i32, align 4
  %34 = add i32 %n, -1
  br label %loop.34

loop.34:                                          ; preds = %loop.34, %loop.33
  %35 = load i32, i32* %i2.i32, align 4
  %36 = sext i32 %35 to i64
  %37 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %36
  %gepload10 = load i32, i32* %37, align 4
  store i32 %gepload10, i32* %t10, align 4
  %38 = load i32, i32* %i1.i32, align 4
  %39 = sext i32 %38 to i64
  %40 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %39
  %gepload11 = load i32, i32* %40, align 4
  store i32 %gepload11, i32* %t14, align 4
  %41 = load i32, i32* %i1.i32, align 4
  %42 = sext i32 %41 to i64
  %43 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %42
  %t10.12 = load i32, i32* %t10, align 4
  %t14.13 = load i32, i32* %t14, align 4
  %44 = add i32 %t10.12, %t14.13
  store i32 %44, i32* %43, align 4
  %45 = load i32, i32* %i2.i32, align 4
  %nextivloop.34 = add nuw nsw i32 %45, 1
  store i32 %nextivloop.34, i32* %i2.i32, align 4
  %condloop.34 = icmp ne i32 %45, %34
  br i1 %condloop.34, label %loop.34, label %afterloop.34

afterloop.34:                                     ; preds = %loop.34
  %46 = load i32, i32* %i1.i32, align 4
  %nextivloop.33 = add nuw nsw i32 %46, 1
  store i32 %nextivloop.33, i32* %i1.i32, align 4
  %condloop.33 = icmp ne i32 %46, %33
  br i1 %condloop.33, label %loop.33, label %for.end8, !llvm.loop !9
}

; Function Attrs: nofree nosync nounwind readnone willreturn
declare i32 @llvm.ssa.copy.i32(i32 returned) #0

attributes #0 = { nofree nosync nounwind readnone willreturn }

!0 = distinct !{!0, !1}
!1 = distinct !{!"intel.optreport.rootnode", !2}
!2 = !{!"intel.optreport"}
!3 = distinct !{!3, !4, !5}
!4 = !{!"llvm.loop.unroll_and_jam.disable"}
!5 = distinct !{!"intel.optreport.rootnode", !6}
!6 = distinct !{!"intel.optreport", !7}
!7 = !{!"intel.optreport.remarks", !8}
!8 = !{!"intel.optreport.remark", i32 25540, !"Loop unrolled and jammed by %d", i32 2}
!9 = distinct !{!9, !4, !10, !11, !12}
!10 = !{!"llvm.loop.intel.loopcount_maximum", i32 1}
!11 = !{!"llvm.loop.unroll.disable"}
!12 = distinct !{!"intel.optreport.rootnode", !13}
!13 = distinct !{!"intel.optreport", !14}
!14 = !{!"intel.optreport.origin", !15}
!15 = !{!"intel.optreport.remark", i32 25491, !"Remainder loop"}
