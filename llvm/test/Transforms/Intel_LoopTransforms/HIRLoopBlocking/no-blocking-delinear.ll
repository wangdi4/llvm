; REQUIRES:asserts
; RUN: opt -intel-libirc-allowed -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-sinking-for-perfect-loopnest,hir-loop-interchange,print<hir>,hir-loop-blocking,print<hir>" -disable-hir-loop-blocking-loop-depth-check=true -hir-allow-loop-materialization-regions=true -hir-print-only=37 -aa-pipeline="scoped-noalias-aa" -debug-only=hir-loop-blocking -disable-output %s 2>&1 | FileCheck %s

; Verify that the loopnest is not blocked even when loop depth check is disabled.
; Vectorization as-is can be more beneficial to the innermost loop as-is with contiguous group accesses.
; After loop blocking, vectorization may not be possible due to unresolved data dependences.

; Previously, the loop blocking was avoided with loop depth check + delinearization.
; When delinearized, LoopDepth (=2) <= MaxNumDimensions (=2), which was the loop depth check preventing loop blocking.


; CHECK: Function: t_run_test_radix2

;         BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, %25 + -1, 1
; CHECK:        |   + DO i2 = 0, ((-1 + sext.i32.i64(%11)) /u (2 * zext.i32.i64(%78))), 1
;               |   |   %89 = (%7)[4 * %78 * i2 + 2 * %78];
;               |   |   %93 = (%7)[4 * %78 * i2 + 2 * %78 + 1];
;               |   |   %96 = (%7)[4 * %78 * i2];
;               |   |   %97 = %96  -  %89;
;               |   |   (%7)[4 * %78 * i2 + 2 * %78] = %97;
;               |   |   %101 = (%7)[4 * %78 * i2 + 1];
;               |   |   %102 = %101  -  %93;
;               |   |   (%7)[4 * %78 * i2 + 2 * %78 + 1] = %102;
;               |   |   %103 = %96  +  %89;
;               |   |   (%7)[4 * %78 * i2] = %103;
;               |   |   %104 = %101  +  %93;
;               |   |   (%7)[4 * %78 * i2 + 1] = %104;
;               |   + END LOOP
;               |
; CHECK:        |   if (%78 >u 1)
; CHECK-NEXT:   |   {
; CHECK-NEXT:   |      + DO i2 = 0, ((-1 + sext.i32.i64(%11)) /u (2 * zext.i32.i64(%78))), 1
; CHECK-NEXT:   |      |   + DO i3 = 0, %78 + -2, 1
;               |      |   |   %115 = (%76)[2 * i3];
;               |      |   |   %116 = (%76)[2 * i3 + 1];
;               |      |   |   %126 = (%7)[4 * %78 * i2 + 2 * i3 + 2 * %78 + 2];
;               |      |   |   %130 = (%7)[4 * %78 * i2 + 2 * i3 + 2 * %78 + 3];
;               |      |   |   %131 = %126  *  %115;
;               |      |   |   %132 = %130  *  %116;
;               |      |   |   %133 = %131  -  %132;
;               |      |   |   %134 = %130  *  %115;
;               |      |   |   %135 = %126  *  %116;
;               |      |   |   %136 = %134  +  %135;
;               |      |   |   %139 = (%7)[4 * %78 * i2 + 2 * i3 + 2];
;               |      |   |   %140 = %139  -  %133;
;               |      |   |   (%7)[4 * %78 * i2 + 2 * i3 + 2 * %78 + 2] = %140;
;               |      |   |   %144 = (%7)[4 * %78 * i2 + 2 * i3 + 3];
;               |      |   |   %145 = %144  -  %136;
;               |      |   |   (%7)[4 * %78 * i2 + 2 * i3 + 2 * %78 + 3] = %145;
;               |      |   |   %146 = %133  +  %139;
;               |      |   |   (%7)[4 * %78 * i2 + 2 * i3 + 2] = %146;
;               |      |   |   %147 = %144  +  %136;
;               |      |   |   (%7)[4 * %78 * i2 + 2 * i3 + 3] = %147;
;               |      |   + END LOOP
;               |      + END LOOP
;               |
;               |      %76 = &((%76)[2 * zext.i32.i64((-2 + %78)) + 2]);
;               |   }
;               |   %78 = 2 * %78;
;               + END LOOP
;         END REGION

