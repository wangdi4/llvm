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
define hidden i32 @x264_pixel_satd_8x4(i8* nocapture readonly, i32, i8* nocapture readonly, i32) #0 {
  %5 = alloca [4 x [4 x i32]], align 16
  %6 = bitcast [4 x [4 x i32]]* %5 to i8*
  %7 = sext i32 %1 to i64
  %8 = sext i32 %3 to i64
  br label %9

; <label>:9:                                      ; preds = %9, %4
  %10 = phi i64 [ 0, %4 ], [ %87, %9 ]
  %11 = phi i8* [ %0, %4 ], [ %88, %9 ]
  %12 = phi i8* [ %2, %4 ], [ %89, %9 ]
  %13 = load i8, i8* %11, align 1, !tbaa !2
  %14 = zext i8 %13 to i32
  %15 = load i8, i8* %12, align 1, !tbaa !2
  %16 = zext i8 %15 to i32
  %17 = sub nsw i32 %14, %16
  %18 = getelementptr inbounds i8, i8* %11, i64 4
  %19 = load i8, i8* %18, align 1, !tbaa !2
  %20 = zext i8 %19 to i32
  %21 = getelementptr inbounds i8, i8* %12, i64 4
  %22 = load i8, i8* %21, align 1, !tbaa !2
  %23 = zext i8 %22 to i32
  %24 = sub nsw i32 %20, %23
  %25 = shl nsw i32 %24, 16
  %26 = add nsw i32 %25, %17
  %27 = getelementptr inbounds i8, i8* %11, i64 1
  %28 = load i8, i8* %27, align 1, !tbaa !2
  %29 = zext i8 %28 to i32
  %30 = getelementptr inbounds i8, i8* %12, i64 1
  %31 = load i8, i8* %30, align 1, !tbaa !2
  %32 = zext i8 %31 to i32
  %33 = sub nsw i32 %29, %32
  %34 = getelementptr inbounds i8, i8* %11, i64 5
  %35 = load i8, i8* %34, align 1, !tbaa !2
  %36 = zext i8 %35 to i32
  %37 = getelementptr inbounds i8, i8* %12, i64 5
  %38 = load i8, i8* %37, align 1, !tbaa !2
  %39 = zext i8 %38 to i32
  %40 = sub nsw i32 %36, %39
  %41 = shl nsw i32 %40, 16
  %42 = add nsw i32 %41, %33
  %43 = getelementptr inbounds i8, i8* %11, i64 2
  %44 = load i8, i8* %43, align 1, !tbaa !2
  %45 = zext i8 %44 to i32
  %46 = getelementptr inbounds i8, i8* %12, i64 2
  %47 = load i8, i8* %46, align 1, !tbaa !2
  %48 = zext i8 %47 to i32
  %49 = sub nsw i32 %45, %48
  %50 = getelementptr inbounds i8, i8* %11, i64 6
  %51 = load i8, i8* %50, align 1, !tbaa !2
  %52 = zext i8 %51 to i32
  %53 = getelementptr inbounds i8, i8* %12, i64 6
  %54 = load i8, i8* %53, align 1, !tbaa !2
  %55 = zext i8 %54 to i32
  %56 = sub nsw i32 %52, %55
  %57 = shl nsw i32 %56, 16
  %58 = add nsw i32 %57, %49
  %59 = getelementptr inbounds i8, i8* %11, i64 3
  %60 = load i8, i8* %59, align 1, !tbaa !2
  %61 = zext i8 %60 to i32
  %62 = getelementptr inbounds i8, i8* %12, i64 3
  %63 = load i8, i8* %62, align 1, !tbaa !2
  %64 = zext i8 %63 to i32
  %65 = sub nsw i32 %61, %64
  %66 = getelementptr inbounds i8, i8* %11, i64 7
  %67 = load i8, i8* %66, align 1, !tbaa !2
  %68 = zext i8 %67 to i32
  %69 = getelementptr inbounds i8, i8* %12, i64 7
  %70 = load i8, i8* %69, align 1, !tbaa !2
  %71 = zext i8 %70 to i32
  %72 = sub nsw i32 %68, %71
  %73 = shl nsw i32 %72, 16
  %74 = add nsw i32 %73, %65
  %75 = add nsw i32 %42, %26
  %76 = sub nsw i32 %26, %42
  %77 = add nsw i32 %74, %58
  %78 = sub nsw i32 %58, %74
  %79 = add nsw i32 %77, %75
  %80 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %10, i64 0
  store i32 %79, i32* %80, align 16, !tbaa !5
  %81 = sub nsw i32 %75, %77
  %82 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %10, i64 2
  store i32 %81, i32* %82, align 8, !tbaa !5
  %83 = add nsw i32 %78, %76
  %84 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %10, i64 1
  store i32 %83, i32* %84, align 4, !tbaa !5
  %85 = sub nsw i32 %76, %78
  %86 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 %10, i64 3
  store i32 %85, i32* %86, align 4, !tbaa !5
  %87 = add nuw nsw i64 %10, 1
  %88 = getelementptr inbounds i8, i8* %11, i64 %7
  %89 = getelementptr inbounds i8, i8* %12, i64 %8
  %90 = icmp eq i64 %87, 4
  br i1 %90, label %91, label %9

; <label>:91:                                     ; preds = %9
  br label %98

; <label>:92:                                     ; preds = %98
  %93 = phi i32 [ %140, %98 ]
  %94 = and i32 %93, 65535
  %95 = lshr i32 %93, 16
  %96 = add nuw nsw i32 %94, %95
  %97 = lshr i32 %96, 1
  ret i32 %97

; <label>:98:                                     ; preds = %98, %91
  %99 = phi i64 [ %141, %98 ], [ 0, %91 ]
  %100 = phi i32 [ %140, %98 ], [ 0, %91 ]
  %101 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 %99
  %102 = load i32, i32* %101, align 4, !tbaa !5
  %103 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 %99
  %104 = load i32, i32* %103, align 4, !tbaa !5
  %105 = add i32 %104, %102
  %106 = sub i32 %102, %104
  %107 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 2, i64 %99
  %108 = load i32, i32* %107, align 4, !tbaa !5
  %109 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 3, i64 %99
  %110 = load i32, i32* %109, align 4, !tbaa !5
  %111 = add i32 %110, %108
  %112 = sub i32 %108, %110
  %113 = add nsw i32 %111, %105
  %114 = sub nsw i32 %105, %111
  %115 = add nsw i32 %112, %106
  %116 = sub nsw i32 %106, %112
  %117 = lshr i32 %113, 15
  %118 = and i32 %117, 65537
  %119 = mul nuw i32 %118, 65535
  %120 = add i32 %119, %113
  %121 = xor i32 %120, %119
  %122 = lshr i32 %115, 15
  %123 = and i32 %122, 65537
  %124 = mul nuw i32 %123, 65535
  %125 = add i32 %124, %115
  %126 = xor i32 %125, %124
  %127 = lshr i32 %114, 15
  %128 = and i32 %127, 65537
  %129 = mul nuw i32 %128, 65535
  %130 = add i32 %129, %114
  %131 = xor i32 %130, %129
  %132 = lshr i32 %116, 15
  %133 = and i32 %132, 65537
  %134 = mul nuw i32 %133, 65535
  %135 = add i32 %134, %116
  %136 = xor i32 %135, %134
  %137 = add i32 %126, %100
  %138 = add i32 %137, %121
  %139 = add i32 %138, %131
  %140 = add i32 %139, %136
  %141 = add nuw nsw i64 %99, 1
  %142 = icmp eq i64 %141, 4
  br i1 %142, label %92, label %98
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
