; RUN: opt -passes=slp-vectorizer -enable-intel-advanced-opts -slp-multinode -mtriple=x86_64 -mcpu=skylake-avx512 -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1
; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind readonly uwtable
define internal i32 @x264_pixel_satd_8x4(i8* nocapture readonly, i32, i8* nocapture readonly, i32) #8 {
  %5 = alloca [4 x [4 x i32]], align 16
  %6 = bitcast [4 x [4 x i32]]* %5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 64, i8* nonnull %6) #17
  %7 = sext i32 %1 to i64
  %8 = sext i32 %3 to i64
  %9 = load i8, i8* %0, align 1, !tbaa !21
  %10 = zext i8 %9 to i32
  %11 = load i8, i8* %2, align 1, !tbaa !21
  %12 = zext i8 %11 to i32
  %13 = sub nsw i32 %10, %12
  %14 = getelementptr inbounds i8, i8* %0, i64 4
  %15 = load i8, i8* %14, align 1, !tbaa !21
  %16 = zext i8 %15 to i32
  %17 = getelementptr inbounds i8, i8* %2, i64 4
  %18 = load i8, i8* %17, align 1, !tbaa !21
  %19 = zext i8 %18 to i32
  %20 = sub nsw i32 %16, %19
  %21 = shl nsw i32 %20, 16
  %22 = add nsw i32 %21, %13
  %23 = getelementptr inbounds i8, i8* %0, i64 1
  %24 = load i8, i8* %23, align 1, !tbaa !21
  %25 = zext i8 %24 to i32
  %26 = getelementptr inbounds i8, i8* %2, i64 1
  %27 = load i8, i8* %26, align 1, !tbaa !21
  %28 = zext i8 %27 to i32
  %29 = sub nsw i32 %25, %28
  %30 = getelementptr inbounds i8, i8* %0, i64 5
  %31 = load i8, i8* %30, align 1, !tbaa !21
  %32 = zext i8 %31 to i32
  %33 = getelementptr inbounds i8, i8* %2, i64 5
  %34 = load i8, i8* %33, align 1, !tbaa !21
  %35 = zext i8 %34 to i32
  %36 = sub nsw i32 %32, %35
  %37 = shl nsw i32 %36, 16
  %38 = add nsw i32 %37, %29
  %39 = getelementptr inbounds i8, i8* %0, i64 2
  %40 = load i8, i8* %39, align 1, !tbaa !21
  %41 = zext i8 %40 to i32
  %42 = getelementptr inbounds i8, i8* %2, i64 2
  %43 = load i8, i8* %42, align 1, !tbaa !21
  %44 = zext i8 %43 to i32
  %45 = sub nsw i32 %41, %44
  %46 = getelementptr inbounds i8, i8* %0, i64 6
  %47 = load i8, i8* %46, align 1, !tbaa !21
  %48 = zext i8 %47 to i32
  %49 = getelementptr inbounds i8, i8* %2, i64 6
  %50 = load i8, i8* %49, align 1, !tbaa !21
  %51 = zext i8 %50 to i32
  %52 = sub nsw i32 %48, %51
  %53 = shl nsw i32 %52, 16
  %54 = add nsw i32 %53, %45
  %55 = getelementptr inbounds i8, i8* %0, i64 3
  %56 = load i8, i8* %55, align 1, !tbaa !21
  %57 = zext i8 %56 to i32
  %58 = getelementptr inbounds i8, i8* %2, i64 3
  %59 = load i8, i8* %58, align 1, !tbaa !21
  %60 = zext i8 %59 to i32
  %61 = sub nsw i32 %57, %60
  %62 = getelementptr inbounds i8, i8* %0, i64 7
  %63 = load i8, i8* %62, align 1, !tbaa !21
  %64 = zext i8 %63 to i32
  %65 = getelementptr inbounds i8, i8* %2, i64 7
  %66 = load i8, i8* %65, align 1, !tbaa !21
  %67 = zext i8 %66 to i32
  %68 = sub nsw i32 %64, %67
  %69 = shl nsw i32 %68, 16
  %70 = add nsw i32 %69, %61
  %71 = add nsw i32 %38, %22
  %72 = sub nsw i32 %22, %38
  %73 = add nsw i32 %70, %54
  %74 = sub nsw i32 %54, %70
  %75 = add nsw i32 %73, %71
  %76 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 0
  store i32 %75, i32* %76, align 16, !tbaa !480
  %77 = sub nsw i32 %71, %73
  %78 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 2
  store i32 %77, i32* %78, align 8, !tbaa !480
  %79 = add nsw i32 %74, %72
  %80 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 1
  store i32 %79, i32* %80, align 4, !tbaa !480
  %81 = sub nsw i32 %72, %74
  %82 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 0, i64 3
  store i32 %81, i32* %82, align 4, !tbaa !480
  %83 = getelementptr inbounds i8, i8* %0, i64 %7
  %84 = getelementptr inbounds i8, i8* %2, i64 %8
  %85 = load i8, i8* %83, align 1, !tbaa !21
  %86 = zext i8 %85 to i32
  %87 = load i8, i8* %84, align 1, !tbaa !21
  %88 = zext i8 %87 to i32
  %89 = sub nsw i32 %86, %88
  %90 = getelementptr inbounds i8, i8* %83, i64 4
  %91 = load i8, i8* %90, align 1, !tbaa !21
  %92 = zext i8 %91 to i32
  %93 = getelementptr inbounds i8, i8* %84, i64 4
  %94 = load i8, i8* %93, align 1, !tbaa !21
  %95 = zext i8 %94 to i32
  %96 = sub nsw i32 %92, %95
  %97 = shl nsw i32 %96, 16
  %98 = add nsw i32 %97, %89
  %99 = getelementptr inbounds i8, i8* %83, i64 1
  %100 = load i8, i8* %99, align 1, !tbaa !21
  %101 = zext i8 %100 to i32
  %102 = getelementptr inbounds i8, i8* %84, i64 1
  %103 = load i8, i8* %102, align 1, !tbaa !21
  %104 = zext i8 %103 to i32
  %105 = sub nsw i32 %101, %104
  %106 = getelementptr inbounds i8, i8* %83, i64 5
  %107 = load i8, i8* %106, align 1, !tbaa !21
  %108 = zext i8 %107 to i32
  %109 = getelementptr inbounds i8, i8* %84, i64 5
  %110 = load i8, i8* %109, align 1, !tbaa !21
  %111 = zext i8 %110 to i32
  %112 = sub nsw i32 %108, %111
  %113 = shl nsw i32 %112, 16
  %114 = add nsw i32 %113, %105
  %115 = getelementptr inbounds i8, i8* %83, i64 2
  %116 = load i8, i8* %115, align 1, !tbaa !21
  %117 = zext i8 %116 to i32
  %118 = getelementptr inbounds i8, i8* %84, i64 2
  %119 = load i8, i8* %118, align 1, !tbaa !21
  %120 = zext i8 %119 to i32
  %121 = sub nsw i32 %117, %120
  %122 = getelementptr inbounds i8, i8* %83, i64 6
  %123 = load i8, i8* %122, align 1, !tbaa !21
  %124 = zext i8 %123 to i32
  %125 = getelementptr inbounds i8, i8* %84, i64 6
  %126 = load i8, i8* %125, align 1, !tbaa !21
  %127 = zext i8 %126 to i32
  %128 = sub nsw i32 %124, %127
  %129 = shl nsw i32 %128, 16
  %130 = add nsw i32 %129, %121
  %131 = getelementptr inbounds i8, i8* %83, i64 3
  %132 = load i8, i8* %131, align 1, !tbaa !21
  %133 = zext i8 %132 to i32
  %134 = getelementptr inbounds i8, i8* %84, i64 3
  %135 = load i8, i8* %134, align 1, !tbaa !21
  %136 = zext i8 %135 to i32
  %137 = sub nsw i32 %133, %136
  %138 = getelementptr inbounds i8, i8* %83, i64 7
  %139 = load i8, i8* %138, align 1, !tbaa !21
  %140 = zext i8 %139 to i32
  %141 = getelementptr inbounds i8, i8* %84, i64 7
  %142 = load i8, i8* %141, align 1, !tbaa !21
  %143 = zext i8 %142 to i32
  %144 = sub nsw i32 %140, %143
  %145 = shl nsw i32 %144, 16
  %146 = add nsw i32 %145, %137
  %147 = add nsw i32 %114, %98
  %148 = sub nsw i32 %98, %114
  %149 = add nsw i32 %146, %130
  %150 = sub nsw i32 %130, %146
  %151 = add nsw i32 %149, %147
  %152 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 0
  store i32 %151, i32* %152, align 16, !tbaa !480
  %153 = sub nsw i32 %147, %149
  %154 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 2
  store i32 %153, i32* %154, align 8, !tbaa !480
  %155 = add nsw i32 %150, %148
  %156 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 1
  store i32 %155, i32* %156, align 4, !tbaa !480
  %157 = sub nsw i32 %148, %150
  %158 = getelementptr inbounds [4 x [4 x i32]], [4 x [4 x i32]]* %5, i64 0, i64 1, i64 3
  store i32 %157, i32* %158, align 4, !tbaa !480
  call void @llvm.lifetime.end.p0i8(i64 64, i8* nonnull %6) #17
  ret i32 0
}


; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: nounwind readonly
declare <16 x i8> @llvm.masked.gather.v16i8.v16p0i8(<16 x i8*>, i32, <16 x i1>, <16 x i8>) #25

attributes #1 = { argmemonly nounwind }
attributes #8 = { nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #17 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 8.0.0 "}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.unroll.disable"}
!5 = distinct !{!5, !4}
!6 = !{!7, !8, i64 0}
!7 = !{!"array@_ZTSA16_h", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!11, !11, i64 0}
!11 = !{!"pointer@_ZTSPh", !8, i64 0}
!12 = !{!13, !17, i64 48}
!13 = !{!"struct@x264_weight_t", !14, i64 0, !14, i64 16, !16, i64 32, !16, i64 36, !16, i64 40, !17, i64 48}
!14 = !{!"array@_ZTSA8_s", !15, i64 0}
!15 = !{!"short", !8, i64 0}
!16 = !{!"int", !8, i64 0}
!17 = !{!"unspecified pointer", !8, i64 0}
!18 = distinct !{!18, !4}
!19 = distinct !{!19, !4}
!20 = !{!16, !16, i64 0}
!21 = !{!8, !8, i64 0}
!128 = !{!"array@_ZTSA4_j", !16, i64 0}

!480 = !{!481, !16, i64 0}
!481 = !{!"array@_ZTSA4_A4_j", !128, i64 0}


; CHECK: [[L1:%.*]] = load <4 x i8>
; CHECK: [[L2:%.*]] = load <4 x i8>
; CHECK: [[L3:%.*]] = load <4 x i8>
; CHECK: [[L4:%.*]] = load <4 x i8>

; CHECK: [[L5:%.*]] = load <4 x i8>
; CHECK: [[L6:%.*]] = load <4 x i8>
; CHECK: [[L7:%.*]] = load <4 x i8>
; CHECK: [[L8:%.*]] = load <4 x i8>

; CHECK: store <8 x i32>