; CHECK: Contains a ref group accessing contiguous memory. May benefit more from vectorization as-is.

; CHECK: Function: t_run_test_radix2
; CHECK-NOT: DO i4

; ModuleID = 'input.ll'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.TCDef = type { [16 x i8], [16 x i8], [16 x i8], [16 x i8], [64 x i8], i16, %struct.version_number, %struct.version_number, %struct.version_number, i32, i32, i32, i32, i16, i64, i64, i64, i64, i16, %struct.snr_result_s, [4 x double], ptr }
%struct.version_number = type { i8, i8, i8, i8 }
%struct.snr_result_s = type { double, double, double, i32, i32, i32, double, double, i32, i32, double, i32 }
%struct.ee_connection_s = type { i32, i32, ptr, i32, i32, %union.pthread_mutex_t, %union.pthread_cond_t, ptr, i32 }
%union.pthread_mutex_t = type { %struct.__pthread_mutex_s }
%struct.__pthread_mutex_s = type { i32, i32, i32, i32, i32, i16, i16, %struct.__pthread_internal_list }
%struct.__pthread_internal_list = type { ptr, ptr }
%union.pthread_cond_t = type { %struct.__pthread_cond_s }
%struct.__pthread_cond_s = type { %union.anon, %union.anon, [2 x i32], [2 x i32], i32, i32, [2 x i32] }
%union.anon = type { i64 }
%struct.intparts_s = type { i8, i16, i32, i32 }

@.str.22 = external hidden unnamed_addr constant [58 x i8], align 1

; Function Attrs: nofree nounwind uwtable
define hidden ptr @t_run_test_radix2(ptr writeonly %0, ptr nocapture readonly %1) #0 {
  %3 = getelementptr inbounds i8, ptr %1, i64 16
  %4 = bitcast ptr %3 to ptr
  %5 = load i32, ptr %4, align 8, !tbaa !6
  %6 = bitcast ptr %1 to ptr
  %7 = load ptr, ptr %6, align 8, !tbaa !15
  %8 = getelementptr inbounds i8, ptr %1, i64 8
  %9 = bitcast ptr %8 to ptr
  %10 = load ptr, ptr %9, align 8, !tbaa !16
  tail call void @llvm.experimental.noalias.scope.decl(metadata !17)
  %11 = sdiv i32 %5, 2
  %12 = icmp eq i32 %11, 1
  br i1 %12, label %165, label %13

13:                                               ; preds = %2
  %14 = icmp sgt i32 %11, 1
  br i1 %14, label %15, label %24

15:                                               ; preds = %13
  br label %16

16:                                               ; preds = %16, %15
  %17 = phi i32 [ %20, %16 ], [ 0, %15 ]
  %18 = phi i32 [ %19, %16 ], [ 1, %15 ]
  %19 = shl nsw i32 %18, 1
  %20 = add nuw nsw i32 %17, 1
  %21 = icmp slt i32 %19, %11
  br i1 %21, label %16, label %22

22:                                               ; preds = %16
  %23 = phi i32 [ %20, %16 ]
  br label %24

24:                                               ; preds = %22, %13
  %25 = phi i32 [ 0, %13 ], [ %23, %22 ]
  %26 = shl nuw i32 1, %25
  %27 = icmp eq i32 %26, %11
  br i1 %27, label %29, label %28

28:                                               ; preds = %24
  tail call void (i32, ptr, ...) @th_exit(i32 1, ptr @.str.22, i32 %11) #4, !noalias !17
  unreachable

