;
;  Subscript ref:   A[ (1+N)*i1 + N*i2 + i3 + N + 1 ]
;  In BanerjeeMIV test, when inputDV is (= = *)
;  For each level, when DV is = and coeffs are the same, we can ignore the coeffs
;   DO i3 = 0, N
;      = (%0)[(1 + sext.i32.i64(%1)) * i1 + sext.i32.i64(%1) * i2 + i3 + sext.i32.i64(%1) + 1]
;      (%0)[(1 + sext.i32.i64(%1)) * i1 + sext.i32.i64(%1) * i2 + i3 + sext.i32.i64(%1) + 1] =
;    END LOOP
;
; REQUIRES: asserts
; RUN: opt -tbaa -scoped-noalias -hir-ssa-deconstruction  -analyze  -hir-temp-cleanup -hir-vec-dir-insert  -S -debug-only=parvec-analysis < %s 2>&1 | FileCheck %s
; RUN: opt -tbaa -scoped-noalias -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert"  -analyze  -S -debug-only=parvec-analysis < %s 2>&1 | FileCheck %s
;
; CHECK-DAG: (%0)[(1 + sext.i32.i64(%1)) * i1 + sext.i32.i64(%1) * i2 + i3 + sext.i32.i64(%1) + 1] --> (%0)[(1 + sext.i32.i64(%1)) * i1 + sext.i32.i64(%1) * i2 + i3 + sext.i32.i64(%1) + 1] FLOW (* * *)
; CHECK:    (= = =)
; CHECK-SAME:    DV improved by RefineDD
;
;Module Before HIR; ModuleID = 'banerjeeMIV.c'
source_filename = "banerjeeMIV.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define hidden fastcc void @dgefa(float* nocapture, i32, i32, i16* nocapture, i32* nocapture) unnamed_addr #0 {
  store i32 0, i32* %4, align 4, !tbaa !4
  %6 = add i32 %2, -1
  %7 = icmp sgt i32 %2, 0
  %8 = icmp sgt i32 %6, 0
  %9 = and i1 %7, %8
  br i1 %9, label %12, label %10

; <label>:10:                                     ; preds = %5
  %11 = sext i32 %6 to i64
  br label %125

; <label>:12:                                     ; preds = %5
  %13 = sext i32 %1 to i64
  %14 = sext i32 %2 to i64
  %15 = zext i32 %2 to i64
  %16 = sext i32 %6 to i64
  %17 = sext i32 %1 to i64
  br label %18

; <label>:18:                                     ; preds = %121, %12
  %19 = phi i64 [ 0, %12 ], [ %21, %121 ]
  %20 = phi i64 [ 1, %12 ], [ %122, %121 ]
  %21 = add nuw nsw i64 %19, 1
  %22 = sub nsw i64 %14, %19
  %23 = mul nsw i64 %19, %17
  %24 = trunc i64 %19 to i32
  %25 = sext i32 %24 to i64
  %26 = add nsw i64 %23, %25
  %27 = getelementptr inbounds float, float* %0, i64 %26
  %28 = icmp slt i64 %22, 1
  br i1 %28, label %50, label %29

; <label>:29:                                     ; preds = %18
  %30 = trunc i64 %22 to i32
  %31 = icmp eq i32 %30, 1
  br i1 %31, label %50, label %32

; <label>:32:                                     ; preds = %29
  %33 = load float, float* %27, align 4, !tbaa !8
  %34 = tail call fast float @llvm.fabs.f32(float %33) #2
  br label %35

; <label>:35:                                     ; preds = %35, %32
  %36 = phi i64 [ 1, %32 ], [ %46, %35 ]
  %37 = phi i32 [ 0, %32 ], [ %45, %35 ]
  %38 = phi float [ %34, %32 ], [ %43, %35 ]
  %39 = getelementptr inbounds float, float* %27, i64 %36
  %40 = load float, float* %39, align 4, !tbaa !8
  %41 = tail call fast float @llvm.fabs.f32(float %40) #2
  %42 = fcmp fast ogt float %41, %38
  %43 = select i1 %42, float %41, float %38
  %44 = trunc i64 %36 to i32
  %45 = select i1 %42, i32 %44, i32 %37
  %46 = add nuw nsw i64 %36, 1
  %47 = icmp eq i64 %46, %22
  br i1 %47, label %48, label %35

; <label>:48:                                     ; preds = %35
  %49 = phi i32 [ %45, %35 ]
  br label %50

; <label>:50:                                     ; preds = %48, %29, %18
  %51 = phi i32 [ -1, %18 ], [ 0, %29 ], [ %49, %48 ]
  %52 = add nsw i32 %51, %24
  %53 = trunc i32 %52 to i16
  %54 = getelementptr inbounds i16, i16* %3, i64 %19
  store i16 %53, i16* %54, align 2, !tbaa !10
  %55 = mul i32 %24, %1
  %56 = add nsw i32 %52, %55
  %57 = sext i32 %56 to i64
  %58 = getelementptr inbounds float, float* %0, i64 %57
  %59 = load float, float* %58, align 4, !tbaa !8
  %60 = fcmp fast une float %59, 0.000000e+00
  br i1 %60, label %61, label %119

; <label>:61:                                     ; preds = %50
  %62 = icmp ne i32 %51, 0
  br i1 %62, label %63, label %67

; <label>:63:                                     ; preds = %61
  %64 = bitcast float* %27 to i32*
  %65 = load i32, i32* %64, align 4, !tbaa !8
  %66 = bitcast float* %58 to i32*
  store i32 %65, i32* %66, align 4, !tbaa !8
  store float %59, float* %27, align 4, !tbaa !8
  br label %67

