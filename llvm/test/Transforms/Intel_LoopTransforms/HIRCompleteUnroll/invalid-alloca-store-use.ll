; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,hir-post-vec-complete-unroll,print<hir>" -disable-output 2>&1 < %s | FileCheck %s

; Verify that the first loop is not unrolled because the alloca stores are used in the second loop which is vectorized and we assume that scalar alloca stores cannot be propagated to vector loads.
; Note that the second loop is now completely unrolled after vectorization.

; CHECK: DO i1 = 0, 3, 1

; CHECK: DO i1 = 0, 3, 4

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly uwtable
define hidden i32 @x264_pixel_satd_8x4(ptr nocapture readonly, i32, ptr nocapture readonly, i32) #0 {
  %5 = alloca [4 x [4 x i32]], align 16
  %6 = sext i32 %1 to i64
  %7 = sext i32 %3 to i64
  br label %8

; <label>:9:                                      ; preds = %8, %4
  %9 = phi i64 [ 0, %4 ], [ %86, %8 ]
  %10 = phi ptr [ %0, %4 ], [ %87, %8 ]
  %11 = phi ptr [ %2, %4 ], [ %88, %8 ]
  %12 = load i8, ptr %10, align 1, !tbaa !2
  %13 = zext i8 %12 to i32
  %14 = load i8, ptr %11, align 1, !tbaa !2
  %15 = zext i8 %14 to i32
  %16 = sub nsw i32 %13, %15
  %17 = getelementptr inbounds i8, ptr %10, i64 4
  %18 = load i8, ptr %17, align 1, !tbaa !2
  %19 = zext i8 %18 to i32
  %20 = getelementptr inbounds i8, ptr %11, i64 4
  %21 = load i8, ptr %20, align 1, !tbaa !2
  %22 = zext i8 %21 to i32
  %23 = sub nsw i32 %19, %22
  %24 = shl nsw i32 %23, 16
  %25 = add nsw i32 %24, %16
  %26 = getelementptr inbounds i8, ptr %10, i64 1
  %27 = load i8, ptr %26, align 1, !tbaa !2
  %28 = zext i8 %27 to i32
  %29 = getelementptr inbounds i8, ptr %11, i64 1
  %30 = load i8, ptr %29, align 1, !tbaa !2
  %31 = zext i8 %30 to i32
  %32 = sub nsw i32 %28, %31
  %33 = getelementptr inbounds i8, ptr %10, i64 5
  %34 = load i8, ptr %33, align 1, !tbaa !2
  %35 = zext i8 %34 to i32
  %36 = getelementptr inbounds i8, ptr %11, i64 5
  %37 = load i8, ptr %36, align 1, !tbaa !2
  %38 = zext i8 %37 to i32
  %39 = sub nsw i32 %35, %38
  %40 = shl nsw i32 %39, 16
  %41 = add nsw i32 %40, %32
  %42 = getelementptr inbounds i8, ptr %10, i64 2
  %43 = load i8, ptr %42, align 1, !tbaa !2
  %44 = zext i8 %43 to i32
  %45 = getelementptr inbounds i8, ptr %11, i64 2
  %46 = load i8, ptr %45, align 1, !tbaa !2
  %47 = zext i8 %46 to i32
  %48 = sub nsw i32 %44, %47
  %49 = getelementptr inbounds i8, ptr %10, i64 6
  %50 = load i8, ptr %49, align 1, !tbaa !2
  %51 = zext i8 %50 to i32
  %52 = getelementptr inbounds i8, ptr %11, i64 6
  %53 = load i8, ptr %52, align 1, !tbaa !2
  %54 = zext i8 %53 to i32
  %55 = sub nsw i32 %51, %54
  %56 = shl nsw i32 %55, 16
  %57 = add nsw i32 %56, %48
  %58 = getelementptr inbounds i8, ptr %10, i64 3
  %59 = load i8, ptr %58, align 1, !tbaa !2
  %60 = zext i8 %59 to i32
  %61 = getelementptr inbounds i8, ptr %11, i64 3
  %62 = load i8, ptr %61, align 1, !tbaa !2
  %63 = zext i8 %62 to i32
  %64 = sub nsw i32 %60, %63
  %65 = getelementptr inbounds i8, ptr %10, i64 7
  %66 = load i8, ptr %65, align 1, !tbaa !2
  %67 = zext i8 %66 to i32
  %68 = getelementptr inbounds i8, ptr %11, i64 7
  %69 = load i8, ptr %68, align 1, !tbaa !2
  %70 = zext i8 %69 to i32
  %71 = sub nsw i32 %67, %70
  %72 = shl nsw i32 %71, 16
  %73 = add nsw i32 %72, %64
  %74 = add nsw i32 %41, %25
  %75 = sub nsw i32 %25, %41
  %76 = add nsw i32 %73, %57
  %77 = sub nsw i32 %57, %73
  %78 = add nsw i32 %76, %74
  %79 = getelementptr inbounds [4 x [4 x i32]], ptr %5, i64 0, i64 %9, i64 0
  store i32 %78, ptr %79, align 16, !tbaa !5
  %80 = sub nsw i32 %74, %76
  %81 = getelementptr inbounds [4 x [4 x i32]], ptr %5, i64 0, i64 %9, i64 2
  store i32 %80, ptr %81, align 8, !tbaa !5
  %82 = add nsw i32 %77, %75
  %83 = getelementptr inbounds [4 x [4 x i32]], ptr %5, i64 0, i64 %9, i64 1
  store i32 %82, ptr %83, align 4, !tbaa !5
  %84 = sub nsw i32 %75, %77
  %85 = getelementptr inbounds [4 x [4 x i32]], ptr %5, i64 0, i64 %9, i64 3
  store i32 %84, ptr %85, align 4, !tbaa !5
  %86 = add nuw nsw i64 %9, 1
  %87 = getelementptr inbounds i8, ptr %10, i64 %6
  %88 = getelementptr inbounds i8, ptr %11, i64 %7
  %89 = icmp eq i64 %86, 4
  br i1 %89, label %90, label %8

