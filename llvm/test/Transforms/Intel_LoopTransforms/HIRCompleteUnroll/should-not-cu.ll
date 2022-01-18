; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -print-after=hir-pre-vec-complete-unroll -hir-print-only=40 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" -aa-pipeline="basic-aa" -hir-print-only=40 -disable-output

; Make sure the first i2-loop is not completely unrolled.

; CHECK: Function: zot
; CHECK: DO i2 = 0, 7, 1

;         BEGIN REGION { }
;               + DO i1 = 0, 2 * %arg17, 1   <DO_LOOP>  <MAX_TC_EST = 4294967295>
;               |   @llvm.lifetime.start.p0i8(32,  &((i8*)(%tmp20)[0]));
;               |   @llvm.lifetime.start.p0i8(32,  &((i8*)(%tmp21)[0]));
;               |
;               |   + DO i2 = 0, 7, 1   <DO_LOOP>     // This loop should not unroll
;               |   |   %tmp72 = (%tmp18)[0][i2];
;               |   |   %tmp73 = %tmp72  *  %arg11;
;               |   |   %tmp75 = (%tmp19)[0][i2];
;               |   |   %tmp76 = %tmp75  *  %arg12;
;               |   |   %tmp77 = %tmp76  +  %tmp73;
;               |   |   %tmp78 = %tmp72  *  %arg13;
;               |   |   %tmp79 = %tmp75  *  %arg14;
;               |   |   %tmp80 = %tmp79  +  %tmp78;
;               |   |   (%tmp18)[0][i2] = %tmp77;
;               |   |   (%tmp19)[0][i2] = %tmp80;
;               |   |   %tmp81 = %tmp77  +  %arg15;
;               |   |   %tmp82 = %tmp80  +  %arg16;
;               |   |   %tmp84 = (%tmp81 > %arg5) ? %arg5 : %tmp81;
;               |   |   %tmp85 = @llvm.maxnum.f32(%tmp84,  0.000000e+00);
;               |   |   %tmp87 = (%tmp82 > %arg6) ? %arg6 : %tmp82;
;               |   |   %tmp88 = @llvm.maxnum.f32(%tmp87,  0.000000e+00);
;               |   |   %tmp89 = %tmp85  +  5.000000e-01;
;               |   |   %tmp90 = fptosi.float.i32(%tmp89);
;               |   |   (%tmp20)[0][i2] = %tmp90;
;               |   |   %tmp92 = %tmp88  +  5.000000e-01;
;               |   |   %tmp93 = fptosi.float.i32(%tmp92);
;               |   |   (%tmp21)[0][i2] = %tmp93;
;               |   + END LOOP
;               |
;               |
;               |   + DO i2 = 0, 3, 1   <DO_LOOP>
;               |   |   + DO i3 = 0, 7, 1   <DO_LOOP>
;               |   |   |   %tmp109 = (%tmp21)[0][i3];
;               |   |   |   %tmp112 = (%tmp20)[0][i3];
;               |   |   |   %tmp119 = (%tmp)[0][i2][i3]  +  (%tmp44)[sext.i32.i64((%arg4 * %tmp109)) + sext.i32.i64(%tmp112)][i2];
;               |   |   |   (%tmp)[0][i2][i3] = %tmp119;
;               |   |   + END LOOP
;               |   + END LOOP
;               |
;               |   @llvm.lifetime.end.p0i8(32,  &((i8*)(%tmp21)[0]));
;               |   @llvm.lifetime.end.p0i8(32,  &((i8*)(%tmp20)[0]));
;               + END LOOP
;         END REGION

; ModuleID = 'extract.ll'
source_filename = "radialblur.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: mustprogress nofree nosync nounwind uwtable
define dso_local void @zot(float* nocapture readonly %arg, float* nocapture %arg1, i32 %arg2, i32 %arg3, i32 %arg4, float %arg5, float %arg6, float %arg7, float %arg8, float %arg9, float %arg10, float %arg11, float %arg12, float %arg13, float %arg14, float %arg15, float %arg16, i32 %arg17) local_unnamed_addr #1 {
bb:
  %tmp = alloca [4 x [8 x float]], align 16
  %tmp18 = alloca [8 x float], align 16
  %tmp19 = alloca [8 x float], align 16
  %tmp20 = alloca [8 x i32], align 16
  %tmp21 = alloca [8 x i32], align 16
  %tmp22 = bitcast [4 x [8 x float]]* %tmp to i8*
  call void @llvm.lifetime.start.p0i8(i64 128, i8* nonnull %tmp22) #3
  br label %bb23

