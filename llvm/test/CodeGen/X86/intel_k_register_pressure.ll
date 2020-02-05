; RUN: llc < %s -mtriple=x86_64-unknown-linux-gnu -mcpu=skylake-avx512 -mattr=prefer-256-bit | FileCheck %s

; Make sure we don't spill k-registers in this loop

; CHECK-LABEL: wombat:
; CHECK-NOT: kmovw {{.*}} # 2-byte Spill

define i32 @wombat(i32 %arg, [400 x float]* %arg1) #0 {
bb:
  br label %bb2

bb2:                                              ; preds = %bb2, %bb
  %tmp = phi i32 [ 0, %bb ], [ %tmp48, %bb2 ]
  %tmp3 = phi <16 x i32> [ zeroinitializer, %bb ], [ %tmp47, %bb2 ]
  %tmp4 = phi <16 x i32> [ zeroinitializer, %bb ], [ %tmp44, %bb2 ]
  %tmp5 = insertelement <16 x [400 x float]*> undef, [400 x float]* %arg1, i32 0
  %tmp6 = shufflevector <16 x [400 x float]*> %tmp5, <16 x [400 x float]*> undef, <16 x i32> zeroinitializer
  %tmp7 = zext i32 %tmp to i64
  %tmp8 = shl nuw nsw i64 %tmp7, 2
  %tmp9 = insertelement <16 x i64> undef, i64 %tmp8, i32 0
  %tmp10 = shufflevector <16 x i64> %tmp9, <16 x i64> undef, <16 x i32> zeroinitializer
  %tmp11 = add <16 x i64> %tmp10, <i64 0, i64 4, i64 8, i64 12, i64 16, i64 20, i64 24, i64 28, i64 32, i64 36, i64 40, i64 44, i64 48, i64 52, i64 56, i64 60>
  %tmp12 = add <16 x i64> %tmp10, <i64 3, i64 7, i64 11, i64 15, i64 19, i64 23, i64 27, i64 31, i64 35, i64 39, i64 43, i64 47, i64 51, i64 55, i64 59, i64 63>
  %tmp13 = getelementptr inbounds [400 x float], <16 x [400 x float]*> %tmp6, <16 x i64> zeroinitializer, <16 x i64> %tmp12
  %tmp14 = call <16 x float> @llvm.masked.gather.v16f32.v16p0f32(<16 x float*> %tmp13, i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x float> undef)
  %tmp15 = call fast <16 x float> @llvm.fabs.v16f32(<16 x float> %tmp14)
  %tmp16 = fadd fast <16 x float> %tmp15, <float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000, float 0x3E80000000000000>
  %tmp17 = getelementptr inbounds [400 x float], <16 x [400 x float]*> %tmp6, <16 x i64> zeroinitializer, <16 x i64> %tmp11
  %tmp18 = call <16 x float> @llvm.masked.gather.v16f32.v16p0f32(<16 x float*> %tmp17, i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x float> undef)
  %tmp19 = fneg fast <16 x float> %tmp16
  %tmp20 = fcmp fast ogt <16 x float> %tmp18, %tmp16
  %tmp21 = select <16 x i1> %tmp20, <16 x i16> <i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2>, <16 x i16> zeroinitializer
  %tmp22 = fcmp fast olt <16 x float> %tmp18, %tmp19
  %tmp23 = select <16 x i1> %tmp22, <16 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>, <16 x i16> %tmp21
  %tmp24 = add <16 x i64> %tmp10, <i64 1, i64 5, i64 9, i64 13, i64 17, i64 21, i64 25, i64 29, i64 33, i64 37, i64 41, i64 45, i64 49, i64 53, i64 57, i64 61>
  %tmp25 = getelementptr inbounds [400 x float], <16 x [400 x float]*> %tmp6, <16 x i64> zeroinitializer, <16 x i64> %tmp24
  %tmp26 = call <16 x float> @llvm.masked.gather.v16f32.v16p0f32(<16 x float*> %tmp25, i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x float> undef)
  %tmp27 = or <16 x i16> %tmp23, <i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4, i16 4>
  %tmp28 = or <16 x i16> %tmp23, <i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8, i16 8>
  %tmp29 = fcmp fast olt <16 x float> %tmp26, %tmp19
  %tmp30 = select <16 x i1> %tmp29, <16 x i16> %tmp28, <16 x i16> %tmp23
  %tmp31 = fcmp fast ogt <16 x float> %tmp26, %tmp16
  %tmp32 = select <16 x i1> %tmp31, <16 x i16> %tmp27, <16 x i16> %tmp30
  %tmp33 = add <16 x i64> %tmp10, <i64 2, i64 6, i64 10, i64 14, i64 18, i64 22, i64 26, i64 30, i64 34, i64 38, i64 42, i64 46, i64 50, i64 54, i64 58, i64 62>
  %tmp34 = getelementptr inbounds [400 x float], <16 x [400 x float]*> %tmp6, <16 x i64> zeroinitializer, <16 x i64> %tmp33
  %tmp35 = call <16 x float> @llvm.masked.gather.v16f32.v16p0f32(<16 x float*> %tmp34, i32 4, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <16 x float> undef)
  %tmp36 = or <16 x i16> %tmp32, <i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16, i16 16>
  %tmp37 = or <16 x i16> %tmp32, <i16 32, i16 32, i16 32, i16 32, i16 32, i16 32, i16 32, i16 32, i16 32, i16 32, i16 32, i16 32, i16 32, i16 32, i16 32, i16 32>
  %tmp38 = fcmp fast ogt <16 x float> %tmp35, %tmp16
  %tmp39 = select <16 x i1> %tmp38, <16 x i16> %tmp37, <16 x i16> %tmp32
  %tmp40 = fcmp fast olt <16 x float> %tmp35, %tmp19
  %tmp41 = select <16 x i1> %tmp40, <16 x i16> %tmp36, <16 x i16> %tmp39
  %tmp42 = and <16 x i16> %tmp41, <i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3, i16 3>
  %tmp43 = zext <16 x i16> %tmp42 to <16 x i32>
  %tmp44 = or <16 x i32> %tmp4, %tmp43
  %tmp45 = and <16 x i16> %tmp41, <i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12, i16 12>
  %tmp46 = zext <16 x i16> %tmp45 to <16 x i32>
  %tmp47 = or <16 x i32> %tmp3, %tmp46
  %tmp48 = add nuw nsw i32 %tmp, 16
  %tmp49 = icmp sgt i32 %tmp48, %arg
  br i1 %tmp49, label %bb50, label %bb2