; <label>:67:                                     ; preds = %63, %61
  %68 = load float, float* %27, align 4, !tbaa !8
  %69 = fdiv fast float -1.000000e+00, %68
  %70 = sub nsw i64 %14, %21
  %71 = add nsw i64 %26, 1
  %72 = getelementptr inbounds float, float* %0, i64 %71
  %73 = icmp sgt i64 %70, 0
  br i1 %73, label %74, label %83

; <label>:74:                                     ; preds = %67
  br label %75

; <label>:75:                                     ; preds = %75, %74
  %76 = phi i64 [ %80, %75 ], [ 0, %74 ]
  %77 = getelementptr inbounds float, float* %72, i64 %76
  %78 = load float, float* %77, align 4, !tbaa !8
  %79 = fmul fast float %78, %69
  store float %79, float* %77, align 4, !tbaa !8
  %80 = add nuw nsw i64 %76, 1
  %81 = icmp eq i64 %80, %70
  br i1 %81, label %82, label %75

; <label>:82:                                     ; preds = %75
  br label %83

; <label>:83:                                     ; preds = %82, %67
  %84 = icmp slt i64 %21, %14
  br i1 %84, label %85, label %121

; <label>:85:                                     ; preds = %83
  %86 = sext i32 %52 to i64
  br label %87

; <label>:87:                                     ; preds = %116, %85
  %88 = phi i64 [ %20, %85 ], [ %117, %116 ]
  %89 = mul nsw i64 %88, %13
  %90 = add nsw i64 %89, %86
  %91 = getelementptr inbounds float, float* %0, i64 %90
  %92 = load float, float* %91, align 4, !tbaa !8
  %93 = add nsw i64 %89, %19
  br i1 %62, label %94, label %99

; <label>:94:                                     ; preds = %87
  %95 = getelementptr inbounds float, float* %0, i64 %93
  %96 = bitcast float* %95 to i32*
  %97 = load i32, i32* %96, align 4, !tbaa !8
  %98 = bitcast float* %91 to i32*
  store i32 %97, i32* %98, align 4, !tbaa !8
  store float %92, float* %95, align 4, !tbaa !8
  br label %99

; <label>:99:                                     ; preds = %94, %87
  %100 = add nsw i64 %93, 1
  %101 = getelementptr inbounds float, float* %0, i64 %100
  %102 = fcmp fast une float %92, 0.000000e+00
  %103 = and i1 %73, %102
  br i1 %103, label %104, label %116

; <label>:104:                                    ; preds = %99
  br label %105

; <label>:105:                                    ; preds = %105, %104
  %106 = phi i64 [ %113, %105 ], [ 0, %104 ]
  %107 = getelementptr inbounds float, float* %101, i64 %106
  %108 = load float, float* %107, align 4, !tbaa !8, !alias.scope !12, !noalias !15
  %109 = getelementptr inbounds float, float* %72, i64 %106
  %110 = load float, float* %109, align 4, !tbaa !8, !alias.scope !15, !noalias !12
  %111 = fmul fast float %110, %92
  %112 = fadd fast float %111, %108
  store float %112, float* %107, align 4, !tbaa !8, !alias.scope !12, !noalias !15
  %113 = add nuw nsw i64 %106, 1
  %114 = icmp eq i64 %113, %70
  br i1 %114, label %115, label %105

; <label>:115:                                    ; preds = %105
  br label %116

; <label>:116:                                    ; preds = %115, %99
  %117 = add nuw nsw i64 %88, 1
  %118 = icmp eq i64 %117, %15
  br i1 %118, label %120, label %87

; <label>:119:                                    ; preds = %50
  store i32 %24, i32* %4, align 4, !tbaa !4
  br label %121

; <label>:120:                                    ; preds = %116
  br label %121

; <label>:121:                                    ; preds = %120, %119, %83
  %122 = add nuw nsw i64 %20, 1
  %123 = icmp eq i64 %21, %16
  br i1 %123, label %124, label %18

; <label>:124:                                    ; preds = %121
  br label %125

; <label>:125:                                    ; preds = %124, %10
  %126 = phi i64 [ %11, %10 ], [ %16, %124 ]
  %127 = trunc i32 %6 to i16
  %128 = getelementptr inbounds i16, i16* %3, i64 %126
  store i16 %127, i16* %128, align 2, !tbaa !10
  %129 = mul nsw i32 %6, %1
  %130 = add nsw i32 %129, %6
  %131 = sext i32 %130 to i64
  %132 = getelementptr inbounds float, float* %0, i64 %131
  %133 = load float, float* %132, align 4, !tbaa !8
  %134 = fcmp fast oeq float %133, 0.000000e+00
  br i1 %134, label %135, label %136

; <label>:135:                                    ; preds = %125
  store i32 %6, i32* %4, align 4, !tbaa !4
  br label %136

; <label>:136:                                    ; preds = %135, %125
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float @llvm.fabs.f32(float) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{!"clang version 9.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 476b2b88bcfe2124908c011f2956ec6280237af7) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 5ab9cd99cb65fa61007276b14e0d98c185ee62a0)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 0}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"float", !6, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"short", !6, i64 0}
!12 = !{!13}
!13 = distinct !{!13, !14, !"daxpy: %dy"}
!14 = distinct !{!14, !"daxpy"}
!15 = !{!16}
!16 = distinct !{!16, !14, !"daxpy: %dx"}