bb23:                                             ; preds = %bb32, %bb
  %tmp24 = phi i64 [ 0, %bb ], [ %tmp33, %bb32 ]
  br label %bb35

bb25:                                             ; preds = %bb32
  %tmp26 = bitcast [8 x float]* %tmp18 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %tmp26) #3
  %tmp27 = bitcast [8 x float]* %tmp19 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %tmp27) #3
  %tmp28 = sitofp i32 %arg3 to float
  %tmp29 = fsub fast float %tmp28, %arg16
  %tmp30 = fmul fast float %tmp29, %arg8
  %tmp31 = fmul fast float %tmp29, %arg10
  br label %bb48

bb32:                                             ; preds = %bb35
  %tmp33 = add nuw nsw i64 %tmp24, 1
  %tmp34 = icmp eq i64 %tmp33, 4
  br i1 %tmp34, label %bb25, label %bb23, !llvm.loop !6

bb35:                                             ; preds = %bb35, %bb23
  %tmp36 = phi i64 [ 0, %bb23 ], [ %tmp38, %bb35 ]
  %tmp37 = getelementptr inbounds [4 x [8 x float]], [4 x [8 x float]]* %tmp, i64 0, i64 %tmp24, i64 %tmp36, !intel-tbaa !8
  store float 0.000000e+00, float* %tmp37, align 4, !tbaa !8
  %tmp38 = add nuw nsw i64 %tmp36, 1
  %tmp39 = icmp eq i64 %tmp38, 8
  br i1 %tmp39, label %bb32, label %bb35, !llvm.loop !14

bb40:                                             ; preds = %bb48
  %tmp41 = shl i32 %arg17, 1
  %tmp42 = bitcast [8 x i32]* %tmp20 to i8*
  %tmp43 = bitcast [8 x i32]* %tmp21 to i8*
  %tmp44 = bitcast float* %arg to [4 x float]*
  %tmp45 = icmp slt i32 %tmp41, 0
  br i1 %tmp45, label %bb63, label %bb46

bb46:                                             ; preds = %bb40
  %tmp47 = or i32 %tmp41, 1
  br label %bb67

bb48:                                             ; preds = %bb48, %bb25
  %tmp49 = phi i64 [ 0, %bb25 ], [ %tmp60, %bb48 ]
  %tmp50 = trunc i64 %tmp49 to i32
  %tmp51 = add nsw i32 %tmp50, %arg2
  %tmp52 = sitofp i32 %tmp51 to float
  %tmp53 = fsub fast float %tmp52, %arg15
  %tmp54 = fmul fast float %tmp53, %arg7
  %tmp55 = fadd fast float %tmp54, %tmp30
  %tmp56 = getelementptr inbounds [8 x float], [8 x float]* %tmp18, i64 0, i64 %tmp49, !intel-tbaa !15
  store float %tmp55, float* %tmp56, align 4, !tbaa !15
  %tmp57 = fmul fast float %tmp53, %arg9
  %tmp58 = fadd fast float %tmp57, %tmp31
  %tmp59 = getelementptr inbounds [8 x float], [8 x float]* %tmp19, i64 0, i64 %tmp49, !intel-tbaa !15
  store float %tmp58, float* %tmp59, align 4, !tbaa !15
  %tmp60 = add nuw nsw i64 %tmp49, 1
  %tmp61 = icmp eq i64 %tmp60, 8
  br i1 %tmp61, label %bb40, label %bb48, !llvm.loop !16

bb62:                                             ; preds = %bb100
  br label %bb63

bb63:                                             ; preds = %bb62, %bb40
  %tmp64 = or i32 %tmp41, 1
  %tmp65 = sitofp i32 %tmp64 to float
  %tmp66 = fdiv fast float 1.000000e+00, %tmp65
  br label %bb122

bb67:                                             ; preds = %bb100, %bb46
  %tmp68 = phi i32 [ %tmp101, %bb100 ], [ 0, %bb46 ]
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %tmp42) #3
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %tmp43) #3
  br label %bb69

