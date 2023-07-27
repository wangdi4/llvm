; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -aa-pipeline="tbaa" -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s

; This test case checks the following-
; - We do not assume that alloca stores can be optimized out in pre-vec pass as it is an optimistic assumption. The only GEP savings here are due to address simplification (base ptr + index) of (%4)[0][8 * i1 + i2].
; - The scalar savings for each of (%1)[16 * i1 + i2], (%2)[32 * i1 + i2] and (%4)[0][8 * i1 + i2] is two. One each from simplification of multiplication and addition in the index. The total savings is 384 (8 * 8 * 3 * 2).

; + DO i1 = 0, 7, 1   <DO_LOOP>
; |   + DO i2 = 0, 7, 1   <DO_LOOP>
; |   |   %14 = (%1)[16 * i1 + i2];
; |   |   %17 = (%2)[32 * i1 + i2];
; |   |   (%4)[0][8 * i1 + i2] = zext.i8.i16(%14) + -1 * zext.i8.i16(%17);
; |   + END LOOP
; + END LOOP

; CHECK: Savings: 384
; CHECK: GEPSavings: 64

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define hidden void @sub8x8_dct8(ptr nocapture, ptr nocapture readonly, ptr nocapture readonly) {
  %4 = alloca [64 x i16], align 16
  br label %5

; <label>:6:                                      ; preds = %23, %3
  %6 = phi i64 [ %26, %23 ], [ 0, %3 ]
  %7 = phi ptr [ %24, %23 ], [ %1, %3 ]
  %8 = phi ptr [ %25, %23 ], [ %2, %3 ]
  %9 = shl i64 %6, 3
  br label %10

; <label>:11:                                     ; preds = %10, %5
  %11 = phi i64 [ 0, %5 ], [ %21, %10 ]
  %12 = getelementptr inbounds i8, ptr %7, i64 %11
  %13 = load i8, ptr %12, align 1, !tbaa !2
  %14 = zext i8 %13 to i16
  %15 = getelementptr inbounds i8, ptr %8, i64 %11
  %16 = load i8, ptr %15, align 1, !tbaa !2
  %17 = zext i8 %16 to i16
  %18 = sub nsw i16 %14, %17
  %19 = add nuw nsw i64 %11, %9
  %20 = getelementptr inbounds [64 x i16], ptr %4, i64 0, i64 %19
  store i16 %18, ptr %20, align 2, !tbaa !5
  %21 = add nuw nsw i64 %11, 1
  %22 = icmp eq i64 %21, 8
  br i1 %22, label %23, label %10

; <label>:24:                                     ; preds = %10
  %24 = getelementptr inbounds i8, ptr %7, i64 16
  %25 = getelementptr inbounds i8, ptr %8, i64 32
  %26 = add nuw nsw i64 %6, 1
  %27 = icmp eq i64 %26, 8
  br i1 %27, label %exit, label %5

exit:
  ret void
}

!0 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 57303327e688d928c77069562958db1ee842a174) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm e1ddb01eee50722b9313e9a66d518772edde6b02)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"short", !3, i64 0}
!7 = !{!8, !6, i64 0}
!8 = !{!"array@_ZTSA64_s", !6, i64 0}