29:                                               ; preds = %24
  %30 = icmp eq i32 %5, 0
  br i1 %30, label %187, label %31

31:                                               ; preds = %29
  br i1 %14, label %32, label %69

32:                                               ; preds = %31
  %33 = add nsw i32 %11, -1
  %34 = zext i32 %33 to i64
  br label %35

35:                                               ; preds = %62, %32
  %36 = phi i64 [ 0, %32 ], [ %66, %62 ]
  %37 = phi i32 [ 0, %32 ], [ %65, %62 ]
  %38 = shl nuw i64 %36, 1
  %39 = shl i32 %37, 1
  %40 = sext i32 %37 to i64
  %41 = icmp slt i64 %36, %40
  br i1 %41, label %42, label %55

42:                                               ; preds = %35
  %43 = getelementptr inbounds double, ptr %7, i64 %38
  %44 = load double, ptr %43, align 8, !tbaa !20, !alias.scope !22
  %45 = or i64 %38, 1
  %46 = getelementptr inbounds double, ptr %7, i64 %45
  %47 = load double, ptr %46, align 8, !tbaa !20, !alias.scope !22
  %48 = sext i32 %39 to i64
  %49 = getelementptr inbounds double, ptr %7, i64 %48
  %50 = load double, ptr %49, align 8, !tbaa !20, !alias.scope !22
  store double %50, ptr %43, align 8, !tbaa !20, !alias.scope !22
  %51 = or i32 %39, 1
  %52 = sext i32 %51 to i64
  %53 = getelementptr inbounds double, ptr %7, i64 %52
  %54 = load double, ptr %53, align 8, !tbaa !20, !alias.scope !22
  store double %54, ptr %46, align 8, !tbaa !20, !alias.scope !22
  store double %44, ptr %49, align 8, !tbaa !20, !alias.scope !22
  store double %47, ptr %53, align 8, !tbaa !20, !alias.scope !22
  br label %55

55:                                               ; preds = %42, %35
  br label %56

56:                                               ; preds = %56, %55
  %57 = phi i32 [ %11, %55 ], [ %59, %56 ]
  %58 = phi i32 [ %37, %55 ], [ %61, %56 ]
  %59 = ashr i32 %57, 1
  %60 = icmp slt i32 %58, %59
  %61 = sub nsw i32 %58, %59
  br i1 %60, label %62, label %56

62:                                               ; preds = %56
  %63 = phi i32 [ %58, %56 ]
  %64 = phi i32 [ %59, %56 ]
  %65 = add nsw i32 %63, %64
  %66 = add nuw nsw i64 %36, 1
  %67 = icmp eq i64 %66, %34
  br i1 %67, label %68, label %35

68:                                               ; preds = %62
  br label %69

69:                                               ; preds = %68, %31
  %70 = icmp sgt i32 %25, 0
  br i1 %70, label %71, label %165

71:                                               ; preds = %69
  %72 = icmp sgt i32 %11, 0
  %73 = sext i32 %11 to i64
  br i1 %72, label %74, label %165

74:                                               ; preds = %71
  br label %75

75:                                               ; preds = %159, %74
  %76 = phi ptr [ %160, %159 ], [ %10, %74 ]
  %77 = phi i32 [ %161, %159 ], [ 0, %74 ]
  %78 = phi i32 [ %162, %159 ], [ 1, %74 ]
  %79 = zext i32 %78 to i64
  %80 = shl nuw nsw i64 %79, 1
  br label %81