bb69:                                             ; preds = %bb69, %bb67
  %tmp70 = phi i64 [ 0, %bb67 ], [ %tmp95, %bb69 ]
  %tmp71 = getelementptr inbounds [8 x float], [8 x float]* %tmp18, i64 0, i64 %tmp70, !intel-tbaa !15
  %tmp72 = load float, float* %tmp71, align 4, !tbaa !15
  %tmp73 = fmul fast float %tmp72, %arg11
  %tmp74 = getelementptr inbounds [8 x float], [8 x float]* %tmp19, i64 0, i64 %tmp70, !intel-tbaa !15
  %tmp75 = load float, float* %tmp74, align 4, !tbaa !15
  %tmp76 = fmul fast float %tmp75, %arg12
  %tmp77 = fadd fast float %tmp76, %tmp73
  %tmp78 = fmul fast float %tmp72, %arg13
  %tmp79 = fmul fast float %tmp75, %arg14
  %tmp80 = fadd fast float %tmp79, %tmp78
  store float %tmp77, float* %tmp71, align 4, !tbaa !15
  store float %tmp80, float* %tmp74, align 4, !tbaa !15
  %tmp81 = fadd fast float %tmp77, %arg15
  %tmp82 = fadd fast float %tmp80, %arg16
  %tmp83 = fcmp fast ogt float %tmp81, %arg5
  %tmp84 = select fast i1 %tmp83, float %arg5, float %tmp81
  %tmp85 = tail call fast float @llvm.maxnum.f32(float %tmp84, float 0.000000e+00)
  %tmp86 = fcmp fast ogt float %tmp82, %arg6
  %tmp87 = select fast i1 %tmp86, float %arg6, float %tmp82
  %tmp88 = tail call fast float @llvm.maxnum.f32(float %tmp87, float 0.000000e+00)
  %tmp89 = fadd fast float %tmp85, 5.000000e-01
  %tmp90 = fptosi float %tmp89 to i32
  %tmp91 = getelementptr inbounds [8 x i32], [8 x i32]* %tmp20, i64 0, i64 %tmp70, !intel-tbaa !17
  store i32 %tmp90, i32* %tmp91, align 4, !tbaa !17
  %tmp92 = fadd fast float %tmp88, 5.000000e-01
  %tmp93 = fptosi float %tmp92 to i32
  %tmp94 = getelementptr inbounds [8 x i32], [8 x i32]* %tmp21, i64 0, i64 %tmp70, !intel-tbaa !17
  store i32 %tmp93, i32* %tmp94, align 4, !tbaa !17
  %tmp95 = add nuw nsw i64 %tmp70, 1
  %tmp96 = icmp eq i64 %tmp95, 8
  br i1 %tmp96, label %bb97, label %bb69, !llvm.loop !20

bb97:                                             ; preds = %bb69
  br label %bb98

bb98:                                             ; preds = %bb103, %bb97
  %tmp99 = phi i64 [ %tmp104, %bb103 ], [ 0, %bb97 ]
  br label %bb106

bb100:                                            ; preds = %bb103
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %tmp43) #3
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %tmp42) #3
  %tmp101 = add nuw nsw i32 %tmp68, 1
  %tmp102 = icmp eq i32 %tmp101, %tmp47
  br i1 %tmp102, label %bb62, label %bb67, !llvm.loop !21

bb103:                                            ; preds = %bb106
  %tmp104 = add nuw nsw i64 %tmp99, 1
  %tmp105 = icmp eq i64 %tmp104, 4
  br i1 %tmp105, label %bb100, label %bb98, !llvm.loop !22

