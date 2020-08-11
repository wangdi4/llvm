; REQUIRES: asserts

; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -tbaa -hir-pre-vec-complete-unroll -debug-only=hir-complete-unroll 2>&1 < %s | FileCheck %s
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

define hidden void @sub8x8_dct8(i16* nocapture, i8* nocapture readonly, i8* nocapture readonly) {
  %4 = alloca [64 x i16], align 16
  %5 = bitcast [64 x i16]* %4 to i8*
  br label %6

; <label>:6:                                      ; preds = %24, %3
  %7 = phi i64 [ %27, %24 ], [ 0, %3 ]
  %8 = phi i8* [ %25, %24 ], [ %1, %3 ]
  %9 = phi i8* [ %26, %24 ], [ %2, %3 ]
  %10 = shl i64 %7, 3
  br label %11

; <label>:11:                                     ; preds = %11, %6
  %12 = phi i64 [ 0, %6 ], [ %22, %11 ]
  %13 = getelementptr inbounds i8, i8* %8, i64 %12
  %14 = load i8, i8* %13, align 1, !tbaa !2
  %15 = zext i8 %14 to i16
  %16 = getelementptr inbounds i8, i8* %9, i64 %12
  %17 = load i8, i8* %16, align 1, !tbaa !2
  %18 = zext i8 %17 to i16
  %19 = sub nsw i16 %15, %18
  %20 = add nuw nsw i64 %12, %10
  %21 = getelementptr inbounds [64 x i16], [64 x i16]* %4, i64 0, i64 %20
  store i16 %19, i16* %21, align 2, !tbaa !5
  %22 = add nuw nsw i64 %12, 1
  %23 = icmp eq i64 %22, 8
  br i1 %23, label %24, label %11

; <label>:24:                                     ; preds = %11
  %25 = getelementptr inbounds i8, i8* %8, i64 16
  %26 = getelementptr inbounds i8, i8* %9, i64 32
  %27 = add nuw nsw i64 %7, 1
  %28 = icmp eq i64 %27, 8
  br i1 %28, label %exit, label %6

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
