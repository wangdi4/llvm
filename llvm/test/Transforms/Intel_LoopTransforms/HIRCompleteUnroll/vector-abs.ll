; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -VPlanDriverHIR -hir-post-vec-complete-unroll -print-after=VPlanDriverHIR -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we are able to completely unroll the i1 loop with vector abs() idiom after vectorizer unrolls the i2 loop.

; HIR-
; + DO i1 = 0, 15, 1   <DO_LOOP>
; |   + DO i2 = 0, 15, 1   <DO_LOOP>
; |   |   %24 = (%0)[sext.i32.i64(%1) * i1 + i2];
; |   |   %27 = (%2)[sext.i32.i64(%3) * i1 + i2];
; |   |   %32 = (zext.i8.i32(%24) + -1 * zext.i8.i32(%27) < 0) ? -1 * zext.i8.i32(%24) + zext.i8.i32(%27) : zext.i8.i32(%24) + -1 * zext.i8.i32(%27);
; |   |   %9 = %32  +  %9;
; |   + END LOOP
; + END LOOP

; CHECK: Function: x264_pixel_sad_16x16

; CHECK: + DO i1 = 0, 15, 1   <DO_LOOP>
; CHECK: |   %.vec = (<16 x i8>*)(%0)[sext.i32.i64(%1) * i1];
; CHECK: |   %.vec2 = (<16 x i8>*)(%2)[sext.i32.i64(%3) * i1];
; CHECK: |   %NBConv = zext.<16 x i8>.<16 x i32>(%.vec);
; CHECK: |   %NBConv3 = zext.<16 x i8>.<16 x i32>(%.vec2);
; CHECK: |   %.vec4 = (%NBConv + -1 * %NBConv3 < 0) ? -1 * %NBConv + %NBConv3 : %NBConv + -1 * %NBConv3;
; CHECK: |   %result.vector = %.vec4 + %result.vector;
; CHECK: + END LOOP

; CHECK: Function: x264_pixel_sad_16x16

; CHECK-NOT: DO i1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readonly uwtable
define hidden i32 @x264_pixel_sad_16x16(i8* nocapture readonly, i32, i8* nocapture readonly, i32) #0 {
  %5 = sext i32 %1 to i64
  %6 = sext i32 %3 to i64
  br label %7

; <label>:7:                                      ; preds = %14, %4
  %8 = phi i32 [ 0, %4 ], [ %18, %14 ]
  %9 = phi i32 [ 0, %4 ], [ %15, %14 ]
  %10 = phi i8* [ %0, %4 ], [ %16, %14 ]
  %11 = phi i8* [ %2, %4 ], [ %17, %14 ]
  br label %20

; <label>:12:                                     ; preds = %14
  %13 = phi i32 [ %15, %14 ]
  ret i32 %13

; <label>:14:                                     ; preds = %20
  %15 = phi i32 [ %33, %20 ]
  %16 = getelementptr inbounds i8, i8* %10, i64 %5
  %17 = getelementptr inbounds i8, i8* %11, i64 %6
  %18 = add nuw nsw i32 %8, 1
  %19 = icmp eq i32 %18, 16
  br i1 %19, label %12, label %7

; <label>:20:                                     ; preds = %20, %7
  %21 = phi i64 [ 0, %7 ], [ %34, %20 ]
  %22 = phi i32 [ %9, %7 ], [ %33, %20 ]
  %23 = getelementptr inbounds i8, i8* %10, i64 %21
  %24 = load i8, i8* %23, align 1, !tbaa !2
  %25 = zext i8 %24 to i32
  %26 = getelementptr inbounds i8, i8* %11, i64 %21
  %27 = load i8, i8* %26, align 1, !tbaa !2
  %28 = zext i8 %27 to i32
  %29 = sub nsw i32 %25, %28
  %30 = icmp slt i32 %29, 0
  %31 = sub nsw i32 0, %29
  %32 = select i1 %30, i32 %31, i32 %29
  %33 = add nsw i32 %32, %22
  %34 = add nuw nsw i64 %21, 1
  %35 = icmp eq i64 %34, 16
  br i1 %35, label %14, label %20
}

attributes #0 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "prefer-vector-width"="256" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+rtm,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1}

!0 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 77914db7f30bcf59421b2746f16c18628c93c401) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 6662bfb533aa970662fae6c4798e1d05adbea4a5)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
