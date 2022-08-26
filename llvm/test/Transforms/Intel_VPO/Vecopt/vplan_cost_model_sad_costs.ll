; RUN: opt < %s -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec \
; RUN:     -mattr=+avx2 -disable-output \
; RUN:     -vplan-cost-model-print-analysis-for-vf=4 | FileCheck %s

; The test checks that the cost of i16 loads in CM dumps is 1 (i.e. the
; load is recognized to be unit stride load.

; CHECK: Cost 1.046875 for i16 %vp{{[0-9]+}} = load i16* %vp{{[0-9]+}}
; CHECK: Cost 1.046875 for i16 %vp{{[0-9]+}} = load i16* %vp{{[0-9]+}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.wombat = type { i32, i32, i32, i32, i8* }

@global = external hidden unnamed_addr global %struct.wombat
@global.1 = external hidden unnamed_addr global %struct.wombat

; Function Attrs: nofree nounwind uwtable
define hidden void @zot(i32* nocapture readonly %arg, i32* nocapture readnone %arg1, i64 %arg2, i64 %arg3, i16* nocapture %arg4, i64 %arg5, i64 %arg6, i64 %arg7, i16* nocapture readonly %arg8, i16* nocapture readonly %arg9, i64 %arg10, i64 %arg11) #0 {
bb:
  %tmp = alloca i32, align 4
  %tmp12 = alloca i32, align 4
  %tmp13 = alloca i32, align 4
  %tmp14 = alloca i32, align 4
  store i32 0, i32* %tmp, align 4
  %tmp15 = trunc i64 %arg3 to i32
  %tmp16 = trunc i64 %arg5 to i32
  %tmp17 = trunc i64 %arg7 to i32
  %tmp18 = trunc i64 %arg11 to i32
  %tmp19 = load i32, i32* %arg, align 4
  store i32 0, i32* %tmp12, align 4
  store i32 %tmp18, i32* %tmp13, align 4
  store i32 1, i32* %tmp14, align 4
  call void @__kmpc_for_static_init_4(%struct.wombat* nonnull @global, i32 %tmp19, i32 34, i32* nonnull %tmp, i32* nonnull %tmp12, i32* nonnull %tmp13, i32* nonnull %tmp14, i32 1, i32 1)
  %tmp20 = load i32, i32* %tmp12, align 4
  %tmp21 = load i32, i32* %tmp13, align 4
  %tmp22 = icmp sgt i32 %tmp20, %tmp21
  br i1 %tmp22, label %bb48, label %bb23

bb23:                                             ; preds = %bb
  %tmp24 = trunc i64 %arg6 to i32
  %tmp25 = trunc i64 %arg2 to i32
  %tmp26 = icmp sgt i32 %tmp25, 0
  %tmp27 = mul i32 %tmp16, 27400
  %tmp28 = mul i32 %tmp27, %tmp24
  %tmp29 = add nsw i32 %tmp15, -4
  %tmp30 = add nsw i32 %tmp17, -4
  %tmp31 = add i64 %arg3, 4294967295
  %tmp32 = add i64 %arg7, -1
  %tmp33 = shl i64 %arg3, 32
  %tmp34 = ashr exact i64 %tmp33, 32
  %tmp35 = add i32 %tmp21, 1
  %tmp36 = shl i64 %arg2, 32
  %tmp37 = ashr exact i64 %tmp36, 32
  br label %bb38

bb38:                                             ; preds = %bb69, %bb23
  %tmp39 = phi i32 [ %tmp70, %bb69 ], [ %tmp20, %bb23 ]
  %tmp40 = sdiv i32 %tmp39, 4
  %tmp41 = and i32 %tmp39, 3
  %tmp42 = shl i32 %tmp40, 4
  %tmp43 = shl nuw nsw i32 %tmp41, 2
  %tmp44 = or i32 %tmp42, %tmp43
  %tmp45 = add nsw i32 %tmp44, -16
  br i1 %tmp26, label %bb46, label %bb69

bb46:                                             ; preds = %bb38
  br label %bb49

bb47:                                             ; preds = %bb69
  br label %bb48

bb48:                                             ; preds = %bb47, %bb
  tail call void @__kmpc_for_static_fini(%struct.wombat* nonnull @global.1, i32 %tmp19)
  ret void

bb49:                                             ; preds = %bb80, %bb46
  %tmp50 = phi i64 [ %tmp81, %bb80 ], [ 0, %bb46 ]
  %tmp51 = trunc i64 %tmp50 to i32
  %tmp52 = lshr i32 %tmp51, 2
  %tmp53 = shl i32 %tmp51, 2
  %tmp54 = add nsw i32 %tmp53, -16
  %tmp55 = mul nsw i32 %tmp53, %tmp15
  %tmp56 = add nsw i32 %tmp55, %tmp44
  %tmp57 = mul nsw i32 %tmp52, %tmp16
  %tmp58 = add nsw i32 %tmp57, %tmp40
  %tmp59 = mul i32 %tmp58, 17536
  %tmp60 = add nsw i32 %tmp59, %tmp28
  %tmp61 = and i32 %tmp53, 12
  %tmp62 = or i32 %tmp61, %tmp41
  %tmp63 = mul nuw nsw i32 %tmp62, 1096
  %tmp64 = add nsw i32 %tmp60, %tmp63
  %tmp65 = sext i32 %tmp64 to i64
  %tmp66 = getelementptr inbounds i16, i16* %arg4, i64 %tmp65
  %tmp67 = sext i32 %tmp56 to i64
  br label %bb72

bb68:                                             ; preds = %bb80
  br label %bb69

bb69:                                             ; preds = %bb68, %bb38
  %tmp70 = add nsw i32 %tmp39, 1
  %tmp71 = icmp eq i32 %tmp70, %tmp35
  br i1 %tmp71, label %bb47, label %bb38

bb72:                                             ; preds = %bb119, %bb49
  %tmp73 = phi i64 [ 0, %bb49 ], [ %tmp122, %bb119 ]
  %tmp74 = trunc i64 %tmp73 to i32
  %tmp75 = urem i32 %tmp74, 33
  %tmp76 = add nsw i32 %tmp45, %tmp75
  %tmp77 = udiv i32 %tmp74, 33
  %tmp78 = add nsw i32 %tmp54, %tmp77
  %tmp79 = icmp sgt i32 %tmp76, -1
  br i1 %tmp79, label %bb83, label %bb91

bb80:                                             ; preds = %bb119
  %tmp81 = add nuw nsw i64 %tmp50, 1
  %tmp82 = icmp eq i64 %tmp81, %tmp37
  br i1 %tmp82, label %bb68, label %bb49

bb83:                                             ; preds = %bb72
  %tmp84 = icmp sgt i32 %tmp76, %tmp29
  %tmp85 = icmp slt i32 %tmp78, 0
  %tmp86 = or i1 %tmp84, %tmp85
  %tmp87 = icmp sgt i32 %tmp78, %tmp30
  %tmp88 = or i1 %tmp87, %tmp86
  br i1 %tmp88, label %bb91, label %bb89

bb89:                                             ; preds = %bb83
  %tmp90 = zext i32 %tmp78 to i64
  br label %bb106

bb91:                                             ; preds = %bb83, %bb72
  %tmp92 = sext i32 %tmp76 to i64
  %tmp93 = sext i32 %tmp78 to i64
  br label %bb94

bb94:                                             ; preds = %bb151, %bb91
  %tmp95 = phi i64 [ 0, %bb91 ], [ %tmp153, %bb151 ]
  %tmp96 = phi i16 [ 0, %bb91 ], [ %tmp152, %bb151 ]
  %tmp97 = add nsw i64 %tmp95, %tmp93
  %tmp98 = icmp sgt i64 %tmp97, 0
  %tmp99 = select i1 %tmp98, i64 %tmp97, i64 0
  %tmp100 = trunc i64 %tmp99 to i32
  %tmp101 = icmp slt i32 %tmp100, %tmp17
  %tmp102 = select i1 %tmp101, i64 %tmp99, i64 %tmp32
  %tmp103 = mul i64 %tmp102, %arg3
  %tmp104 = mul nsw i64 %tmp95, %tmp34
  %tmp105 = add nsw i64 %tmp104, %tmp67
  br label %bb124

bb106:                                            ; preds = %bb176, %bb89
  %tmp107 = phi i64 [ 0, %bb89 ], [ %tmp178, %bb176 ]
  %tmp108 = phi i16 [ 0, %bb89 ], [ %tmp177, %bb176 ]
  %tmp109 = add nuw nsw i64 %tmp107, %tmp90
  %tmp110 = mul nsw i64 %tmp109, %tmp34
  %tmp111 = trunc i64 %tmp110 to i32
  %tmp112 = add i32 %tmp76, %tmp111
  %tmp113 = mul nsw i64 %tmp107, %tmp34
  %tmp114 = add nsw i64 %tmp113, %tmp67
  br label %bb155

bb115:                                            ; preds = %bb151
  %tmp116 = phi i16 [ %tmp152, %bb151 ]
  br label %bb119

bb117:                                            ; preds = %bb176
  %tmp118 = phi i16 [ %tmp177, %bb176 ]
  br label %bb119

bb119:                                            ; preds = %bb117, %bb115
  %tmp120 = phi i16 [ %tmp116, %bb115 ], [ %tmp118, %bb117 ]
  %tmp121 = getelementptr inbounds i16, i16* %tmp66, i64 %tmp73
  store i16 %tmp120, i16* %tmp121, align 2
  %tmp122 = add nuw nsw i64 %tmp73, 1
  %tmp123 = icmp eq i64 %tmp122, 1089
  br i1 %tmp123, label %bb80, label %bb72

bb124:                                            ; preds = %bb94
  %tmp148 = add nuw nsw i16 0, 0
  br label %bb151

bb151:                                            ; preds = %bb124
  %tmp152 = phi i16 [ %tmp148, %bb124 ]
  %tmp153 = add nuw nsw i64 %tmp95, 1
  %tmp154 = icmp eq i64 %tmp153, 4
  br i1 %tmp154, label %bb115, label %bb94

bb155:                                            ; preds = %bb155, %bb106
  %tmp156 = phi i64 [ 0, %bb106 ], [ %tmp174, %bb155 ]
  %tmp157 = phi i16 [ %tmp108, %bb106 ], [ %tmp173, %bb155 ]
  %tmp158 = trunc i64 %tmp156 to i32
  %tmp159 = add i32 %tmp112, %tmp158
  %tmp160 = sext i32 %tmp159 to i64
  %tmp161 = getelementptr inbounds i16, i16* %arg8, i64 %tmp160
  %tmp162 = load i16, i16* %tmp161, align 2
  %tmp163 = zext i16 %tmp162 to i32
  %tmp164 = add nsw i64 %tmp114, %tmp156
  %tmp165 = getelementptr inbounds i16, i16* %arg9, i64 %tmp164
  %tmp166 = load i16, i16* %tmp165, align 2
  %tmp167 = zext i16 %tmp166 to i32
  %tmp168 = sub nsw i32 %tmp163, %tmp167
  %tmp169 = icmp slt i32 %tmp168, 0
  %tmp170 = sub nsw i32 0, %tmp168
  %tmp171 = select i1 %tmp169, i32 %tmp170, i32 %tmp168
  %tmp172 = trunc i32 %tmp171 to i16
  %tmp173 = add i16 %tmp157, %tmp172
  %tmp174 = add nuw nsw i64 %tmp156, 1
  %tmp175 = icmp eq i64 %tmp174, 4
  br i1 %tmp175, label %bb176, label %bb155

bb176:                                            ; preds = %bb155
  %tmp177 = phi i16 [ %tmp173, %bb155 ]
  %tmp178 = add nuw nsw i64 %tmp107, 1
  %tmp179 = icmp eq i64 %tmp178, 4
  br i1 %tmp179, label %bb117, label %bb106
}

; Function Attrs: nofree nounwind
declare void @__kmpc_for_static_init_4(%struct.wombat* nocapture readonly, i32, i32, i32* nocapture, i32* nocapture, i32* nocapture, i32* nocapture, i32, i32) local_unnamed_addr #1

; Function Attrs: nofree nounwind
declare void @__kmpc_for_static_fini(%struct.wombat* nocapture readonly, i32) local_unnamed_addr #1

attributes #0 = { nofree nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "mt-func"="true" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nofree nounwind }