81:                                               ; preds = %81, %75
  %82 = phi i64 [ 0, %75 ], [ %105, %81 ]
  %83 = trunc i64 %82 to i32
  %84 = shl nsw i32 %83, 1
  %85 = add nuw nsw i32 %78, %83
  %86 = shl nsw i32 %85, 1
  %87 = sext i32 %86 to i64
  %88 = getelementptr inbounds double, ptr %7, i64 %87
  %89 = load double, ptr %88, align 8, !tbaa !20, !alias.scope !17
  %90 = or i32 %86, 1
  %91 = sext i32 %90 to i64
  %92 = getelementptr inbounds double, ptr %7, i64 %91
  %93 = load double, ptr %92, align 8, !tbaa !20, !alias.scope !17
  %94 = sext i32 %84 to i64
  %95 = getelementptr inbounds double, ptr %7, i64 %94
  %96 = load double, ptr %95, align 8, !tbaa !20, !alias.scope !17
  %97 = fsub fast double %96, %89
  store double %97, ptr %88, align 8, !tbaa !20, !alias.scope !17
  %98 = or i32 %84, 1
  %99 = sext i32 %98 to i64
  %100 = getelementptr inbounds double, ptr %7, i64 %99
  %101 = load double, ptr %100, align 8, !tbaa !20, !alias.scope !17
  %102 = fsub fast double %101, %93
  store double %102, ptr %92, align 8, !tbaa !20, !alias.scope !17
  %103 = fadd fast double %96, %89
  store double %103, ptr %95, align 8, !tbaa !20, !alias.scope !17
  %104 = fadd fast double %101, %93
  store double %104, ptr %100, align 8, !tbaa !20, !alias.scope !17
  %105 = add nuw nsw i64 %82, %80
  %106 = icmp slt i64 %105, %73
  br i1 %106, label %81, label %107

107:                                              ; preds = %81
  %108 = icmp ugt i32 %78, 1
  br i1 %108, label %109, label %159

109:                                              ; preds = %107
  %110 = add nsw i32 %78, -2
  br label %111

111:                                              ; preds = %150, %109
  %112 = phi ptr [ %151, %150 ], [ %76, %109 ]
  %113 = phi i32 [ %152, %150 ], [ 1, %109 ]
  %114 = getelementptr inbounds double, ptr %112, i64 1
  %115 = load double, ptr %112, align 8, !tbaa !20, !noalias !17
  %116 = load double, ptr %114, align 8, !tbaa !20, !noalias !17
  br label %117

117:                                              ; preds = %117, %111
  %118 = phi i64 [ 0, %111 ], [ %148, %117 ]
  %119 = trunc i64 %118 to i32
  %120 = add nuw nsw i32 %113, %119
  %121 = shl nsw i32 %120, 1
  %122 = add nuw nsw i32 %120, %78
  %123 = shl nsw i32 %122, 1
  %124 = sext i32 %123 to i64
  %125 = getelementptr inbounds double, ptr %7, i64 %124
  %126 = load double, ptr %125, align 8, !tbaa !20, !alias.scope !17
  %127 = or i32 %123, 1
  %128 = sext i32 %127 to i64
  %129 = getelementptr inbounds double, ptr %7, i64 %128
  %130 = load double, ptr %129, align 8, !tbaa !20, !alias.scope !17
  %131 = fmul fast double %126, %115
  %132 = fmul fast double %130, %116
  %133 = fsub fast double %131, %132
  %134 = fmul fast double %130, %115
  %135 = fmul fast double %126, %116
  %136 = fadd fast double %134, %135
  %137 = sext i32 %121 to i64
  %138 = getelementptr inbounds double, ptr %7, i64 %137
  %139 = load double, ptr %138, align 8, !tbaa !20, !alias.scope !17
  %140 = fsub fast double %139, %133
  store double %140, ptr %125, align 8, !tbaa !20, !alias.scope !17
  %141 = or i32 %121, 1
  %142 = sext i32 %141 to i64
  %143 = getelementptr inbounds double, ptr %7, i64 %142
  %144 = load double, ptr %143, align 8, !tbaa !20, !alias.scope !17
  %145 = fsub fast double %144, %136
  store double %145, ptr %129, align 8, !tbaa !20, !alias.scope !17
  %146 = fadd fast double %133, %139
  store double %146, ptr %138, align 8, !tbaa !20, !alias.scope !17
  %147 = fadd fast double %144, %136
  store double %147, ptr %143, align 8, !tbaa !20, !alias.scope !17
  %148 = add nuw nsw i64 %118, %80
  %149 = icmp slt i64 %148, %73
  br i1 %149, label %117, label %150