; <label>:91:                                     ; preds = %8
  br label %97

; <label>:92:                                     ; preds = %97
  %92 = phi i32 [ %139, %97 ]
  %93 = and i32 %92, 65535
  %94 = lshr i32 %92, 16
  %95 = add nuw nsw i32 %93, %94
  %96 = lshr i32 %95, 1
  ret i32 %96

; <label>:98:                                     ; preds = %97, %90
  %98 = phi i64 [ %140, %97 ], [ 0, %90 ]
  %99 = phi i32 [ %139, %97 ], [ 0, %90 ]
  %100 = getelementptr inbounds [4 x [4 x i32]], ptr %5, i64 0, i64 0, i64 %98
  %101 = load i32, ptr %100, align 4, !tbaa !5
  %102 = getelementptr inbounds [4 x [4 x i32]], ptr %5, i64 0, i64 1, i64 %98
  %103 = load i32, ptr %102, align 4, !tbaa !5
  %104 = add i32 %103, %101
  %105 = sub i32 %101, %103
  %106 = getelementptr inbounds [4 x [4 x i32]], ptr %5, i64 0, i64 2, i64 %98
  %107 = load i32, ptr %106, align 4, !tbaa !5
  %108 = getelementptr inbounds [4 x [4 x i32]], ptr %5, i64 0, i64 3, i64 %98
  %109 = load i32, ptr %108, align 4, !tbaa !5
  %110 = add i32 %109, %107
  %111 = sub i32 %107, %109
  %112 = add nsw i32 %110, %104
  %113 = sub nsw i32 %104, %110
  %114 = add nsw i32 %111, %105
  %115 = sub nsw i32 %105, %111
  %116 = lshr i32 %112, 15
  %117 = and i32 %116, 65537
  %118 = mul nuw i32 %117, 65535
  %119 = add i32 %118, %112
  %120 = xor i32 %119, %118
  %121 = lshr i32 %114, 15
  %122 = and i32 %121, 65537
  %123 = mul nuw i32 %122, 65535
  %124 = add i32 %123, %114
  %125 = xor i32 %124, %123
  %126 = lshr i32 %113, 15
  %127 = and i32 %126, 65537
  %128 = mul nuw i32 %127, 65535
  %129 = add i32 %128, %113
  %130 = xor i32 %129, %128
  %131 = lshr i32 %115, 15
  %132 = and i32 %131, 65537
  %133 = mul nuw i32 %132, 65535
  %134 = add i32 %133, %115
  %135 = xor i32 %134, %133
  %136 = add i32 %125, %99
  %137 = add i32 %136, %120
  %138 = add i32 %137, %130
  %139 = add i32 %138, %135
  %140 = add nuw nsw i64 %98, 1
  %141 = icmp eq i64 %140, 4
  br i1 %141, label %91, label %97
}

attributes #0 = { nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+aes,+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1}

!0 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang fc51344260bbdfd037a9f4c243f3a428e17b33c1) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm d44dfbe62519e91bb24fcbe44a4e9c83e5060b54)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !8, i64 0}
!6 = !{!"array@_ZTSA4_A4_j", !7, i64 0}
!7 = !{!"array@_ZTSA4_j", !8, i64 0}
!8 = !{!"int", !3, i64 0}
; end INTEL_FEATURE_SW_ADVANCED