bb106:                                            ; preds = %bb106, %bb98
  %tmp107 = phi i64 [ 0, %bb98 ], [ %tmp120, %bb106 ]
  %tmp108 = getelementptr inbounds [8 x i32], [8 x i32]* %tmp21, i64 0, i64 %tmp107, !intel-tbaa !17
  %tmp109 = load i32, i32* %tmp108, align 4, !tbaa !17
  %tmp110 = mul nsw i32 %tmp109, %arg4
  %tmp111 = getelementptr inbounds [8 x i32], [8 x i32]* %tmp20, i64 0, i64 %tmp107, !intel-tbaa !17
  %tmp112 = load i32, i32* %tmp111, align 4, !tbaa !17
  %tmp113 = add nsw i32 %tmp110, %tmp112
  %tmp114 = sext i32 %tmp113 to i64
  %tmp115 = getelementptr inbounds [4 x float], [4 x float]* %tmp44, i64 %tmp114, i64 %tmp99
  %tmp116 = load float, float* %tmp115, align 4, !tbaa !23
  %tmp117 = getelementptr inbounds [4 x [8 x float]], [4 x [8 x float]]* %tmp, i64 0, i64 %tmp99, i64 %tmp107, !intel-tbaa !8
  %tmp118 = load float, float* %tmp117, align 4, !tbaa !8
  %tmp119 = fadd fast float %tmp118, %tmp116
  store float %tmp119, float* %tmp117, align 4, !tbaa !8
  %tmp120 = add nuw nsw i64 %tmp107, 1
  %tmp121 = icmp eq i64 %tmp120, 8
  br i1 %tmp121, label %bb103, label %bb106, !llvm.loop !25

bb122:                                            ; preds = %bb124, %bb63
  %tmp123 = phi i64 [ 0, %bb63 ], [ %tmp125, %bb124 ]
  br label %bb128

bb124:                                            ; preds = %bb128
  %tmp125 = add nuw nsw i64 %tmp123, 1
  %tmp126 = icmp eq i64 %tmp125, 4
  br i1 %tmp126, label %bb127, label %bb122, !llvm.loop !26

bb127:                                            ; preds = %bb124
  br label %bb139

bb128:                                            ; preds = %bb128, %bb122
  %tmp129 = phi i64 [ 0, %bb122 ], [ %tmp133, %bb128 ]
  %tmp130 = getelementptr inbounds [4 x [8 x float]], [4 x [8 x float]]* %tmp, i64 0, i64 %tmp123, i64 %tmp129, !intel-tbaa !8
  %tmp131 = load float, float* %tmp130, align 4, !tbaa !8
  %tmp132 = fmul fast float %tmp131, %tmp66
  store float %tmp132, float* %tmp130, align 4, !tbaa !8
  %tmp133 = add nuw nsw i64 %tmp129, 1
  %tmp134 = icmp eq i64 %tmp133, 8
  br i1 %tmp134, label %bb124, label %bb128, !llvm.loop !27

bb135:                                            ; preds = %bb154
  %tmp136 = bitcast float* %arg1 to [4 x float]*
  %tmp137 = mul nsw i32 %arg4, %arg3
  %tmp138 = add nsw i32 %tmp137, %arg2
  br label %bb157

bb139:                                            ; preds = %bb154, %bb127
  %tmp140 = phi i64 [ %tmp155, %bb154 ], [ 0, %bb127 ]
  %tmp141 = getelementptr inbounds [4 x [8 x float]], [4 x [8 x float]]* %tmp, i64 0, i64 3, i64 %tmp140, !intel-tbaa !8
  %tmp142 = load float, float* %tmp141, align 4, !tbaa !8
  %tmp143 = fcmp fast une float %tmp142, 0.000000e+00
  br i1 %tmp143, label %bb144, label %bb154

bb144:                                            ; preds = %bb139
  %tmp145 = fdiv fast float 1.000000e+00, %tmp142
  br label %bb146

bb146:                                            ; preds = %bb146, %bb144
  %tmp147 = phi i64 [ 0, %bb144 ], [ %tmp151, %bb146 ]
  %tmp148 = getelementptr inbounds [4 x [8 x float]], [4 x [8 x float]]* %tmp, i64 0, i64 %tmp147, i64 %tmp140, !intel-tbaa !8
  %tmp149 = load float, float* %tmp148, align 4, !tbaa !8
  %tmp150 = fmul fast float %tmp149, %tmp145
  store float %tmp150, float* %tmp148, align 4, !tbaa !8
  %tmp151 = add nuw nsw i64 %tmp147, 1
  %tmp152 = icmp eq i64 %tmp151, 4
  br i1 %tmp152, label %bb153, label %bb146, !llvm.loop !28

bb153:                                            ; preds = %bb146
  br label %bb154

bb154:                                            ; preds = %bb153, %bb139
  %tmp155 = add nuw nsw i64 %tmp140, 1
  %tmp156 = icmp eq i64 %tmp155, 8
  br i1 %tmp156, label %bb135, label %bb139, !llvm.loop !29

bb157:                                            ; preds = %bb160, %bb135
  %tmp158 = phi i64 [ 0, %bb135 ], [ %tmp161, %bb160 ]
  br label %bb163