150:                                              ; preds = %117
  %151 = getelementptr inbounds double, ptr %112, i64 2
  %152 = add nuw nsw i32 %113, 1
  %153 = icmp eq i32 %152, %78
  br i1 %153, label %154, label %111

154:                                              ; preds = %150
  %155 = zext i32 %110 to i64
  %156 = shl nuw nsw i64 %155, 1
  %157 = getelementptr double, ptr %76, i64 2
  %158 = getelementptr double, ptr %157, i64 %156
  br label %159

159:                                              ; preds = %154, %107
  %160 = phi ptr [ %76, %107 ], [ %158, %154 ]
  %161 = add nuw nsw i32 %77, 1
  %162 = shl nsw i32 %78, 1
  %163 = icmp eq i32 %161, %25
  br i1 %163, label %164, label %75

164:                                              ; preds = %159
  br label %165

165:                                              ; preds = %164, %71, %69, %2
  %166 = icmp eq i32 %5, 0
  br i1 %166, label %187, label %167

167:                                              ; preds = %165
  %168 = sext i32 %5 to i64
  br label %169

169:                                              ; preds = %169, %167
  %170 = phi i64 [ 0, %167 ], [ %181, %169 ]
  %171 = phi double [ 0.000000e+00, %167 ], [ %180, %169 ]
  %172 = phi double [ -1.000000e+100, %167 ], [ %179, %169 ]
  %173 = phi double [ 1.000000e+100, %167 ], [ %177, %169 ]
  %174 = getelementptr inbounds double, ptr %7, i64 %170
  %175 = load double, ptr %174, align 8, !tbaa !20
  %176 = fcmp fast ogt double %173, %175
  %177 = select fast i1 %176, double %175, double %173
  %178 = fcmp fast olt double %172, %175
  %179 = select fast i1 %178, double %175, double %172
  %180 = fadd fast double %175, %171
  %181 = add nuw nsw i64 %170, 1
  %182 = icmp eq i64 %181, %168
  br i1 %182, label %183, label %169, !llvm.loop !25

183:                                              ; preds = %169
  %184 = phi double [ %177, %169 ]
  %185 = phi double [ %179, %169 ]
  %186 = phi double [ %180, %169 ]
  br label %187

187:                                              ; preds = %183, %165, %29
  %188 = phi double [ 1.000000e+100, %165 ], [ 1.000000e+100, %29 ], [ %184, %183 ]
  %189 = phi double [ -1.000000e+100, %165 ], [ -1.000000e+100, %29 ], [ %185, %183 ]
  %190 = phi double [ 0.000000e+00, %165 ], [ 0.000000e+00, %29 ], [ %186, %183 ]
  %191 = uitofp i32 %5 to double
  %192 = fdiv fast double %190, %191
  %193 = getelementptr inbounds i8, ptr %1, i64 56
  %194 = bitcast ptr %193 to ptr
  %195 = tail call fastcc i32 @fp_iaccurate_bits_dp(double %192, ptr nonnull %194) #4
  %196 = getelementptr inbounds i8, ptr %1, i64 68
  %197 = bitcast ptr %196 to ptr
  %198 = load i32, ptr %197, align 4, !tbaa !27
  %199 = icmp ult i32 %195, %198
  %200 = zext i1 %199 to i16
  %201 = getelementptr inbounds %struct.TCDef, ptr %0, i64 0, i32 13, !intel-tbaa !28
  store i16 %200, ptr %201, align 8
  %202 = zext i32 %195 to i64
  %203 = getelementptr inbounds %struct.TCDef, ptr %0, i64 0, i32 14, !intel-tbaa !37
  store i64 %202, ptr %203, align 8, !tbaa !37
  %204 = getelementptr inbounds %struct.TCDef, ptr %0, i64 0, i32 20, i64 0, !intel-tbaa !38
  store double %192, ptr %204, align 8, !tbaa !38
  %205 = getelementptr inbounds %struct.TCDef, ptr %0, i64 0, i32 20, i64 1, !intel-tbaa !38
  store double %188, ptr %205, align 8, !tbaa !38
  %206 = getelementptr inbounds %struct.TCDef, ptr %0, i64 0, i32 20, i64 2, !intel-tbaa !38
  store double %189, ptr %206, align 8, !tbaa !38
  %207 = getelementptr %struct.TCDef, ptr %0, i64 0, i32 0, i64 0
  ret ptr %207
}

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.experimental.noalias.scope.decl(metadata) #1

