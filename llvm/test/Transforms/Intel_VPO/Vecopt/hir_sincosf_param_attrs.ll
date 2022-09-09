; RUN: opt %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vplan-vec -hir-cg -vector-library=SVML -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = dso_local global float 0.000000e+00, align 4
@b = dso_local global float 0.000000e+00, align 4

; Function Attrs: mustprogress nofree nounwind uwtable
define dso_local void @_Z1cv() local_unnamed_addr #0 {
; CHECK-LABEL: @_Z1cv(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[T24:%.*]] = alloca float, align 4
; CHECK-NEXT:    [[T23:%.*]] = alloca float, align 4
; CHECK-NEXT:    [[T22:%.*]] = alloca <4 x float>, align 16
; CHECK-NEXT:    [[T21:%.*]] = alloca <4 x float>, align 16
; CHECK-NEXT:    [[T20:%.*]] = alloca { <4 x float>, <4 x float> }, align 16
; CHECK-NEXT:    [[T19:%.*]] = alloca float, align 4
; CHECK-NEXT:    [[T11:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[T18:%.*]] = alloca float, align 4
; CHECK-NEXT:    [[T16:%.*]] = alloca float, align 4
; CHECK-NEXT:    [[T14:%.*]] = alloca <4 x float>, align 16
; CHECK-NEXT:    [[T13:%.*]] = alloca <4 x float>, align 16
; CHECK-NEXT:    [[T12:%.*]] = alloca { <4 x float>, <4 x float> }, align 16
; CHECK-NEXT:    [[I1_I32:%.*]] = alloca i32, align 4
; CHECK-NEXT:    br label [[ENTRY_SPLIT:%.*]]
; CHECK:       entry.split:
; CHECK-NEXT:    br i1 true, label [[REGION_0:%.*]], label [[ENTRY_SPLIT_SPLIT:%.*]]
; CHECK:       for.end.split:
; CHECK-NEXT:    br label [[EXIT:%.*]]
; CHECK:       exit:
; CHECK-NEXT:    ret void
; CHECK:       region.0:
; CHECK-NEXT:    store i32 0, i32* [[I1_I32]], align 4
; CHECK-NEXT:    br label [[LOOP_18:%.*]]
; CHECK:       loop.18:
; CHECK-NEXT:    [[TMP0:%.*]] = call svml_cc { <4 x float>, <4 x float> } @__svml_sincosf4(<4 x float> noundef <float 5.000000e-01, float 5.000000e-01, float 5.000000e-01, float 5.000000e-01>) #[[ATTR5:[0-9]+]]
; CHECK-NEXT:    store { <4 x float>, <4 x float> } [[TMP0]], { <4 x float>, <4 x float> }* [[T12]], align 16
; CHECK-NEXT:    [[T12_:%.*]] = load { <4 x float>, <4 x float> }, { <4 x float>, <4 x float> }* [[T12]], align 16
; CHECK-NEXT:    [[SINCOS_SIN7:%.*]] = extractvalue { <4 x float>, <4 x float> } [[T12_]], 0
; CHECK-NEXT:    store <4 x float> [[SINCOS_SIN7]], <4 x float>* [[T13]], align 16
; CHECK-NEXT:    [[T12_8:%.*]] = load { <4 x float>, <4 x float> }, { <4 x float>, <4 x float> }* [[T12]], align 16
; CHECK-NEXT:    [[SINCOS_COS9:%.*]] = extractvalue { <4 x float>, <4 x float> } [[T12_8]], 1
; CHECK-NEXT:    store <4 x float> [[SINCOS_COS9]], <4 x float>* [[T14]], align 16
; CHECK-NEXT:    [[T13_:%.*]] = load <4 x float>, <4 x float>* [[T13]], align 16
; CHECK-NEXT:    [[ELEM39:%.*]] = extractelement <4 x float> [[T13_]], i64 3
; CHECK-NEXT:    store float [[ELEM39]], float* [[T16]], align 4
; CHECK-NEXT:    [[T16_:%.*]] = load float, float* [[T16]], align 4
; CHECK-NEXT:    store float [[T16_]], float* @a, align 4
; CHECK-NEXT:    [[T14_:%.*]] = load <4 x float>, <4 x float>* [[T14]], align 16
; CHECK-NEXT:    [[ELEM310:%.*]] = extractelement <4 x float> [[T14_]], i64 3
; CHECK-NEXT:    store float [[ELEM310]], float* [[T18]], align 4
; CHECK-NEXT:    [[T18_:%.*]] = load float, float* [[T18]], align 4
; CHECK-NEXT:    store float [[T18_]], float* @b, align 4
; CHECK-NEXT:    [[TMP1:%.*]] = load i32, i32* [[I1_I32]], align 4
; CHECK-NEXT:    [[NEXTIVLOOP_18:%.*]] = add nuw nsw i32 [[TMP1]], 4
; CHECK-NEXT:    store i32 [[NEXTIVLOOP_18]], i32* [[I1_I32]], align 4
; CHECK-NEXT:    [[CONDLOOP_18:%.*]] = icmp sle i32 [[NEXTIVLOOP_18]], 99
; CHECK-NEXT:    br i1 [[CONDLOOP_18]], label [[LOOP_18]], label [[AFTERLOOP_18:%.*]], !llvm.loop [[LOOP6:![0-9]+]]
; CHECK:       afterloop.18:
; CHECK-NEXT:    store i32 100, i32* [[T11]], align 4
; CHECK-NEXT:    store i32 100, i32* [[I1_I32]], align 4
; CHECK-NEXT:    br label [[LOOP_37:%.*]]
; CHECK:       loop.39:
; CHECK-NEXT:    store float 5.000000e-01, float* [[T19]], align 4
; CHECK-NEXT:    [[T19_:%.*]] = load float, float* [[T19]], align 4
; CHECK-NEXT:    [[DOTSPLATINSERT:%.*]] = insertelement <4 x float> poison, float [[T19_]], i32 0
; CHECK-NEXT:    [[DOTSPLAT:%.*]] = shufflevector <4 x float> [[DOTSPLATINSERT]], <4 x float> poison, <4 x i32> zeroinitializer
; CHECK-NEXT:    [[TMP2:%.*]] = call svml_cc { <4 x float>, <4 x float> } @__svml_sincosf4(<4 x float> noundef [[DOTSPLAT]]) #[[ATTR5]]
; CHECK-NEXT:    store { <4 x float>, <4 x float> } [[TMP2]], { <4 x float>, <4 x float> }* [[T20]], align 16
; CHECK-NEXT:    [[T20_:%.*]] = load { <4 x float>, <4 x float> }, { <4 x float>, <4 x float> }* [[T20]], align 16
; CHECK-NEXT:    [[SINCOS_SIN410:%.*]] = extractvalue { <4 x float>, <4 x float> } [[T20_]], 0
; CHECK-NEXT:    store <4 x float> [[SINCOS_SIN410]], <4 x float>* [[T21]], align 16
; CHECK-NEXT:    [[T20_11:%.*]] = load { <4 x float>, <4 x float> }, { <4 x float>, <4 x float> }* [[T20]], align 16
; CHECK-NEXT:    [[SINCOS_COS512:%.*]] = extractvalue { <4 x float>, <4 x float> } [[T20_11]], 1
; CHECK-NEXT:    store <4 x float> [[SINCOS_COS512]], <4 x float>* [[T22]], align 16
; CHECK-NEXT:    [[T21_:%.*]] = load <4 x float>, <4 x float>* [[T21]], align 16
; CHECK-NEXT:    [[ELEM13:%.*]] = extractelement <4 x float> [[T21_]], i64 0
; CHECK-NEXT:    store float [[ELEM13]], float* [[T23]], align 4
; CHECK-NEXT:    [[T23_:%.*]] = load float, float* [[T23]], align 4
; CHECK-NEXT:    store float [[T23_]], float* @a, align 4
; CHECK-NEXT:    [[T22_:%.*]] = load <4 x float>, <4 x float>* [[T22]], align 16
; CHECK-NEXT:    [[ELEM614:%.*]] = extractelement <4 x float> [[T22_]], i64 0
; CHECK-NEXT:    store float [[ELEM614]], float* [[T24]], align 4
; CHECK-NEXT:    [[T24_:%.*]] = load float, float* [[T24]], align 4
; CHECK-NEXT:    store float [[T24_]], float* @b, align 4
; CHECK-NEXT:    [[TMP3:%.*]] = load i32, i32* [[I1_I32]], align 4
; CHECK-NEXT:    [[NEXTIVLOOP_37:%.*]] = add nuw nsw i32 [[TMP3]], 1
; CHECK-NEXT:    store i32 [[NEXTIVLOOP_37]], i32* [[I1_I32]], align 4
; CHECK-NEXT:    [[CONDLOOP_37:%.*]] = icmp ne i32 [[TMP3]], 100
; CHECK-NEXT:    br i1 [[CONDLOOP_37]], label [[LOOP_37]], label [[AFTERLOOP_37:%.*]], !llvm.loop [[LOOP11:![0-9]+]]
; CHECK:       afterloop.39:
; CHECK-NEXT:    br label [[FOR_END_SPLIT:%.*]]
;
; CHECK:       declare noundef { <4 x float>, <4 x float> } @__svml_sincosf4(<4 x float> noundef)
; CHECK:       attributes #[[ATTR5]] = { nounwind "imf-precision"="medium" }
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %d.03 = phi i32 [ 0, %entry ], [ %inc, %for.body ], !in.de.ssa !3
  tail call void @sincosf(float noundef 5.000000e-01, float* noundef nonnull @a, float* noundef nonnull @b) #3
  %inc = add nuw nsw i32 %d.03, 1
  %exitcond.not = icmp eq i32 %inc, 101
  %d.03.in = call i32 @llvm.ssa.copy.i32(i32 %inc), !in.de.ssa !3
  br i1 %exitcond.not, label %for.end, label %for.body, !llvm.loop !4

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %exit

exit:
  ret void
}

; Function Attrs: nofree nounwind
declare dso_local void @sincosf(float noundef, float* noundef, float* noundef) local_unnamed_addr #1

; Function Attrs: nocallback nofree nosync nounwind readnone willreturn
declare i32 @llvm.ssa.copy.i32(i32 returned) #2

attributes #0 = { mustprogress nofree nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "imf-precision"="medium" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nocallback nofree nosync nounwind readnone willreturn }
attributes #3 = { nounwind "imf-precision"="medium" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!"d.03.de.ssa"}
!4 = distinct !{!4, !5}
!5 = !{!"llvm.loop.mustprogress"}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3