bb159:                                            ; preds = %bb160
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %tmp27) #3
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %tmp26) #3
  call void @llvm.lifetime.end.p0i8(i64 128, i8* nonnull %tmp22) #3
  ret void

bb160:                                            ; preds = %bb163
  %tmp161 = add nuw nsw i64 %tmp158, 1
  %tmp162 = icmp eq i64 %tmp161, 4
  br i1 %tmp162, label %bb159, label %bb157, !llvm.loop !30

bb163:                                            ; preds = %bb163, %bb157
  %tmp164 = phi i64 [ 0, %bb157 ], [ %tmp171, %bb163 ]
  %tmp165 = getelementptr inbounds [4 x [8 x float]], [4 x [8 x float]]* %tmp, i64 0, i64 %tmp158, i64 %tmp164, !intel-tbaa !8
  %tmp166 = load float, float* %tmp165, align 4, !tbaa !8
  %tmp167 = trunc i64 %tmp164 to i32
  %tmp168 = add nsw i32 %tmp138, %tmp167
  %tmp169 = sext i32 %tmp168 to i64
  %tmp170 = getelementptr inbounds [4 x float], [4 x float]* %tmp136, i64 %tmp169, i64 %tmp158
  store float %tmp166, float* %tmp170, align 4, !tbaa !23
  %tmp171 = add nuw nsw i64 %tmp164, 1
  %tmp172 = icmp eq i64 %tmp171, 8
  br i1 %tmp172, label %bb160, label %bb163, !llvm.loop !31
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.maxnum.f32(float, float) #2

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
attributes #1 = { mustprogress nofree nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN8uuluuuuuuuuuuuuuuu__Z15evaluate_vectorPfS_iiiffffffffffffi,_ZGVcN8uuluuuuuuuuuuuuuuu__Z15evaluate_vectorPfS_iiiffffffffffffi,_ZGVdN8uuluuuuuuuuuuuuuuu__Z15evaluate_vectorPfS_iiiffffffffffffi,_ZGVeN8uuluuuuuuuuuuuuuuu__Z15evaluate_vectorPfS_iiiffffffffffffi,_ZGVbM8uuluuuuuuuuuuuuuuu__Z15evaluate_vectorPfS_iiiffffffffffffi,_ZGVcM8uuluuuuuuuuuuuuuuu__Z15evaluate_vectorPfS_iiiffffffffffffi,_ZGVdM8uuluuuuuuuuuuuuuuu__Z15evaluate_vectorPfS_iiiffffffffffffi,_ZGVeM8uuluuuuuuuuuuuuuuu__Z15evaluate_vectorPfS_iiiffffffffffffi" }
attributes #2 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { nounwind }

!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2, !3, !4}
!llvm.ident = !{!5}
!nvvm.annotations = !{}

!0 = !{i32 0, i32 2050, i32 39063270, !"_ZN10RadialBlur15execute_offloadEi", i32 296, i32 0, i32 0}
!1 = !{i32 0, i32 2050, i32 39063270, !"_ZN16RadialBlurLocals15execute_offloadEi", i32 406, i32 1, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 7, !"openmp", i32 50}
!4 = !{i32 7, !"uwtable", i32 1}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = !{!9, !11, i64 0}
!9 = !{!"array@_ZTSA4_A8_f", !10, i64 0}
!10 = !{!"array@_ZTSA8_f", !11, i64 0}
!11 = !{!"float", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C++ TBAA"}
!14 = distinct !{!14, !7}
!15 = !{!10, !11, i64 0}
!16 = distinct !{!16, !7}
!17 = !{!18, !19, i64 0}
!18 = !{!"array@_ZTSA8_i", !19, i64 0}
!19 = !{!"int", !12, i64 0}
!20 = distinct !{!20, !7}
!21 = distinct !{!21, !7}
!22 = distinct !{!22, !7}
!23 = !{!24, !11, i64 0}
!24 = !{!"array@_ZTSA4_f", !11, i64 0}
!25 = distinct !{!25, !7}
!26 = distinct !{!26, !7}
!27 = distinct !{!27, !7}
!28 = distinct !{!28, !7}
!29 = distinct !{!29, !7}
!30 = distinct !{!30, !7}
!31 = distinct !{!31, !7}