bb50:                                             ; preds = %bb2
  %tmp51 = shufflevector <16 x i32> %tmp44, <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp52 = or <16 x i32> %tmp44, %tmp51
  %tmp53 = shufflevector <16 x i32> %tmp52, <16 x i32> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp54 = or <16 x i32> %tmp52, %tmp53
  %tmp55 = shufflevector <16 x i32> %tmp54, <16 x i32> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp56 = or <16 x i32> %tmp54, %tmp55
  %tmp57 = shufflevector <16 x i32> %tmp56, <16 x i32> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp58 = or <16 x i32> %tmp56, %tmp57
  %tmp59 = extractelement <16 x i32> %tmp58, i64 0
  %tmp60 = shufflevector <16 x i32> %tmp47, <16 x i32> undef, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp61 = or <16 x i32> %tmp47, %tmp60
  %tmp62 = shufflevector <16 x i32> %tmp61, <16 x i32> undef, <16 x i32> <i32 4, i32 5, i32 6, i32 7, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp63 = or <16 x i32> %tmp61, %tmp62
  %tmp64 = shufflevector <16 x i32> %tmp63, <16 x i32> undef, <16 x i32> <i32 2, i32 3, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp65 = or <16 x i32> %tmp63, %tmp64
  %tmp66 = shufflevector <16 x i32> %tmp65, <16 x i32> undef, <16 x i32> <i32 1, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %tmp67 = or <16 x i32> %tmp65, %tmp66
  %tmp68 = extractelement <16 x i32> %tmp67, i64 0
  ret i32 %tmp68
}
; Function Attrs: nounwind readnone speculatable willreturn
declare <16 x float> @llvm.fabs.v16f32(<16 x float>) #1
; Function Attrs: nounwind readonly willreturn
declare <16 x float> @llvm.masked.gather.v16f32.v16p0f32(<16 x float*>, i32 immarg, <16 x i1>, <16 x float>) #2

attributes #0 = { "min-legal-vector-width"="256" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { nounwind readonly willreturn }