; Function Attrs: nofree noreturn nounwind uwtable
declare hidden void @th_exit(i32, ptr nocapture readonly, ...) unnamed_addr #2

; Function Attrs: nofree nosync nounwind uwtable
declare hidden fastcc i32 @fp_iaccurate_bits_dp(double, ptr nocapture readonly) unnamed_addr #3

attributes #0 = { nofree nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #2 = { nofree noreturn nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #3 = { nofree nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 1, !"LTOPostLink", i32 1}
!6 = !{!7, !11, i64 16}
!7 = !{!"struct@radix2_params_s", !8, i64 0, !8, i64 8, !11, i64 16, !11, i64 20, !12, i64 24, !13, i64 32, !13, i64 44, !13, i64 56, !11, i64 68, !11, i64 72}
!8 = !{!"pointer@_ZTSPd", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!"int", !9, i64 0}
!12 = !{!"pointer@_ZTSP10intparts_s", !9, i64 0}
!13 = !{!"struct@intparts_s", !9, i64 0, !14, i64 2, !11, i64 4, !11, i64 8}
!14 = !{!"short", !9, i64 0}
!15 = !{!7, !8, i64 0}
!16 = !{!7, !8, i64 8}
!17 = !{!18}
!18 = distinct !{!18, !19, !"FFT_transform_internal: %data"}
!19 = distinct !{!19, !"FFT_transform_internal"}
!20 = !{!21, !21, i64 0}
!21 = !{!"double", !9, i64 0}
!22 = !{!23, !18}
!23 = distinct !{!23, !24, !"FFT_bitreverse: %data"}
!24 = distinct !{!24, !"FFT_bitreverse"}
!25 = distinct !{!25, !26}
!26 = !{!"llvm.loop.mustprogress"}
!27 = !{!7, !11, i64 68}
!28 = !{!29, !14, i64 160}
!29 = !{!"struct@TCDef", !30, i64 0, !30, i64 16, !30, i64 32, !30, i64 48, !31, i64 64, !14, i64 128, !32, i64 130, !32, i64 134, !32, i64 138, !11, i64 144, !11, i64 148, !11, i64 152, !11, i64 156, !14, i64 160, !33, i64 168, !33, i64 176, !33, i64 184, !33, i64 192, !14, i64 200, !34, i64 208, !35, i64 288, !36, i64 320}
!30 = !{!"array@_ZTSA16_c", !9, i64 0}
!31 = !{!"array@_ZTSA64_c", !9, i64 0}
!32 = !{!"struct@", !9, i64 0, !9, i64 1, !9, i64 2, !9, i64 3}
!33 = !{!"long", !9, i64 0}
!34 = !{!"struct@snr_result_s", !21, i64 0, !21, i64 8, !21, i64 16, !11, i64 24, !11, i64 28, !11, i64 32, !21, i64 40, !21, i64 48, !11, i64 56, !11, i64 60, !21, i64 64, !11, i64 72}
!35 = !{!"array@_ZTSA4_d", !21, i64 0}
!36 = !{!"pointer@_ZTSPP15ee_connection_s", !9, i64 0}
!37 = !{!29, !33, i64 168}
!38 = !{!29, !21, i64 288}

