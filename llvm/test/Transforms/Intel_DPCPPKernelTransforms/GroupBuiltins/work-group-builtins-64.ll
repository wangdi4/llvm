; RUN: opt -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S < %s
;; This file is used as Built-in module to test work group built-in pass for 64bit modules
;; The only requirment is to contain a valid LLVM IR.

; Function Attrs: nounwind readnone
declare i32 @_Z19work_group_add_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z19work_group_add_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z19work_group_add_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z19work_group_add_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z19work_group_add_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z19work_group_add_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z19work_group_add_utiljj(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z19work_group_add_utilDv4_jS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z19work_group_add_utilDv8_jS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z19work_group_add_utilDv16_jS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z19work_group_add_utilDv32_jS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z19work_group_add_utilDv64_jS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z19work_group_add_utilll(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z19work_group_add_utilDv4_lS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z19work_group_add_utilDv8_lS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z19work_group_add_utilDv16_lS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z19work_group_add_utilDv32_lS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z19work_group_add_utilDv64_lS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z19work_group_add_utilmm(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z19work_group_add_utilDv4_mS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z19work_group_add_utilDv8_mS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z19work_group_add_utilDv16_mS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z19work_group_add_utilDv32_mS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z19work_group_add_utilDv64_mS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare float @_Z19work_group_add_utilff(float %src, float %accum) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z19work_group_add_utilDv4_fS_(<4 x float> %src, <4 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z19work_group_add_utilDv8_fS_(<8 x float> %src, <8 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z19work_group_add_utilDv16_fS_(<16 x float> %src, <16 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z19work_group_add_utilDv32_fS_(<32 x float> %src, <32 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z19work_group_add_utilDv64_fS_(<64 x float> %src, <64 x float> %accum) #3

; Function Attrs: nounwind readnone
declare double @_Z19work_group_add_utildd(double %src, double %accum) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z19work_group_add_utilDv4_dS_(<4 x double> %src, <4 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z19work_group_add_utilDv8_dS_(<8 x double> %src, <8 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z19work_group_add_utilDv16_dS_(<16 x double> %src, <16 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z19work_group_add_utilDv32_dS_(<32 x double> %src, <32 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z19work_group_add_utilDv64_dS_(<64 x double> %src, <64 x double> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z19work_group_mul_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z19work_group_mul_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z19work_group_mul_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z19work_group_mul_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z19work_group_mul_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z19work_group_mul_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z19work_group_mul_utiljj(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z19work_group_mul_utilDv4_jS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z19work_group_mul_utilDv8_jS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z19work_group_mul_utilDv16_jS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z19work_group_mul_utilDv32_jS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z19work_group_mul_utilDv64_jS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z19work_group_mul_utilll(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z19work_group_mul_utilDv4_lS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z19work_group_mul_utilDv8_lS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z19work_group_mul_utilDv16_lS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z19work_group_mul_utilDv32_lS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z19work_group_mul_utilDv64_lS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z19work_group_mul_utilmm(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z19work_group_mul_utilDv4_mS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z19work_group_mul_utilDv8_mS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z19work_group_mul_utilDv16_mS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z19work_group_mul_utilDv32_mS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z19work_group_mul_utilDv64_mS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare float @_Z19work_group_mul_utilff(float %src, float %accum) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z19work_group_mul_utilDv4_fS_(<4 x float> %src, <4 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z19work_group_mul_utilDv8_fS_(<8 x float> %src, <8 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z19work_group_mul_utilDv16_fS_(<16 x float> %src, <16 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z19work_group_mul_utilDv32_fS_(<32 x float> %src, <32 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z19work_group_mul_utilDv64_fS_(<64 x float> %src, <64 x float> %accum) #3

; Function Attrs: nounwind readnone
declare double @_Z19work_group_mul_utildd(double %src, double %accum) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z19work_group_mul_utilDv4_dS_(<4 x double> %src, <4 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z19work_group_mul_utilDv8_dS_(<8 x double> %src, <8 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z19work_group_mul_utilDv16_dS_(<16 x double> %src, <16 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z19work_group_mul_utilDv32_dS_(<32 x double> %src, <32 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z19work_group_mul_utilDv64_dS_(<64 x double> %src, <64 x double> %accum) #3

; Function Attrs: nounwind readnone
declare signext i8 @_Z27work_group_bitwise_and_utilcc(i8 signext %src, i8 signext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z27work_group_bitwise_and_utilDv4_cS_(<4 x i8> %src, <4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z27work_group_bitwise_and_utilDv8_cS_(<8 x i8> %src, <8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z27work_group_bitwise_and_utilDv16_cS_(<16 x i8> %src, <16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z27work_group_bitwise_and_utilDv32_cS_(<32 x i8> %src, <32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z27work_group_bitwise_and_utilDv64_cS_(<64 x i8> %src, <64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare zeroext i8 @_Z27work_group_bitwise_and_utilhh(i8 zeroext %src, i8 zeroext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z27work_group_bitwise_and_utilDv4_hS_(<4 x i8> %src, <4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z27work_group_bitwise_and_utilDv8_hS_(<8 x i8> %src, <8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z27work_group_bitwise_and_utilDv16_hS_(<16 x i8> %src, <16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z27work_group_bitwise_and_utilDv32_hS_(<32 x i8> %src, <32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z27work_group_bitwise_and_utilDv64_hS_(<64 x i8> %src, <64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare signext i16 @_Z27work_group_bitwise_and_utilss(i16 signext %src, i16 signext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z27work_group_bitwise_and_utilDv4_sS_(<4 x i16> %src, <4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z27work_group_bitwise_and_utilDv8_sS_(<8 x i16> %src, <8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z27work_group_bitwise_and_utilDv16_sS_(<16 x i16> %src, <16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z27work_group_bitwise_and_utilDv32_sS_(<32 x i16> %src, <32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z27work_group_bitwise_and_utilDv64_sS_(<64 x i16> %src, <64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare zeroext i16 @_Z27work_group_bitwise_and_utiltt(i16 zeroext %src, i16 zeroext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z27work_group_bitwise_and_utilDv4_tS_(<4 x i16> %src, <4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z27work_group_bitwise_and_utilDv8_tS_(<8 x i16> %src, <8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z27work_group_bitwise_and_utilDv16_tS_(<16 x i16> %src, <16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z27work_group_bitwise_and_utilDv32_tS_(<32 x i16> %src, <32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z27work_group_bitwise_and_utilDv64_tS_(<64 x i16> %src, <64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z27work_group_bitwise_and_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z27work_group_bitwise_and_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z27work_group_bitwise_and_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z27work_group_bitwise_and_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z27work_group_bitwise_and_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z27work_group_bitwise_and_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z27work_group_bitwise_and_utiljj(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z27work_group_bitwise_and_utilDv4_jS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z27work_group_bitwise_and_utilDv8_jS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z27work_group_bitwise_and_utilDv16_jS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z27work_group_bitwise_and_utilDv32_jS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z27work_group_bitwise_and_utilDv64_jS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z27work_group_bitwise_and_utilll(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z27work_group_bitwise_and_utilDv4_lS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z27work_group_bitwise_and_utilDv8_lS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z27work_group_bitwise_and_utilDv16_lS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z27work_group_bitwise_and_utilDv32_lS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z27work_group_bitwise_and_utilDv64_lS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z27work_group_bitwise_and_utilmm(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z27work_group_bitwise_and_utilDv4_mS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z27work_group_bitwise_and_utilDv8_mS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z27work_group_bitwise_and_utilDv16_mS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z27work_group_bitwise_and_utilDv32_mS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z27work_group_bitwise_and_utilDv64_mS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare signext i8 @_Z26work_group_bitwise_or_utilcc(i8 signext %src, i8 signext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z26work_group_bitwise_or_utilDv4_cS_(<4 x i8> %src, <4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z26work_group_bitwise_or_utilDv8_cS_(<8 x i8> %src, <8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z26work_group_bitwise_or_utilDv16_cS_(<16 x i8> %src, <16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z26work_group_bitwise_or_utilDv32_cS_(<32 x i8> %src, <32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z26work_group_bitwise_or_utilDv64_cS_(<64 x i8> %src, <64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare zeroext i8 @_Z26work_group_bitwise_or_utilhh(i8 zeroext %src, i8 zeroext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z26work_group_bitwise_or_utilDv4_hS_(<4 x i8> %src, <4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z26work_group_bitwise_or_utilDv8_hS_(<8 x i8> %src, <8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z26work_group_bitwise_or_utilDv16_hS_(<16 x i8> %src, <16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z26work_group_bitwise_or_utilDv32_hS_(<32 x i8> %src, <32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z26work_group_bitwise_or_utilDv64_hS_(<64 x i8> %src, <64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare signext i16 @_Z26work_group_bitwise_or_utilss(i16 signext %src, i16 signext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z26work_group_bitwise_or_utilDv4_sS_(<4 x i16> %src, <4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z26work_group_bitwise_or_utilDv8_sS_(<8 x i16> %src, <8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z26work_group_bitwise_or_utilDv16_sS_(<16 x i16> %src, <16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z26work_group_bitwise_or_utilDv32_sS_(<32 x i16> %src, <32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z26work_group_bitwise_or_utilDv64_sS_(<64 x i16> %src, <64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare zeroext i16 @_Z26work_group_bitwise_or_utiltt(i16 zeroext %src, i16 zeroext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z26work_group_bitwise_or_utilDv4_tS_(<4 x i16> %src, <4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z26work_group_bitwise_or_utilDv8_tS_(<8 x i16> %src, <8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z26work_group_bitwise_or_utilDv16_tS_(<16 x i16> %src, <16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z26work_group_bitwise_or_utilDv32_tS_(<32 x i16> %src, <32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z26work_group_bitwise_or_utilDv64_tS_(<64 x i16> %src, <64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z26work_group_bitwise_or_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z26work_group_bitwise_or_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z26work_group_bitwise_or_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z26work_group_bitwise_or_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z26work_group_bitwise_or_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z26work_group_bitwise_or_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z26work_group_bitwise_or_utiljj(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z26work_group_bitwise_or_utilDv4_jS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z26work_group_bitwise_or_utilDv8_jS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z26work_group_bitwise_or_utilDv16_jS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z26work_group_bitwise_or_utilDv32_jS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z26work_group_bitwise_or_utilDv64_jS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z26work_group_bitwise_or_utilll(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z26work_group_bitwise_or_utilDv4_lS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z26work_group_bitwise_or_utilDv8_lS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z26work_group_bitwise_or_utilDv16_lS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z26work_group_bitwise_or_utilDv32_lS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z26work_group_bitwise_or_utilDv64_lS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z26work_group_bitwise_or_utilmm(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z26work_group_bitwise_or_utilDv4_mS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z26work_group_bitwise_or_utilDv8_mS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z26work_group_bitwise_or_utilDv16_mS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z26work_group_bitwise_or_utilDv32_mS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z26work_group_bitwise_or_utilDv64_mS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare signext i8 @_Z27work_group_bitwise_xor_utilcc(i8 signext %src, i8 signext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z27work_group_bitwise_xor_utilDv4_cS_(<4 x i8> %src, <4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z27work_group_bitwise_xor_utilDv8_cS_(<8 x i8> %src, <8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z27work_group_bitwise_xor_utilDv16_cS_(<16 x i8> %src, <16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z27work_group_bitwise_xor_utilDv32_cS_(<32 x i8> %src, <32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z27work_group_bitwise_xor_utilDv64_cS_(<64 x i8> %src, <64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare zeroext i8 @_Z27work_group_bitwise_xor_utilhh(i8 zeroext %src, i8 zeroext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z27work_group_bitwise_xor_utilDv4_hS_(<4 x i8> %src, <4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z27work_group_bitwise_xor_utilDv8_hS_(<8 x i8> %src, <8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z27work_group_bitwise_xor_utilDv16_hS_(<16 x i8> %src, <16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z27work_group_bitwise_xor_utilDv32_hS_(<32 x i8> %src, <32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z27work_group_bitwise_xor_utilDv64_hS_(<64 x i8> %src, <64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare signext i16 @_Z27work_group_bitwise_xor_utilss(i16 signext %src, i16 signext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z27work_group_bitwise_xor_utilDv4_sS_(<4 x i16> %src, <4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z27work_group_bitwise_xor_utilDv8_sS_(<8 x i16> %src, <8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z27work_group_bitwise_xor_utilDv16_sS_(<16 x i16> %src, <16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z27work_group_bitwise_xor_utilDv32_sS_(<32 x i16> %src, <32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z27work_group_bitwise_xor_utilDv64_sS_(<64 x i16> %src, <64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare zeroext i16 @_Z27work_group_bitwise_xor_utiltt(i16 zeroext %src, i16 zeroext %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z27work_group_bitwise_xor_utilDv4_tS_(<4 x i16> %src, <4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z27work_group_bitwise_xor_utilDv8_tS_(<8 x i16> %src, <8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z27work_group_bitwise_xor_utilDv16_tS_(<16 x i16> %src, <16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z27work_group_bitwise_xor_utilDv32_tS_(<32 x i16> %src, <32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z27work_group_bitwise_xor_utilDv64_tS_(<64 x i16> %src, <64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z27work_group_bitwise_xor_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z27work_group_bitwise_xor_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z27work_group_bitwise_xor_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z27work_group_bitwise_xor_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z27work_group_bitwise_xor_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z27work_group_bitwise_xor_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z27work_group_bitwise_xor_utiljj(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z27work_group_bitwise_xor_utilDv4_jS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z27work_group_bitwise_xor_utilDv8_jS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z27work_group_bitwise_xor_utilDv16_jS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z27work_group_bitwise_xor_utilDv32_jS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z27work_group_bitwise_xor_utilDv64_jS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z27work_group_bitwise_xor_utilll(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z27work_group_bitwise_xor_utilDv4_lS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z27work_group_bitwise_xor_utilDv8_lS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z27work_group_bitwise_xor_utilDv16_lS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z27work_group_bitwise_xor_utilDv32_lS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z27work_group_bitwise_xor_utilDv64_lS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z27work_group_bitwise_xor_utilmm(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z27work_group_bitwise_xor_utilDv4_mS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z27work_group_bitwise_xor_utilDv8_mS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z27work_group_bitwise_xor_utilDv16_mS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z27work_group_bitwise_xor_utilDv32_mS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z27work_group_bitwise_xor_utilDv64_mS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z27work_group_logical_and_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z27work_group_logical_and_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z27work_group_logical_and_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z27work_group_logical_and_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z27work_group_logical_and_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z27work_group_logical_and_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z26work_group_logical_or_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z26work_group_logical_or_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z26work_group_logical_or_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z26work_group_logical_or_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z26work_group_logical_or_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z26work_group_logical_or_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z27work_group_logical_xor_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z27work_group_logical_xor_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z27work_group_logical_xor_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z27work_group_logical_xor_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z27work_group_logical_xor_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z27work_group_logical_xor_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z14work_group_alli(i32 %predicate) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z14work_group_allDv4_i(<4 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z14work_group_allDv8_i(<8 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z14work_group_allDv16_i(<16 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z14work_group_allDv32_i(<32 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z14work_group_allDv64_i(<64 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare i32 @_Z19work_group_all_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z25__finalize_work_group_allDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z25__finalize_work_group_allDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z25__finalize_work_group_allDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z25__finalize_work_group_allDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z25__finalize_work_group_allDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind
declare i32 @_Z14work_group_alliPi(i32 %predicate, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z14work_group_allDv4_iPS_(<4 x i32> %predicate, <4 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z19work_group_all_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind
declare <8 x i32> @_Z14work_group_allDv8_iPS_(<8 x i32> %predicate, <8 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z19work_group_all_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind
declare <16 x i32> @_Z14work_group_allDv16_iPS_(<16 x i32> %predicate, <16 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z19work_group_all_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind
declare <32 x i32> @_Z14work_group_allDv32_iPS_(<32 x i32> %predicate, <32 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z19work_group_all_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind
declare <64 x i32> @_Z14work_group_allDv64_iPS_(<64 x i32> %predicate, <64 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z19work_group_all_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z14work_group_anyi(i32 %predicate) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z14work_group_anyDv4_i(<4 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z14work_group_anyDv8_i(<8 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z14work_group_anyDv16_i(<16 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z14work_group_anyDv32_i(<32 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z14work_group_anyDv64_i(<64 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z25__finalize_work_group_anyDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z19work_group_any_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z25__finalize_work_group_anyDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z25__finalize_work_group_anyDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z25__finalize_work_group_anyDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z25__finalize_work_group_anyDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind
declare i32 @_Z14work_group_anyiPi(i32 %predicate, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z14work_group_anyDv4_iPS_(<4 x i32> %predicate, <4 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z19work_group_any_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind
declare <8 x i32> @_Z14work_group_anyDv8_iPS_(<8 x i32> %predicate, <8 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z19work_group_any_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind
declare <16 x i32> @_Z14work_group_anyDv16_iPS_(<16 x i32> %predicate, <16 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z19work_group_any_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind
declare <32 x i32> @_Z14work_group_anyDv32_iPS_(<32 x i32> %predicate, <32 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z19work_group_any_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind
declare <64 x i32> @_Z14work_group_anyDv64_iPS_(<64 x i32> %predicate, <64 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z19work_group_any_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastim(i32 %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_im(<4 x i32> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_im(<8 x i32> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_im(<16 x i32> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_im(<32 x i32> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_im(<64 x i32> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastjm(i32 %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_jm(<4 x i32> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_jm(<8 x i32> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_jm(<16 x i32> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_jm(<32 x i32> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_jm(<64 x i32> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastlm(i64 %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_lm(<4 x i64> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_lm(<8 x i64> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_lm(<16 x i64> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_lm(<32 x i64> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_lm(<64 x i64> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastmm(i64 %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_mm(<4 x i64> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_mm(<8 x i64> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_mm(<16 x i64> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_mm(<32 x i64> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_mm(<64 x i64> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare float @_Z20work_group_broadcastfm(float %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z20work_group_broadcastDv4_fm(<4 x float> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z20work_group_broadcastDv8_fm(<8 x float> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z20work_group_broadcastDv16_fm(<16 x float> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z20work_group_broadcastDv32_fm(<32 x float> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z20work_group_broadcastDv64_fm(<64 x float> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare double @_Z20work_group_broadcastdm(double %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z20work_group_broadcastDv4_dm(<4 x double> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z20work_group_broadcastDv8_dm(<8 x double> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z20work_group_broadcastDv16_dm(<16 x double> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z20work_group_broadcastDv32_dm(<32 x double> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z20work_group_broadcastDv64_dm(<64 x double> %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastimm(i32 %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_imm(<4 x i32> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_imm(<8 x i32> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_imm(<16 x i32> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_imm(<32 x i32> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_imm(<64 x i32> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastjmm(i32 %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_jmm(<4 x i32> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_jmm(<8 x i32> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_jmm(<16 x i32> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_jmm(<32 x i32> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_jmm(<64 x i32> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastlmm(i64 %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_lmm(<4 x i64> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_lmm(<8 x i64> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_lmm(<16 x i64> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_lmm(<32 x i64> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_lmm(<64 x i64> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastmmm(i64 %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_mmm(<4 x i64> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_mmm(<8 x i64> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_mmm(<16 x i64> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_mmm(<32 x i64> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_mmm(<64 x i64> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare float @_Z20work_group_broadcastfmm(float %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z20work_group_broadcastDv4_fmm(<4 x float> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z20work_group_broadcastDv8_fmm(<8 x float> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z20work_group_broadcastDv16_fmm(<16 x float> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z20work_group_broadcastDv32_fmm(<32 x float> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z20work_group_broadcastDv64_fmm(<64 x float> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare double @_Z20work_group_broadcastdmm(double %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z20work_group_broadcastDv4_dmm(<4 x double> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z20work_group_broadcastDv8_dmm(<8 x double> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z20work_group_broadcastDv16_dmm(<16 x double> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z20work_group_broadcastDv32_dmm(<32 x double> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z20work_group_broadcastDv64_dmm(<64 x double> %src, i64 %local_id_x, i64 %local_id_y) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastimmm(i32 %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_immm(<4 x i32> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_immm(<8 x i32> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_immm(<16 x i32> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_immm(<32 x i32> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_immm(<64 x i32> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastjmmm(i32 %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_jmmm(<4 x i32> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_jmmm(<8 x i32> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_jmmm(<16 x i32> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_jmmm(<32 x i32> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_jmmm(<64 x i32> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastlmmm(i64 %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_lmmm(<4 x i64> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_lmmm(<8 x i64> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_lmmm(<16 x i64> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_lmmm(<32 x i64> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_lmmm(<64 x i64> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastmmmm(i64 %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_mmmm(<4 x i64> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_mmmm(<8 x i64> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_mmmm(<16 x i64> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_mmmm(<32 x i64> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_mmmm(<64 x i64> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare float @_Z20work_group_broadcastfmmm(float %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z20work_group_broadcastDv4_fmmm(<4 x float> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z20work_group_broadcastDv8_fmmm(<8 x float> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z20work_group_broadcastDv16_fmmm(<16 x float> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z20work_group_broadcastDv32_fmmm(<32 x float> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z20work_group_broadcastDv64_fmmm(<64 x float> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare double @_Z20work_group_broadcastdmmm(double %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z20work_group_broadcastDv4_dmmm(<4 x double> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z20work_group_broadcastDv8_dmmm(<8 x double> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z20work_group_broadcastDv16_dmmm(<16 x double> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z20work_group_broadcastDv32_dmmm(<32 x double> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z20work_group_broadcastDv64_dmmm(<64 x double> %src, i64 %local_id_x, i64 %local_id_y, i64 %local_id_z) #3

; Function Attrs: nounwind
declare signext i8 @_Z20work_group_broadcastcmmPc(i8 signext %src, i64 %linear_local_id, i64 %linear_id, i8* %accum) #5

; Function Attrs: nounwind
declare <4 x i8> @_Z20work_group_broadcastDv4_cmmPS_(<4 x i8> %src, i64 %linear_local_id, i64 %linear_id, <4 x i8>* %accum) #5

; Function Attrs: nounwind
declare <8 x i8> @_Z20work_group_broadcastDv8_cmmPS_(<8 x i8> %src, i64 %linear_local_id, i64 %linear_id, <8 x i8>* %accum) #5

; Function Attrs: nounwind
declare <16 x i8> @_Z20work_group_broadcastDv16_cmmPS_(<16 x i8> %src, i64 %linear_local_id, i64 %linear_id, <16 x i8>* %accum) #5

; Function Attrs: nounwind
declare <32 x i8> @_Z20work_group_broadcastDv32_cmmPS_(<32 x i8> %src, i64 %linear_local_id, i64 %linear_id, <32 x i8>* %accum) #5

; Function Attrs: nounwind
declare <64 x i8> @_Z20work_group_broadcastDv64_cmmPS_(<64 x i8> %src, i64 %linear_local_id, i64 %linear_id, <64 x i8>* %accum) #5

; Function Attrs: nounwind
declare zeroext i8 @_Z20work_group_broadcasthmmPh(i8 zeroext %src, i64 %linear_local_id, i64 %linear_id, i8* %accum) #5

; Function Attrs: nounwind
declare <4 x i8> @_Z20work_group_broadcastDv4_hmmPS_(<4 x i8> %src, i64 %linear_local_id, i64 %linear_id, <4 x i8>* %accum) #5

; Function Attrs: nounwind
declare <8 x i8> @_Z20work_group_broadcastDv8_hmmPS_(<8 x i8> %src, i64 %linear_local_id, i64 %linear_id, <8 x i8>* %accum) #5

; Function Attrs: nounwind
declare <16 x i8> @_Z20work_group_broadcastDv16_hmmPS_(<16 x i8> %src, i64 %linear_local_id, i64 %linear_id, <16 x i8>* %accum) #5

; Function Attrs: nounwind
declare <32 x i8> @_Z20work_group_broadcastDv32_hmmPS_(<32 x i8> %src, i64 %linear_local_id, i64 %linear_id, <32 x i8>* %accum) #5

; Function Attrs: nounwind
declare <64 x i8> @_Z20work_group_broadcastDv64_hmmPS_(<64 x i8> %src, i64 %linear_local_id, i64 %linear_id, <64 x i8>* %accum) #5

; Function Attrs: nounwind
declare signext i16 @_Z20work_group_broadcastsmmPs(i16 signext %src, i64 %linear_local_id, i64 %linear_id, i16* %accum) #5

; Function Attrs: nounwind
declare <4 x i16> @_Z20work_group_broadcastDv4_smmPS_(<4 x i16> %src, i64 %linear_local_id, i64 %linear_id, <4 x i16>* %accum) #5

; Function Attrs: nounwind
declare <8 x i16> @_Z20work_group_broadcastDv8_smmPS_(<8 x i16> %src, i64 %linear_local_id, i64 %linear_id, <8 x i16>* %accum) #5

; Function Attrs: nounwind
declare <16 x i16> @_Z20work_group_broadcastDv16_smmPS_(<16 x i16> %src, i64 %linear_local_id, i64 %linear_id, <16 x i16>* %accum) #5

; Function Attrs: nounwind
declare <32 x i16> @_Z20work_group_broadcastDv32_smmPS_(<32 x i16> %src, i64 %linear_local_id, i64 %linear_id, <32 x i16>* %accum) #5

; Function Attrs: nounwind
declare <64 x i16> @_Z20work_group_broadcastDv64_smmPS_(<64 x i16> %src, i64 %linear_local_id, i64 %linear_id, <64 x i16>* %accum) #5

; Function Attrs: nounwind
declare zeroext i16 @_Z20work_group_broadcasttmmPt(i16 zeroext %src, i64 %linear_local_id, i64 %linear_id, i16* %accum) #5

; Function Attrs: nounwind
declare <4 x i16> @_Z20work_group_broadcastDv4_tmmPS_(<4 x i16> %src, i64 %linear_local_id, i64 %linear_id, <4 x i16>* %accum) #5

; Function Attrs: nounwind
declare <8 x i16> @_Z20work_group_broadcastDv8_tmmPS_(<8 x i16> %src, i64 %linear_local_id, i64 %linear_id, <8 x i16>* %accum) #5

; Function Attrs: nounwind
declare <16 x i16> @_Z20work_group_broadcastDv16_tmmPS_(<16 x i16> %src, i64 %linear_local_id, i64 %linear_id, <16 x i16>* %accum) #5

; Function Attrs: nounwind
declare <32 x i16> @_Z20work_group_broadcastDv32_tmmPS_(<32 x i16> %src, i64 %linear_local_id, i64 %linear_id, <32 x i16>* %accum) #5

; Function Attrs: nounwind
declare <64 x i16> @_Z20work_group_broadcastDv64_tmmPS_(<64 x i16> %src, i64 %linear_local_id, i64 %linear_id, <64 x i16>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z20work_group_broadcastimmPi(i32 %src, i64 %linear_local_id, i64 %linear_id, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z20work_group_broadcastDv4_immPS_(<4 x i32> %src, i64 %linear_local_id, i64 %linear_id, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z20work_group_broadcastDv8_immPS_(<8 x i32> %src, i64 %linear_local_id, i64 %linear_id, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z20work_group_broadcastDv16_immPS_(<16 x i32> %src, i64 %linear_local_id, i64 %linear_id, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z20work_group_broadcastDv32_immPS_(<32 x i32> %src, i64 %linear_local_id, i64 %linear_id, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z20work_group_broadcastDv64_immPS_(<64 x i32> %src, i64 %linear_local_id, i64 %linear_id, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z20work_group_broadcastjmmPj(i32 %src, i64 %linear_local_id, i64 %linear_id, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z20work_group_broadcastDv4_jmmPS_(<4 x i32> %src, i64 %linear_local_id, i64 %linear_id, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z20work_group_broadcastDv8_jmmPS_(<8 x i32> %src, i64 %linear_local_id, i64 %linear_id, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z20work_group_broadcastDv16_jmmPS_(<16 x i32> %src, i64 %linear_local_id, i64 %linear_id, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z20work_group_broadcastDv32_jmmPS_(<32 x i32> %src, i64 %linear_local_id, i64 %linear_id, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z20work_group_broadcastDv64_jmmPS_(<64 x i32> %src, i64 %linear_local_id, i64 %linear_id, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z20work_group_broadcastlmmPl(i64 %src, i64 %linear_local_id, i64 %linear_id, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z20work_group_broadcastDv4_lmmPS_(<4 x i64> %src, i64 %linear_local_id, i64 %linear_id, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z20work_group_broadcastDv8_lmmPS_(<8 x i64> %src, i64 %linear_local_id, i64 %linear_id, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z20work_group_broadcastDv16_lmmPS_(<16 x i64> %src, i64 %linear_local_id, i64 %linear_id, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z20work_group_broadcastDv32_lmmPS_(<32 x i64> %src, i64 %linear_local_id, i64 %linear_id, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z20work_group_broadcastDv64_lmmPS_(<64 x i64> %src, i64 %linear_local_id, i64 %linear_id, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z20work_group_broadcastmmmPm(i64 %src, i64 %linear_local_id, i64 %linear_id, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z20work_group_broadcastDv4_mmmPS_(<4 x i64> %src, i64 %linear_local_id, i64 %linear_id, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z20work_group_broadcastDv8_mmmPS_(<8 x i64> %src, i64 %linear_local_id, i64 %linear_id, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z20work_group_broadcastDv16_mmmPS_(<16 x i64> %src, i64 %linear_local_id, i64 %linear_id, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z20work_group_broadcastDv32_mmmPS_(<32 x i64> %src, i64 %linear_local_id, i64 %linear_id, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z20work_group_broadcastDv64_mmmPS_(<64 x i64> %src, i64 %linear_local_id, i64 %linear_id, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z20work_group_broadcastfmmPf(float %src, i64 %linear_local_id, i64 %linear_id, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z20work_group_broadcastDv4_fmmPS_(<4 x float> %src, i64 %linear_local_id, i64 %linear_id, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z20work_group_broadcastDv8_fmmPS_(<8 x float> %src, i64 %linear_local_id, i64 %linear_id, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z20work_group_broadcastDv16_fmmPS_(<16 x float> %src, i64 %linear_local_id, i64 %linear_id, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z20work_group_broadcastDv32_fmmPS_(<32 x float> %src, i64 %linear_local_id, i64 %linear_id, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z20work_group_broadcastDv64_fmmPS_(<64 x float> %src, i64 %linear_local_id, i64 %linear_id, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z20work_group_broadcastdmmPd(double %src, i64 %linear_local_id, i64 %linear_id, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z20work_group_broadcastDv4_dmmPS_(<4 x double> %src, i64 %linear_local_id, i64 %linear_id, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z20work_group_broadcastDv8_dmmPS_(<8 x double> %src, i64 %linear_local_id, i64 %linear_id, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z20work_group_broadcastDv16_dmmPS_(<16 x double> %src, i64 %linear_local_id, i64 %linear_id, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z20work_group_broadcastDv32_dmmPS_(<32 x double> %src, i64 %linear_local_id, i64 %linear_id, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z20work_group_broadcastDv64_dmmPS_(<64 x double> %src, i64 %linear_local_id, i64 %linear_id, <64 x double>* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z20work_group_broadcastDv4_immDv4_jPS_(<4 x i32> %src, i64 %linear_local_id, i64 %linear_id, <4 x i32> %mask, <4 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_addi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_addDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_addDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_addDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_addDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_addDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_addj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_addDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_addDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_addDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_addDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_addDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_addl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_addDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_addDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_addDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_addDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_addDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_addm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_addDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_addDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_addDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_addDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_addDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z21work_group_reduce_addf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z21work_group_reduce_addDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z21work_group_reduce_addDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z21work_group_reduce_addDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z21work_group_reduce_addDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z21work_group_reduce_addDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z21work_group_reduce_addd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z21work_group_reduce_addDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z21work_group_reduce_addDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z21work_group_reduce_addDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z21work_group_reduce_addDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z21work_group_reduce_addDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_addDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_addDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_addDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_addDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_addDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_addDv4_j(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_addDv8_j(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_addDv16_j(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_addDv32_j(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_addDv64_j(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_addDv4_l(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_addDv8_l(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_addDv16_l(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_addDv32_l(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_addDv64_l(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_addDv4_m(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_addDv8_m(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_addDv16_m(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_addDv32_m(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_addDv64_m(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z32__finalize_work_group_reduce_addDv4_f(<4 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z32__finalize_work_group_reduce_addDv8_f(<8 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z32__finalize_work_group_reduce_addDv16_f(<16 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z32__finalize_work_group_reduce_addDv32_f(<32 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z32__finalize_work_group_reduce_addDv64_f(<64 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z32__finalize_work_group_reduce_addDv4_d(<4 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z32__finalize_work_group_reduce_addDv8_d(<8 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z32__finalize_work_group_reduce_addDv16_d(<16 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z32__finalize_work_group_reduce_addDv32_d(<32 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z32__finalize_work_group_reduce_addDv64_d(<64 x double> %accum) #3

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_addiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_addDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_addDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_addDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_addDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_addDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_addjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_addDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_addDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_addDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_addDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_addDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_addlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_addDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_addDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_addDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_addDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_addDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_addmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_addDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_addDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_addDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_addDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_addDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z21work_group_reduce_addfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z21work_group_reduce_addDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z21work_group_reduce_addDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z21work_group_reduce_addDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z21work_group_reduce_addDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z21work_group_reduce_addDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z21work_group_reduce_adddPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z21work_group_reduce_addDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z21work_group_reduce_addDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z21work_group_reduce_addDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z21work_group_reduce_addDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z21work_group_reduce_addDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_muli(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_mulDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_mulDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_mulDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_mulDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_mulDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_mulj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_mulDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_mulDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_mulDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_mulDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_mulDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_mull(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_mulDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_mulDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_mulDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_mulDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_mulDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_mulm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_mulDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_mulDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_mulDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_mulDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_mulDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z21work_group_reduce_mulf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z21work_group_reduce_mulDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z21work_group_reduce_mulDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z21work_group_reduce_mulDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z21work_group_reduce_mulDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z21work_group_reduce_mulDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z21work_group_reduce_muld(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z21work_group_reduce_mulDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z21work_group_reduce_mulDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z21work_group_reduce_mulDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z21work_group_reduce_mulDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z21work_group_reduce_mulDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_mulDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_mulDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_mulDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_mulDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_mulDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_mulDv4_j(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_mulDv8_j(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_mulDv16_j(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_mulDv32_j(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_mulDv64_j(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_mulDv4_l(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_mulDv8_l(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_mulDv16_l(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_mulDv32_l(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_mulDv64_l(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_mulDv4_m(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_mulDv8_m(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_mulDv16_m(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_mulDv32_m(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_mulDv64_m(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z32__finalize_work_group_reduce_mulDv4_f(<4 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z32__finalize_work_group_reduce_mulDv8_f(<8 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z32__finalize_work_group_reduce_mulDv16_f(<16 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z32__finalize_work_group_reduce_mulDv32_f(<32 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z32__finalize_work_group_reduce_mulDv64_f(<64 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z32__finalize_work_group_reduce_mulDv4_d(<4 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z32__finalize_work_group_reduce_mulDv8_d(<8 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z32__finalize_work_group_reduce_mulDv16_d(<16 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z32__finalize_work_group_reduce_mulDv32_d(<32 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z32__finalize_work_group_reduce_mulDv64_d(<64 x double> %accum) #3

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_muliPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_mulDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_mulDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_mulDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_mulDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_mulDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_muljPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_mulDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_mulDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_mulDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_mulDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_mulDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_mullPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_mulDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_mulDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_mulDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_mulDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_mulDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_mulmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_mulDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_mulDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_mulDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_mulDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_mulDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z21work_group_reduce_mulfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z21work_group_reduce_mulDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z21work_group_reduce_mulDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z21work_group_reduce_mulDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z21work_group_reduce_mulDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z21work_group_reduce_mulDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z21work_group_reduce_muldPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z21work_group_reduce_mulDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z21work_group_reduce_mulDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z21work_group_reduce_mulDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z21work_group_reduce_mulDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z21work_group_reduce_mulDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_maxi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_maxDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_maxDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_maxDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_maxDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_maxDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_maxj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_maxDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_maxDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_maxDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_maxDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_maxDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_maxl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_maxDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_maxDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_maxDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_maxDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_maxDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_maxm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_maxDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_maxDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_maxDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_maxDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_maxDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z21work_group_reduce_maxf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z21work_group_reduce_maxDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z21work_group_reduce_maxDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z21work_group_reduce_maxDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z21work_group_reduce_maxDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z21work_group_reduce_maxDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z21work_group_reduce_maxd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z21work_group_reduce_maxDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z21work_group_reduce_maxDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z21work_group_reduce_maxDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z21work_group_reduce_maxDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z21work_group_reduce_maxDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_maxDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_maxDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_maxDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_maxDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_maxDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_maxDv4_j(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_maxDv8_j(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_maxDv16_j(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_maxDv32_j(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_maxDv64_j(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_maxDv4_l(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_maxDv8_l(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_maxDv16_l(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_maxDv32_l(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_maxDv64_l(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_maxDv4_m(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_maxDv8_m(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_maxDv16_m(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_maxDv32_m(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_maxDv64_m(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z32__finalize_work_group_reduce_maxDv4_f(<4 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z32__finalize_work_group_reduce_maxDv8_f(<8 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z32__finalize_work_group_reduce_maxDv16_f(<16 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z32__finalize_work_group_reduce_maxDv32_f(<32 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z32__finalize_work_group_reduce_maxDv64_f(<64 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z32__finalize_work_group_reduce_maxDv4_d(<4 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z32__finalize_work_group_reduce_maxDv8_d(<8 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z32__finalize_work_group_reduce_maxDv16_d(<16 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z32__finalize_work_group_reduce_maxDv32_d(<32 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z32__finalize_work_group_reduce_maxDv64_d(<64 x double> %accum) #3

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_maxiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_maxDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_maxDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_maxDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_maxDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_maxDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_maxjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_maxDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_maxDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_maxDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_maxDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_maxDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_maxlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_maxDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_maxDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_maxDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_maxDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_maxDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_maxmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_maxDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_maxDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_maxDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_maxDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_maxDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z21work_group_reduce_maxfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z21work_group_reduce_maxDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z21work_group_reduce_maxDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z21work_group_reduce_maxDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z21work_group_reduce_maxDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z21work_group_reduce_maxDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z21work_group_reduce_maxdPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z21work_group_reduce_maxDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z21work_group_reduce_maxDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z21work_group_reduce_maxDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z21work_group_reduce_maxDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z21work_group_reduce_maxDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_mini(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_minDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_minDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_minDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_minDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_minDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_minj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_minDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_minDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_minDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_minDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_minDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_minl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_minDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_minDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_minDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_minDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_minDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_minm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_minDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_minDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_minDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_minDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_minDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z21work_group_reduce_minf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z21work_group_reduce_minDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z21work_group_reduce_minDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z21work_group_reduce_minDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z21work_group_reduce_minDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z21work_group_reduce_minDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z21work_group_reduce_mind(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z21work_group_reduce_minDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z21work_group_reduce_minDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z21work_group_reduce_minDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z21work_group_reduce_minDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z21work_group_reduce_minDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_minDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_minDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_minDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_minDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_minDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_minDv4_j(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_minDv8_j(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_minDv16_j(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_minDv32_j(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_minDv64_j(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_minDv4_l(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_minDv8_l(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_minDv16_l(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_minDv32_l(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_minDv64_l(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_minDv4_m(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_minDv8_m(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_minDv16_m(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_minDv32_m(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_minDv64_m(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z32__finalize_work_group_reduce_minDv4_f(<4 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z32__finalize_work_group_reduce_minDv8_f(<8 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z32__finalize_work_group_reduce_minDv16_f(<16 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z32__finalize_work_group_reduce_minDv32_f(<32 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z32__finalize_work_group_reduce_minDv64_f(<64 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z32__finalize_work_group_reduce_minDv4_d(<4 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z32__finalize_work_group_reduce_minDv8_d(<8 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z32__finalize_work_group_reduce_minDv16_d(<16 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z32__finalize_work_group_reduce_minDv32_d(<32 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z32__finalize_work_group_reduce_minDv64_d(<64 x double> %accum) #3

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_miniPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_minDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_minDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_minDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_minDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_minDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_minjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_minDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_minDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_minDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_minDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_minDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_minlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_minDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_minDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_minDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_minDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_minDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_minmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_minDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_minDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_minDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_minDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_minDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z21work_group_reduce_minfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z21work_group_reduce_minDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z21work_group_reduce_minDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z21work_group_reduce_minDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z21work_group_reduce_minDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z21work_group_reduce_minDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z21work_group_reduce_mindPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z21work_group_reduce_minDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z21work_group_reduce_minDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z21work_group_reduce_minDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z21work_group_reduce_minDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z21work_group_reduce_minDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare signext i8 @_Z29work_group_reduce_bitwise_andc(i8 signext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z29work_group_reduce_bitwise_andDv4_c(<4 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z29work_group_reduce_bitwise_andDv8_c(<8 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z29work_group_reduce_bitwise_andDv16_c(<16 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z29work_group_reduce_bitwise_andDv32_c(<32 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z29work_group_reduce_bitwise_andDv64_c(<64 x i8> %src) #3

; Function Attrs: nounwind readnone
declare zeroext i8 @_Z29work_group_reduce_bitwise_andh(i8 zeroext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z29work_group_reduce_bitwise_andDv4_h(<4 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z29work_group_reduce_bitwise_andDv8_h(<8 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z29work_group_reduce_bitwise_andDv16_h(<16 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z29work_group_reduce_bitwise_andDv32_h(<32 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z29work_group_reduce_bitwise_andDv64_h(<64 x i8> %src) #3

; Function Attrs: nounwind readnone
declare signext i16 @_Z29work_group_reduce_bitwise_ands(i16 signext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z29work_group_reduce_bitwise_andDv4_s(<4 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z29work_group_reduce_bitwise_andDv8_s(<8 x i16> %src) #3

; Function Attrs: nounwind readnone
declare signext <16 x i16> @_Z29work_group_reduce_bitwise_andDv16_s(<16 x i16> signext %src) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z29work_group_reduce_bitwise_andDv32_s(<32 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z29work_group_reduce_bitwise_andDv64_s(<64 x i16> %src) #3

; Function Attrs: nounwind readnone
declare zeroext i16 @_Z29work_group_reduce_bitwise_andt(i16 zeroext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z29work_group_reduce_bitwise_andDv4_t(<4 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z29work_group_reduce_bitwise_andDv8_t(<8 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z29work_group_reduce_bitwise_andDv16_t(<16 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z29work_group_reduce_bitwise_andDv32_t(<32 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z29work_group_reduce_bitwise_andDv64_t(<64 x i16> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_reduce_bitwise_andi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_reduce_bitwise_andDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_reduce_bitwise_andDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_reduce_bitwise_andDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_reduce_bitwise_andDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_reduce_bitwise_andDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_reduce_bitwise_andj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_reduce_bitwise_andDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_reduce_bitwise_andDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_reduce_bitwise_andDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_reduce_bitwise_andDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_reduce_bitwise_andDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_reduce_bitwise_andl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_reduce_bitwise_andDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_reduce_bitwise_andDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_reduce_bitwise_andDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_reduce_bitwise_andDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_reduce_bitwise_andDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_reduce_bitwise_andm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_reduce_bitwise_andDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_reduce_bitwise_andDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_reduce_bitwise_andDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_reduce_bitwise_andDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_reduce_bitwise_andDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind
declare signext i8 @_Z29work_group_reduce_bitwise_andcPc(i8 signext %src, i8* %accum) #5

; Function Attrs: nounwind
declare <4 x i8> @_Z29work_group_reduce_bitwise_andDv4_cPS_(<4 x i8> %src, <4 x i8>* %accum) #5

; Function Attrs: nounwind
declare <8 x i8> @_Z29work_group_reduce_bitwise_andDv8_cPS_(<8 x i8> %src, <8 x i8>* %accum) #5

; Function Attrs: nounwind
declare <16 x i8> @_Z29work_group_reduce_bitwise_andDv16_cPS_(<16 x i8> %src, <16 x i8>* %accum) #5

; Function Attrs: nounwind
declare <32 x i8> @_Z29work_group_reduce_bitwise_andDv32_cPS_(<32 x i8> %src, <32 x i8>* %accum) #5

; Function Attrs: nounwind
declare <64 x i8> @_Z29work_group_reduce_bitwise_andDv64_cPS_(<64 x i8> %src, <64 x i8>* %accum) #5

; Function Attrs: nounwind
declare zeroext i8 @_Z29work_group_reduce_bitwise_andhPh(i8 zeroext %src, i8* %accum) #5

; Function Attrs: nounwind
declare <4 x i8> @_Z29work_group_reduce_bitwise_andDv4_hPS_(<4 x i8> %src, <4 x i8>* %accum) #5

; Function Attrs: nounwind
declare <8 x i8> @_Z29work_group_reduce_bitwise_andDv8_hPS_(<8 x i8> %src, <8 x i8>* %accum) #5

; Function Attrs: nounwind
declare <16 x i8> @_Z29work_group_reduce_bitwise_andDv16_hPS_(<16 x i8> %src, <16 x i8>* %accum) #5
 
; Function Attrs: nounwind
declare <32 x i8> @_Z29work_group_reduce_bitwise_andDv32_hPS_(<32 x i8> %src, <32 x i8>* %accum) #5

; Function Attrs: nounwind
declare <64 x i8> @_Z29work_group_reduce_bitwise_andDv64_hPS_(<64 x i8> %src, <64 x i8>* %accum) #5

; Function Attrs: nounwind
declare signext i16 @_Z29work_group_reduce_bitwise_andsPs(i16 signext %src, i16* %accum) #5

; Function Attrs: nounwind
declare <4 x i16> @_Z29work_group_reduce_bitwise_andDv4_sPS_(<4 x i16> %src, <4 x i16>* %accum) #5

; Function Attrs: nounwind
declare <8 x i16> @_Z29work_group_reduce_bitwise_andDv8_sPS_(<8 x i16> %src, <8 x i16>* %accum) #5
 
; Function Attrs: nounwind
declare <16 x i16> @_Z29work_group_reduce_bitwise_andDv16_sPS_(<16 x i16> %src, <16 x i16>* %accum) #5

; Function Attrs: nounwind
declare <32 x i16> @_Z29work_group_reduce_bitwise_andDv32_sPS_(<32 x i16> %src, <32 x i16>* %accum) #5

; Function Attrs: nounwind
declare <64 x i16> @_Z29work_group_reduce_bitwise_andDv64_sPS_(<64 x i16> %src, <64 x i16>* %accum) #5

; Function Attrs: nounwind
declare zeroext i16 @_Z29work_group_reduce_bitwise_andtPt(i16 zeroext %src, i16* %accum) #5

; Function Attrs: nounwind
declare <4 x i16> @_Z29work_group_reduce_bitwise_andDv4_tPS_(<4 x i16> %src, <4 x i16>* %accum) #5

; Function Attrs: nounwind
declare <8 x i16> @_Z29work_group_reduce_bitwise_andDv8_tPS_(<8 x i16> %src, <8 x i16>* %accum) #5
 
; Function Attrs: nounwind
declare <16 x i16> @_Z29work_group_reduce_bitwise_andDv16_tPS_(<16 x i16> %src, <16 x i16>* %accum) #5

; Function Attrs: nounwind
declare <32 x i16> @_Z29work_group_reduce_bitwise_andDv32_tPS_(<32 x i16> %src, <32 x i16>* %accum) #5

; Function Attrs: nounwind
declare <64 x i16> @_Z29work_group_reduce_bitwise_andDv64_tPS_(<64 x i16> %src, <64 x i16>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_reduce_bitwise_andiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_reduce_bitwise_andDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5
 
; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_reduce_bitwise_andDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_reduce_bitwise_andDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_reduce_bitwise_andDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_reduce_bitwise_andDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_reduce_bitwise_andjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_reduce_bitwise_andDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5
 
; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_reduce_bitwise_andDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_reduce_bitwise_andDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_reduce_bitwise_andDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_reduce_bitwise_andDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_reduce_bitwise_andlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_reduce_bitwise_andDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_reduce_bitwise_andDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_reduce_bitwise_andDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_reduce_bitwise_andDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_reduce_bitwise_andDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_reduce_bitwise_andmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_reduce_bitwise_andDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_reduce_bitwise_andDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_reduce_bitwise_andDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_reduce_bitwise_andDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_reduce_bitwise_andDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind readnone
declare signext i8 @_Z28work_group_reduce_bitwise_orc(i8 signext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z28work_group_reduce_bitwise_orDv4_c(<4 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z28work_group_reduce_bitwise_orDv8_c(<8 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z28work_group_reduce_bitwise_orDv16_c(<16 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z28work_group_reduce_bitwise_orDv32_c(<32 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z28work_group_reduce_bitwise_orDv64_c(<64 x i8> %src) #3

; Function Attrs: nounwind readnone
declare zeroext i8 @_Z28work_group_reduce_bitwise_orh(i8 zeroext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z28work_group_reduce_bitwise_orDv4_h(<4 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z28work_group_reduce_bitwise_orDv8_h(<8 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z28work_group_reduce_bitwise_orDv16_h(<16 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z28work_group_reduce_bitwise_orDv32_h(<32 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z28work_group_reduce_bitwise_orDv64_h(<64 x i8> %src) #3

; Function Attrs: nounwind readnone
declare signext i16 @_Z28work_group_reduce_bitwise_ors(i16 signext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z28work_group_reduce_bitwise_orDv4_s(<4 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z28work_group_reduce_bitwise_orDv8_s(<8 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z28work_group_reduce_bitwise_orDv16_s(<16 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z28work_group_reduce_bitwise_orDv32_s(<32 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z28work_group_reduce_bitwise_orDv64_s(<64 x i16> %src) #3

; Function Attrs: nounwind readnone
declare zeroext i16 @_Z28work_group_reduce_bitwise_ort(i16 zeroext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z28work_group_reduce_bitwise_orDv4_t(<4 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z28work_group_reduce_bitwise_orDv8_t(<8 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z28work_group_reduce_bitwise_orDv16_t(<16 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z28work_group_reduce_bitwise_orDv32_t(<32 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z28work_group_reduce_bitwise_orDv64_t(<64 x i16> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z28work_group_reduce_bitwise_ori(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z28work_group_reduce_bitwise_orDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z28work_group_reduce_bitwise_orDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z28work_group_reduce_bitwise_orDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z28work_group_reduce_bitwise_orDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z28work_group_reduce_bitwise_orDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z28work_group_reduce_bitwise_orj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z28work_group_reduce_bitwise_orDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z28work_group_reduce_bitwise_orDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z28work_group_reduce_bitwise_orDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z28work_group_reduce_bitwise_orDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z28work_group_reduce_bitwise_orDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z28work_group_reduce_bitwise_orl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z28work_group_reduce_bitwise_orDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z28work_group_reduce_bitwise_orDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z28work_group_reduce_bitwise_orDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z28work_group_reduce_bitwise_orDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z28work_group_reduce_bitwise_orDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z28work_group_reduce_bitwise_orm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z28work_group_reduce_bitwise_orDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z28work_group_reduce_bitwise_orDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z28work_group_reduce_bitwise_orDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z28work_group_reduce_bitwise_orDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z28work_group_reduce_bitwise_orDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind
declare signext i8 @_Z28work_group_reduce_bitwise_orcPc(i8 signext %src, i8* %accum) #5

; Function Attrs: nounwind
declare <4 x i8> @_Z28work_group_reduce_bitwise_orDv4_cPS_(<4 x i8> %src, <4 x i8>* %accum) #5

; Function Attrs: nounwind
declare <8 x i8> @_Z28work_group_reduce_bitwise_orDv8_cPS_(<8 x i8> %src, <8 x i8>* %accum) #5

; Function Attrs: nounwind
declare <16 x i8> @_Z28work_group_reduce_bitwise_orDv16_cPS_(<16 x i8> %src, <16 x i8>* %accum) #5
 
; Function Attrs: nounwind
declare <32 x i8> @_Z28work_group_reduce_bitwise_orDv32_cPS_(<32 x i8> %src, <32 x i8>* %accum) #5

; Function Attrs: nounwind
declare <64 x i8> @_Z28work_group_reduce_bitwise_orDv64_cPS_(<64 x i8> %src, <64 x i8>* %accum) #5

; Function Attrs: nounwind
declare zeroext i8 @_Z28work_group_reduce_bitwise_orhPh(i8 zeroext %src, i8* %accum) #5

; Function Attrs: nounwind
declare <4 x i8> @_Z28work_group_reduce_bitwise_orDv4_hPS_(<4 x i8> %src, <4 x i8>* %accum) #5

; Function Attrs: nounwind
declare <8 x i8> @_Z28work_group_reduce_bitwise_orDv8_hPS_(<8 x i8> %src, <8 x i8>* %accum) #5

; Function Attrs: nounwind
declare <16 x i8> @_Z28work_group_reduce_bitwise_orDv16_hPS_(<16 x i8> %src, <16 x i8>* %accum) #5
 
; Function Attrs: nounwind
declare <32 x i8> @_Z28work_group_reduce_bitwise_orDv32_hPS_(<32 x i8> %src, <32 x i8>* %accum) #5

; Function Attrs: nounwind
declare <64 x i8> @_Z28work_group_reduce_bitwise_orDv64_hPS_(<64 x i8> %src, <64 x i8>* %accum) #5

; Function Attrs: nounwind
declare signext i16 @_Z28work_group_reduce_bitwise_orsPs(i16 signext %src, i16* %accum) #5

; Function Attrs: nounwind
declare <4 x i16> @_Z28work_group_reduce_bitwise_orDv4_sPS_(<4 x i16> %src, <4 x i16>* %accum) #5

; Function Attrs: nounwind
declare <8 x i16> @_Z28work_group_reduce_bitwise_orDv8_sPS_(<8 x i16> %src, <8 x i16>* %accum) #5
 
; Function Attrs: nounwind
declare <16 x i16> @_Z28work_group_reduce_bitwise_orDv16_sPS_(<16 x i16> %src, <16 x i16>* %accum) #5

; Function Attrs: nounwind
declare <32 x i16> @_Z28work_group_reduce_bitwise_orDv32_sPS_(<32 x i16> %src, <32 x i16>* %accum) #5

; Function Attrs: nounwind
declare <64 x i16> @_Z28work_group_reduce_bitwise_orDv64_sPS_(<64 x i16> %src, <64 x i16>* %accum) #5

; Function Attrs: nounwind
declare zeroext i16 @_Z28work_group_reduce_bitwise_ortPt(i16 zeroext %src, i16* %accum) #5

; Function Attrs: nounwind
declare <4 x i16> @_Z28work_group_reduce_bitwise_orDv4_tPS_(<4 x i16> %src, <4 x i16>* %accum) #5

; Function Attrs: nounwind
declare <8 x i16> @_Z28work_group_reduce_bitwise_orDv8_tPS_(<8 x i16> %src, <8 x i16>* %accum) #5
 
; Function Attrs: nounwind
declare <16 x i16> @_Z28work_group_reduce_bitwise_orDv16_tPS_(<16 x i16> %src, <16 x i16>* %accum) #5

; Function Attrs: nounwind
declare <32 x i16> @_Z28work_group_reduce_bitwise_orDv32_tPS_(<32 x i16> %src, <32 x i16>* %accum) #5

; Function Attrs: nounwind
declare <64 x i16> @_Z28work_group_reduce_bitwise_orDv64_tPS_(<64 x i16> %src, <64 x i16>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z28work_group_reduce_bitwise_oriPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z28work_group_reduce_bitwise_orDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5
 
; Function Attrs: nounwind
declare <8 x i32> @_Z28work_group_reduce_bitwise_orDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z28work_group_reduce_bitwise_orDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z28work_group_reduce_bitwise_orDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z28work_group_reduce_bitwise_orDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z28work_group_reduce_bitwise_orjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z28work_group_reduce_bitwise_orDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5
 
; Function Attrs: nounwind
declare <8 x i32> @_Z28work_group_reduce_bitwise_orDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z28work_group_reduce_bitwise_orDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z28work_group_reduce_bitwise_orDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z28work_group_reduce_bitwise_orDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z28work_group_reduce_bitwise_orlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z28work_group_reduce_bitwise_orDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z28work_group_reduce_bitwise_orDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z28work_group_reduce_bitwise_orDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z28work_group_reduce_bitwise_orDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z28work_group_reduce_bitwise_orDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z28work_group_reduce_bitwise_ormPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z28work_group_reduce_bitwise_orDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z28work_group_reduce_bitwise_orDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z28work_group_reduce_bitwise_orDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z28work_group_reduce_bitwise_orDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z28work_group_reduce_bitwise_orDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind readnone
declare signext i8 @_Z29work_group_reduce_bitwise_xorc(i8 signext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z29work_group_reduce_bitwise_xorDv4_c(<4 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z29work_group_reduce_bitwise_xorDv8_c(<8 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z29work_group_reduce_bitwise_xorDv16_c(<16 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z29work_group_reduce_bitwise_xorDv32_c(<32 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z29work_group_reduce_bitwise_xorDv64_c(<64 x i8> %src) #3

; Function Attrs: nounwind readnone
declare zeroext i8 @_Z29work_group_reduce_bitwise_xorh(i8 zeroext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z29work_group_reduce_bitwise_xorDv4_h(<4 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z29work_group_reduce_bitwise_xorDv8_h(<8 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z29work_group_reduce_bitwise_xorDv16_h(<16 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z29work_group_reduce_bitwise_xorDv32_h(<32 x i8> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z29work_group_reduce_bitwise_xorDv64_h(<64 x i8> %src) #3

; Function Attrs: nounwind readnone
declare signext i16 @_Z29work_group_reduce_bitwise_xors(i16 signext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z29work_group_reduce_bitwise_xorDv4_s(<4 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z29work_group_reduce_bitwise_xorDv8_s(<8 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z29work_group_reduce_bitwise_xorDv16_s(<16 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z29work_group_reduce_bitwise_xorDv32_s(<32 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z29work_group_reduce_bitwise_xorDv64_s(<64 x i16> %src) #3

; Function Attrs: nounwind readnone
declare zeroext i16 @_Z29work_group_reduce_bitwise_xort(i16 zeroext %src) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z29work_group_reduce_bitwise_xorDv4_t(<4 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z29work_group_reduce_bitwise_xorDv8_t(<8 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z29work_group_reduce_bitwise_xorDv16_t(<16 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z29work_group_reduce_bitwise_xorDv32_t(<32 x i16> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z29work_group_reduce_bitwise_xorDv64_t(<64 x i16> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_reduce_bitwise_xori(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_reduce_bitwise_xorDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_reduce_bitwise_xorDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_reduce_bitwise_xorDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_reduce_bitwise_xorDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_reduce_bitwise_xorDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_reduce_bitwise_xorj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_reduce_bitwise_xorDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_reduce_bitwise_xorDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_reduce_bitwise_xorDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_reduce_bitwise_xorDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_reduce_bitwise_xorDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_reduce_bitwise_xorl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_reduce_bitwise_xorDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_reduce_bitwise_xorDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_reduce_bitwise_xorDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_reduce_bitwise_xorDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_reduce_bitwise_xorDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_reduce_bitwise_xorm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_reduce_bitwise_xorDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_reduce_bitwise_xorDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_reduce_bitwise_xorDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_reduce_bitwise_xorDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_reduce_bitwise_xorDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind
declare signext i8 @_Z29work_group_reduce_bitwise_xorcPc(i8 signext %src, i8* %accum) #5

; Function Attrs: nounwind
declare <4 x i8> @_Z29work_group_reduce_bitwise_xorDv4_cPS_(<4 x i8> %src, <4 x i8>* %accum) #5

; Function Attrs: nounwind
declare <8 x i8> @_Z29work_group_reduce_bitwise_xorDv8_cPS_(<8 x i8> %src, <8 x i8>* %accum) #5

; Function Attrs: nounwind
declare <16 x i8> @_Z29work_group_reduce_bitwise_xorDv16_cPS_(<16 x i8> %src, <16 x i8>* %accum) #5
 
; Function Attrs: nounwind
declare <32 x i8> @_Z29work_group_reduce_bitwise_xorDv32_cPS_(<32 x i8> %src, <32 x i8>* %accum) #5

; Function Attrs: nounwind
declare <64 x i8> @_Z29work_group_reduce_bitwise_xorDv64_cPS_(<64 x i8> %src, <64 x i8>* %accum) #5

; Function Attrs: nounwind
declare zeroext i8 @_Z29work_group_reduce_bitwise_xorhPh(i8 zeroext %src, i8* %accum) #5

; Function Attrs: nounwind
declare <4 x i8> @_Z29work_group_reduce_bitwise_xorDv4_hPS_(<4 x i8> %src, <4 x i8>* %accum) #5

; Function Attrs: nounwind
declare <8 x i8> @_Z29work_group_reduce_bitwise_xorDv8_hPS_(<8 x i8> %src, <8 x i8>* %accum) #5

; Function Attrs: nounwind
declare <16 x i8> @_Z29work_group_reduce_bitwise_xorDv16_hPS_(<16 x i8> %src, <16 x i8>* %accum) #5
 
; Function Attrs: nounwind
declare <32 x i8> @_Z29work_group_reduce_bitwise_xorDv32_hPS_(<32 x i8> %src, <32 x i8>* %accum) #5

; Function Attrs: nounwind
declare <64 x i8> @_Z29work_group_reduce_bitwise_xorDv64_hPS_(<64 x i8> %src, <64 x i8>* %accum) #5

; Function Attrs: nounwind
declare signext i16 @_Z29work_group_reduce_bitwise_xorsPs(i16 signext %src, i16* %accum) #5

; Function Attrs: nounwind
declare <4 x i16> @_Z29work_group_reduce_bitwise_xorDv4_sPS_(<4 x i16> %src, <4 x i16>* %accum) #5

; Function Attrs: nounwind
declare <8 x i16> @_Z29work_group_reduce_bitwise_xorDv8_sPS_(<8 x i16> %src, <8 x i16>* %accum) #5
 
; Function Attrs: nounwind
declare <16 x i16> @_Z29work_group_reduce_bitwise_xorDv16_sPS_(<16 x i16> %src, <16 x i16>* %accum) #5

; Function Attrs: nounwind
declare <32 x i16> @_Z29work_group_reduce_bitwise_xorDv32_sPS_(<32 x i16> %src, <32 x i16>* %accum) #5

; Function Attrs: nounwind
declare <64 x i16> @_Z29work_group_reduce_bitwise_xorDv64_sPS_(<64 x i16> %src, <64 x i16>* %accum) #5

; Function Attrs: nounwind
declare zeroext i16 @_Z29work_group_reduce_bitwise_xortPt(i16 zeroext %src, i16* %accum) #5

; Function Attrs: nounwind
declare <4 x i16> @_Z29work_group_reduce_bitwise_xorDv4_tPS_(<4 x i16> %src, <4 x i16>* %accum) #5

; Function Attrs: nounwind
declare <8 x i16> @_Z29work_group_reduce_bitwise_xorDv8_tPS_(<8 x i16> %src, <8 x i16>* %accum) #5
 
; Function Attrs: nounwind
declare <16 x i16> @_Z29work_group_reduce_bitwise_xorDv16_tPS_(<16 x i16> %src, <16 x i16>* %accum) #5

; Function Attrs: nounwind
declare <32 x i16> @_Z29work_group_reduce_bitwise_xorDv32_tPS_(<32 x i16> %src, <32 x i16>* %accum) #5

; Function Attrs: nounwind
declare <64 x i16> @_Z29work_group_reduce_bitwise_xorDv64_tPS_(<64 x i16> %src, <64 x i16>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_reduce_bitwise_xoriPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_reduce_bitwise_xorDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5
 
; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_reduce_bitwise_xorDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_reduce_bitwise_xorDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_reduce_bitwise_xorDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_reduce_bitwise_xorDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_reduce_bitwise_xorjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_reduce_bitwise_xorDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5
 
; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_reduce_bitwise_xorDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_reduce_bitwise_xorDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_reduce_bitwise_xorDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_reduce_bitwise_xorDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_reduce_bitwise_xorlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_reduce_bitwise_xorDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_reduce_bitwise_xorDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_reduce_bitwise_xorDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_reduce_bitwise_xorDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_reduce_bitwise_xorDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_reduce_bitwise_xormPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_reduce_bitwise_xorDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_reduce_bitwise_xorDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_reduce_bitwise_xorDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_reduce_bitwise_xorDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_reduce_bitwise_xorDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_reduce_logical_andi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_reduce_logical_andDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_reduce_logical_andDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_reduce_logical_andDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_reduce_logical_andDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_reduce_logical_andDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_reduce_logical_andiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_reduce_logical_andDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_reduce_logical_andDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_reduce_logical_andDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_reduce_logical_andDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_reduce_logical_andDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z28work_group_reduce_logical_ori(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z28work_group_reduce_logical_orDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z28work_group_reduce_logical_orDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z28work_group_reduce_logical_orDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z28work_group_reduce_logical_orDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z28work_group_reduce_logical_orDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind
declare i32 @_Z28work_group_reduce_logical_oriPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z28work_group_reduce_logical_orDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z28work_group_reduce_logical_orDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z28work_group_reduce_logical_orDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z28work_group_reduce_logical_orDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z28work_group_reduce_logical_orDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_reduce_logical_xori(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_reduce_logical_xorDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_reduce_logical_xorDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_reduce_logical_xorDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_reduce_logical_xorDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_reduce_logical_xorDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_reduce_logical_xoriPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_reduce_logical_xorDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_reduce_logical_xorDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_reduce_logical_xorDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_reduce_logical_xorDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_reduce_logical_xorDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z40__finalize_work_group_reduce_bitwise_andDv4_c(<4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z40__finalize_work_group_reduce_bitwise_andDv8_c(<8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z40__finalize_work_group_reduce_bitwise_andDv16_c(<16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z40__finalize_work_group_reduce_bitwise_andDv32_c(<32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z40__finalize_work_group_reduce_bitwise_andDv64_c(<64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z40__finalize_work_group_reduce_bitwise_andDv4_h(<4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z40__finalize_work_group_reduce_bitwise_andDv8_h(<8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z40__finalize_work_group_reduce_bitwise_andDv16_h(<16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z40__finalize_work_group_reduce_bitwise_andDv32_h(<32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z40__finalize_work_group_reduce_bitwise_andDv64_h(<64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z40__finalize_work_group_reduce_bitwise_andDv4_s(<4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z40__finalize_work_group_reduce_bitwise_andDv8_s(<8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z40__finalize_work_group_reduce_bitwise_andDv16_s(<16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z40__finalize_work_group_reduce_bitwise_andDv32_s(<32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z40__finalize_work_group_reduce_bitwise_andDv64_s(<64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z40__finalize_work_group_reduce_bitwise_andDv4_t(<4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z40__finalize_work_group_reduce_bitwise_andDv8_t(<8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z40__finalize_work_group_reduce_bitwise_andDv16_t(<16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z40__finalize_work_group_reduce_bitwise_andDv32_t(<32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z40__finalize_work_group_reduce_bitwise_andDv64_t(<64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z40__finalize_work_group_reduce_bitwise_andDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z40__finalize_work_group_reduce_bitwise_andDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z40__finalize_work_group_reduce_bitwise_andDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z40__finalize_work_group_reduce_bitwise_andDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z40__finalize_work_group_reduce_bitwise_andDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z40__finalize_work_group_reduce_bitwise_andDv4_j(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z40__finalize_work_group_reduce_bitwise_andDv8_j(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z40__finalize_work_group_reduce_bitwise_andDv16_j(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z40__finalize_work_group_reduce_bitwise_andDv32_j(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z40__finalize_work_group_reduce_bitwise_andDv64_j(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z40__finalize_work_group_reduce_bitwise_andDv4_l(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z40__finalize_work_group_reduce_bitwise_andDv8_l(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z40__finalize_work_group_reduce_bitwise_andDv16_l(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z40__finalize_work_group_reduce_bitwise_andDv32_l(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z40__finalize_work_group_reduce_bitwise_andDv64_l(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z40__finalize_work_group_reduce_bitwise_andDv4_m(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z40__finalize_work_group_reduce_bitwise_andDv8_m(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z40__finalize_work_group_reduce_bitwise_andDv16_m(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z40__finalize_work_group_reduce_bitwise_andDv32_m(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z40__finalize_work_group_reduce_bitwise_andDv64_m(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z39__finalize_work_group_reduce_bitwise_orDv4_c(<4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z39__finalize_work_group_reduce_bitwise_orDv8_c(<8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z39__finalize_work_group_reduce_bitwise_orDv16_c(<16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z39__finalize_work_group_reduce_bitwise_orDv32_c(<32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z39__finalize_work_group_reduce_bitwise_orDv64_c(<64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z39__finalize_work_group_reduce_bitwise_orDv4_h(<4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z39__finalize_work_group_reduce_bitwise_orDv8_h(<8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z39__finalize_work_group_reduce_bitwise_orDv16_h(<16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z39__finalize_work_group_reduce_bitwise_orDv32_h(<32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z39__finalize_work_group_reduce_bitwise_orDv64_h(<64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z39__finalize_work_group_reduce_bitwise_orDv4_s(<4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z39__finalize_work_group_reduce_bitwise_orDv8_s(<8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z39__finalize_work_group_reduce_bitwise_orDv16_s(<16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z39__finalize_work_group_reduce_bitwise_orDv32_s(<32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z39__finalize_work_group_reduce_bitwise_orDv64_s(<64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z39__finalize_work_group_reduce_bitwise_orDv4_t(<4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z39__finalize_work_group_reduce_bitwise_orDv8_t(<8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z39__finalize_work_group_reduce_bitwise_orDv16_t(<16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z39__finalize_work_group_reduce_bitwise_orDv32_t(<32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z39__finalize_work_group_reduce_bitwise_orDv64_t(<64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z39__finalize_work_group_reduce_bitwise_orDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z39__finalize_work_group_reduce_bitwise_orDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z39__finalize_work_group_reduce_bitwise_orDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z39__finalize_work_group_reduce_bitwise_orDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z39__finalize_work_group_reduce_bitwise_orDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z39__finalize_work_group_reduce_bitwise_orDv4_j(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z39__finalize_work_group_reduce_bitwise_orDv8_j(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z39__finalize_work_group_reduce_bitwise_orDv16_j(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z39__finalize_work_group_reduce_bitwise_orDv32_j(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z39__finalize_work_group_reduce_bitwise_orDv64_j(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z39__finalize_work_group_reduce_bitwise_orDv4_l(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z39__finalize_work_group_reduce_bitwise_orDv8_l(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z39__finalize_work_group_reduce_bitwise_orDv16_l(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z39__finalize_work_group_reduce_bitwise_orDv32_l(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z39__finalize_work_group_reduce_bitwise_orDv64_l(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z39__finalize_work_group_reduce_bitwise_orDv4_m(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z39__finalize_work_group_reduce_bitwise_orDv8_m(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z39__finalize_work_group_reduce_bitwise_orDv16_m(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z39__finalize_work_group_reduce_bitwise_orDv32_m(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z39__finalize_work_group_reduce_bitwise_orDv64_m(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z40__finalize_work_group_reduce_bitwise_xorDv4_c(<4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z40__finalize_work_group_reduce_bitwise_xorDv8_c(<8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z40__finalize_work_group_reduce_bitwise_xorDv16_c(<16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z40__finalize_work_group_reduce_bitwise_xorDv32_c(<32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z40__finalize_work_group_reduce_bitwise_xorDv64_c(<64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i8> @_Z40__finalize_work_group_reduce_bitwise_xorDv4_h(<4 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i8> @_Z40__finalize_work_group_reduce_bitwise_xorDv8_h(<8 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i8> @_Z40__finalize_work_group_reduce_bitwise_xorDv16_h(<16 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i8> @_Z40__finalize_work_group_reduce_bitwise_xorDv32_h(<32 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i8> @_Z40__finalize_work_group_reduce_bitwise_xorDv64_h(<64 x i8> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z40__finalize_work_group_reduce_bitwise_xorDv4_s(<4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z40__finalize_work_group_reduce_bitwise_xorDv8_s(<8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z40__finalize_work_group_reduce_bitwise_xorDv16_s(<16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z40__finalize_work_group_reduce_bitwise_xorDv32_s(<32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z40__finalize_work_group_reduce_bitwise_xorDv64_s(<64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i16> @_Z40__finalize_work_group_reduce_bitwise_xorDv4_t(<4 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i16> @_Z40__finalize_work_group_reduce_bitwise_xorDv8_t(<8 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i16> @_Z40__finalize_work_group_reduce_bitwise_xorDv16_t(<16 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i16> @_Z40__finalize_work_group_reduce_bitwise_xorDv32_t(<32 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i16> @_Z40__finalize_work_group_reduce_bitwise_xorDv64_t(<64 x i16> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z40__finalize_work_group_reduce_bitwise_xorDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z40__finalize_work_group_reduce_bitwise_xorDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z40__finalize_work_group_reduce_bitwise_xorDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z40__finalize_work_group_reduce_bitwise_xorDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z40__finalize_work_group_reduce_bitwise_xorDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z40__finalize_work_group_reduce_bitwise_xorDv4_j(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z40__finalize_work_group_reduce_bitwise_xorDv8_j(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z40__finalize_work_group_reduce_bitwise_xorDv16_j(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z40__finalize_work_group_reduce_bitwise_xorDv32_j(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z40__finalize_work_group_reduce_bitwise_xorDv64_j(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z40__finalize_work_group_reduce_bitwise_xorDv4_l(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z40__finalize_work_group_reduce_bitwise_xorDv8_l(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z40__finalize_work_group_reduce_bitwise_xorDv16_l(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z40__finalize_work_group_reduce_bitwise_xorDv32_l(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z40__finalize_work_group_reduce_bitwise_xorDv64_l(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z40__finalize_work_group_reduce_bitwise_xorDv4_m(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z40__finalize_work_group_reduce_bitwise_xorDv8_m(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z40__finalize_work_group_reduce_bitwise_xorDv16_m(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z40__finalize_work_group_reduce_bitwise_xorDv32_m(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z40__finalize_work_group_reduce_bitwise_xorDv64_m(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare<4 x i32> @_Z40__finalize_work_group_reduce_logical_andDv4_i(<4 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<8 x i32> @_Z40__finalize_work_group_reduce_logical_andDv8_i(<8 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<16 x i32> @_Z40__finalize_work_group_reduce_logical_andDv16_i(<16 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<32 x i32> @_Z40__finalize_work_group_reduce_logical_andDv32_i(<32 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<64 x i32> @_Z40__finalize_work_group_reduce_logical_andDv64_i(<64 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<4 x i32> @_Z39__finalize_work_group_reduce_logical_orDv4_i(<4 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<8 x i32> @_Z39__finalize_work_group_reduce_logical_orDv8_i(<8 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<16 x i32> @_Z39__finalize_work_group_reduce_logical_orDv16_i(<16 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<32 x i32> @_Z39__finalize_work_group_reduce_logical_orDv32_i(<32 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<64 x i32> @_Z39__finalize_work_group_reduce_logical_orDv64_i(<64 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<4 x i32> @_Z40__finalize_work_group_reduce_logical_xorDv4_i(<4 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<8 x i32> @_Z40__finalize_work_group_reduce_logical_xorDv8_i(<8 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<16 x i32> @_Z40__finalize_work_group_reduce_logical_xorDv16_i(<16 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<32 x i32> @_Z40__finalize_work_group_reduce_logical_xorDv32_i(<32 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare<64 x i32> @_Z40__finalize_work_group_reduce_logical_xorDv64_i(<64 x i32> noundef %accum) #3 

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_addi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_addDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_addDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_addDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_addDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_addDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_addj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_addDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_addDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_addDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_addDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_addDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_addl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_addDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_addDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_addDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_addDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_addDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_addm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_addDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_addDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_addDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_addDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_addDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_exclusive_addf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_exclusive_addDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_exclusive_addDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_exclusive_addDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_exclusive_addDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_exclusive_addDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_exclusive_addd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_exclusive_addDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_exclusive_addDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_exclusive_addDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_exclusive_addDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_exclusive_addDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_addiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_addDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_addDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_addDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_addDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_addDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_addjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_addDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_addDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_addDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_addDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_addDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_addlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_addDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_addDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_addDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_addDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_addDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_addmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_addDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_addDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_addDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_addDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_addDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_exclusive_addfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_exclusive_addDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_exclusive_addDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_exclusive_addDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_exclusive_addDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_exclusive_addDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_exclusive_adddPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_exclusive_addDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_exclusive_addDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_exclusive_addDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_exclusive_addDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_exclusive_addDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_maxi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_maxDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_maxDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_maxDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_maxDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_maxj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_maxDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_maxDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_maxDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_maxDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_maxl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_maxDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_maxDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_maxDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_maxDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_maxDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_maxm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_maxDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_maxDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_maxDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_maxDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_maxDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_exclusive_maxf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_exclusive_maxDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_exclusive_maxDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_exclusive_maxDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_exclusive_maxDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_exclusive_maxDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_exclusive_maxd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_exclusive_maxDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_exclusive_maxDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_exclusive_maxDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_exclusive_maxDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_exclusive_maxDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_maxiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_maxDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_maxDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_maxDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_maxDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_maxjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_maxDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_maxDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_maxDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_maxDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_maxlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_maxDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_maxDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_maxDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_maxDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_maxDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_maxmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_maxDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_maxDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_maxDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_maxDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_maxDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_exclusive_maxfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_exclusive_maxDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_exclusive_maxDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_exclusive_maxDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_exclusive_maxDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_exclusive_maxDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_exclusive_maxdPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_exclusive_maxDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_exclusive_maxDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_exclusive_maxDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_exclusive_maxDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_exclusive_maxDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_mini(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_minDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_minDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_minDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_minDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_minDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_minj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_minDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_minDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_minDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_minDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_minDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_minl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_minDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_minDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_minDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_minDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_minDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_minm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_minDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_minDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_minDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_minDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_minDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_exclusive_minf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_exclusive_minDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_exclusive_minDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_exclusive_minDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_exclusive_minDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_exclusive_minDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_exclusive_mind(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_exclusive_minDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_exclusive_minDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_exclusive_minDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_exclusive_minDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_exclusive_minDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_miniPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_minDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_minDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_minDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_minDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_minDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_minjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_minDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_minDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_minDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_minDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_minDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_minlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_minDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_minDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_minDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_minDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_minDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_minmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_minDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_minDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_minDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_minDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_minDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_exclusive_minfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_exclusive_minDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_exclusive_minDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_exclusive_minDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_exclusive_minDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_exclusive_minDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_exclusive_mindPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_exclusive_minDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_exclusive_minDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_exclusive_minDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_exclusive_minDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_exclusive_minDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_addi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_addDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_addDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_addDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_addDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_addDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_addj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_addDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_addDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_addDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_addDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_addDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_addl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_addDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_addDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_addDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_addDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_addDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_addm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_addDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_addDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_addDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_addDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_addDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_inclusive_addf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_inclusive_addDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_inclusive_addDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_inclusive_addDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_inclusive_addDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_inclusive_addDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_inclusive_addd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_inclusive_addDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_inclusive_addDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_inclusive_addDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_inclusive_addDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_inclusive_addDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_addiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_addDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_addDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_addDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_addDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_addDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_addjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_addDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_addDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_addDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_addDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_addDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_addlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_addDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_addDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_addDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_addDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_addDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_addmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_addDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_addDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_addDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_addDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_addDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_inclusive_addfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_inclusive_addDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_inclusive_addDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_inclusive_addDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_inclusive_addDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_inclusive_addDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_inclusive_adddPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_inclusive_addDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_inclusive_addDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_inclusive_addDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_inclusive_addDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_inclusive_addDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_maxi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_maxDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_maxDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_maxDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_maxDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_maxDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_maxj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_maxDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_maxDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_maxDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_maxDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_maxDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_maxl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_maxDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_maxDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_maxDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_maxDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_maxDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_maxm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_maxDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_maxDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_maxDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_maxDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_maxDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_inclusive_maxf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_inclusive_maxDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_inclusive_maxDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_inclusive_maxDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_inclusive_maxDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_inclusive_maxDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_inclusive_maxd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_inclusive_maxDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_inclusive_maxDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_inclusive_maxDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_inclusive_maxDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_inclusive_maxDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_maxiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_maxDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_maxDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_maxDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_maxDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_maxDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_maxjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_maxDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_maxDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_maxDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_maxDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_maxDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_maxlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_maxDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_maxDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_maxDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_maxDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_maxDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_maxmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_maxDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_maxDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_maxDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_maxDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_maxDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_inclusive_maxfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_inclusive_maxDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_inclusive_maxDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_inclusive_maxDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_inclusive_maxDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_inclusive_maxDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_inclusive_maxdPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_inclusive_maxDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_inclusive_maxDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_inclusive_maxDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_inclusive_maxDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_inclusive_maxDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_mini(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_minDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_minDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_minDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_minDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_minDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_minj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_minDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_minDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_minDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_minDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_minDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_minl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_minDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_minDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_minDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_minDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_minDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_minm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_minDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_minDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_minDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_minDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_minDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_inclusive_minf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_inclusive_minDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_inclusive_minDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_inclusive_minDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_inclusive_minDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_inclusive_minDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_inclusive_mind(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_inclusive_minDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_inclusive_minDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_inclusive_minDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_inclusive_minDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_inclusive_minDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_miniPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_minDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_minDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_minDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_minDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_minDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_minjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_minDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_minDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_minDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_minDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_minDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_minlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_minDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_minDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_minDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_minDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_minDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_minmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_minDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_minDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_minDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_minDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_minDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_inclusive_minfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_inclusive_minDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_inclusive_minDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_inclusive_minDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_inclusive_minDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_inclusive_minDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_inclusive_mindPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_inclusive_minDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_inclusive_minDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_inclusive_minDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_inclusive_minDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_inclusive_minDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5


attributes #3 = { nounwind readnone }
attributes #5 = { nounwind }

; DEBUGIFY-NOT: WARNING
