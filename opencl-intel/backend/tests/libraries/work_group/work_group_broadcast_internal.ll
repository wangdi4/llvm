; REQUIRES: 64bit
; RUN: llvm-extract %libdir/clbltfnz0.rtl -rfunc work_group_broadcast -S -o - | FileCheck %s

; The GroupBuiltin pass will replace work_group_broadcast calls with the following internal implementations.

; char
; CHECK: signext i8 @_Z20work_group_broadcastcmmPc(i8 signext %src, i64 %linear_local_id, i64 %linear_id, i8* nocapture %accum)
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_cmmPS_(<4 x i8> %src, i64 %linear_local_id, i64 %linear_id, <4 x i8>* nocapture %accum)
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_cmmPS_(<8 x i8> %src, i64 %linear_local_id, i64 %linear_id, <8 x i8>* nocapture %accum)
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_cmmPS_(<16 x i8> %src, i64 %linear_local_id, i64 %linear_id, <16 x i8>* nocapture %accum)
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_cmmPS_(<32 x i8> %src, i64 %linear_local_id, i64 %linear_id, <32 x i8>* nocapture %accum)
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_cmmPS_(<64 x i8> %src, i64 %linear_local_id, i64 %linear_id, <64 x i8>* nocapture %accum)

; unsigned char
; CHECK: zeroext i8 @_Z20work_group_broadcasthmmPh(i8 zeroext %src, i64 %linear_local_id, i64 %linear_id, i8* nocapture %accum)
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_hmmPS_(<4 x i8> %src, i64 %linear_local_id, i64 %linear_id, <4 x i8>* nocapture %accum)
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_hmmPS_(<8 x i8> %src, i64 %linear_local_id, i64 %linear_id, <8 x i8>* nocapture %accum)
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_hmmPS_(<16 x i8> %src, i64 %linear_local_id, i64 %linear_id, <16 x i8>* nocapture %accum)
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_hmmPS_(<32 x i8> %src, i64 %linear_local_id, i64 %linear_id, <32 x i8>* nocapture %accum)
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_hmmPS_(<64 x i8> %src, i64 %linear_local_id, i64 %linear_id, <64 x i8>* nocapture %accum)

; short
; CHECK: signext i16 @_Z20work_group_broadcastsmmPs(i16 signext %src, i64 %linear_local_id, i64 %linear_id, i16* nocapture %accum)
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_smmPS_(<4 x i16> %src, i64 %linear_local_id, i64 %linear_id, <4 x i16>* nocapture %accum)
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_smmPS_(<8 x i16> %src, i64 %linear_local_id, i64 %linear_id, <8 x i16>* nocapture %accum)
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_smmPS_(<16 x i16> %src, i64 %linear_local_id, i64 %linear_id, <16 x i16>* nocapture %accum)
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_smmPS_(<32 x i16> %src, i64 %linear_local_id, i64 %linear_id, <32 x i16>* nocapture %accum)
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_smmPS_(<64 x i16> %src, i64 %linear_local_id, i64 %linear_id, <64 x i16>* nocapture %accum)

; unsigned short
; CHECK: zeroext i16 @_Z20work_group_broadcasttmmPt(i16 zeroext %src, i64 %linear_local_id, i64 %linear_id, i16* nocapture %accum)
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_tmmPS_(<4 x i16> %src, i64 %linear_local_id, i64 %linear_id, <4 x i16>* nocapture %accum)
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_tmmPS_(<8 x i16> %src, i64 %linear_local_id, i64 %linear_id, <8 x i16>* nocapture %accum)
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_tmmPS_(<16 x i16> %src, i64 %linear_local_id, i64 %linear_id, <16 x i16>* nocapture %accum)
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_tmmPS_(<32 x i16> %src, i64 %linear_local_id, i64 %linear_id, <32 x i16>* nocapture %accum)
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_tmmPS_(<64 x i16> %src, i64 %linear_local_id, i64 %linear_id, <64 x i16>* nocapture %accum)

; int
; CHECK: i32 @_Z20work_group_broadcastimmPi(i32 %src, i64 %linear_local_id, i64 %linear_id, i32* nocapture %accum)
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_immPS_(<4 x i32> %src, i64 %linear_local_id, i64 %linear_id, <4 x i32>* nocapture %accum)
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_immPS_(<8 x i32> %src, i64 %linear_local_id, i64 %linear_id, <8 x i32>* nocapture %accum)
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_immPS_(<16 x i32> %src, i64 %linear_local_id, i64 %linear_id, <16 x i32>* nocapture %accum)
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_immPS_(<32 x i32> %src, i64 %linear_local_id, i64 %linear_id, <32 x i32>* nocapture %accum)
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_immPS_(<64 x i32> %src, i64 %linear_local_id, i64 %linear_id, <64 x i32>* nocapture %accum)

; unsigned int
; CHECK: i32 @_Z20work_group_broadcastjmmPj(i32 %src, i64 %linear_local_id, i64 %linear_id, i32* nocapture %accum)
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_jmmPS_(<4 x i32> %src, i64 %linear_local_id, i64 %linear_id, <4 x i32>* nocapture %accum)
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_jmmPS_(<8 x i32> %src, i64 %linear_local_id, i64 %linear_id, <8 x i32>* nocapture %accum)
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_jmmPS_(<16 x i32> %src, i64 %linear_local_id, i64 %linear_id, <16 x i32>* nocapture %accum)
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_jmmPS_(<32 x i32> %src, i64 %linear_local_id, i64 %linear_id, <32 x i32>* nocapture %accum)
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_jmmPS_(<64 x i32> %src, i64 %linear_local_id, i64 %linear_id, <64 x i32>* nocapture %accum)

; long
; CHECK: i64 @_Z20work_group_broadcastlmmPl(i64 %src, i64 %linear_local_id, i64 %linear_id, i64* nocapture %accum)
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_lmmPS_(<4 x i64> %src, i64 %linear_local_id, i64 %linear_id, <4 x i64>* nocapture %accum)
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_lmmPS_(<8 x i64> %src, i64 %linear_local_id, i64 %linear_id, <8 x i64>* nocapture %accum)
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_lmmPS_(<16 x i64> %src, i64 %linear_local_id, i64 %linear_id, <16 x i64>* nocapture %accum)
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_lmmPS_(<32 x i64> %src, i64 %linear_local_id, i64 %linear_id, <32 x i64>* nocapture %accum)
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_lmmPS_(<64 x i64> %src, i64 %linear_local_id, i64 %linear_id, <64 x i64>* nocapture %accum)

; unsigned long
; CHECK: i64 @_Z20work_group_broadcastmmmPm(i64 %src, i64 %linear_local_id, i64 %linear_id, i64* nocapture %accum)
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_mmmPS_(<4 x i64> %src, i64 %linear_local_id, i64 %linear_id, <4 x i64>* nocapture %accum)
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_mmmPS_(<8 x i64> %src, i64 %linear_local_id, i64 %linear_id, <8 x i64>* nocapture %accum)
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_mmmPS_(<16 x i64> %src, i64 %linear_local_id, i64 %linear_id, <16 x i64>* nocapture %accum)
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_mmmPS_(<32 x i64> %src, i64 %linear_local_id, i64 %linear_id, <32 x i64>* nocapture %accum)
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_mmmPS_(<64 x i64> %src, i64 %linear_local_id, i64 %linear_id, <64 x i64>* nocapture %accum)

; float
; CHECK: float @_Z20work_group_broadcastfmmPf(float %src, i64 %linear_local_id, i64 %linear_id, float* nocapture %accum)
; CHECK: <4 x float> @_Z20work_group_broadcastDv4_fmmPS_(<4 x float> %src, i64 %linear_local_id, i64 %linear_id, <4 x float>* nocapture %accum)
; CHECK: <8 x float> @_Z20work_group_broadcastDv8_fmmPS_(<8 x float> %src, i64 %linear_local_id, i64 %linear_id, <8 x float>* nocapture %accum)
; CHECK: <16 x float> @_Z20work_group_broadcastDv16_fmmPS_(<16 x float> %src, i64 %linear_local_id, i64 %linear_id, <16 x float>* nocapture %accum)
; CHECK: <32 x float> @_Z20work_group_broadcastDv32_fmmPS_(<32 x float> %src, i64 %linear_local_id, i64 %linear_id, <32 x float>* nocapture %accum)
; CHECK: <64 x float> @_Z20work_group_broadcastDv64_fmmPS_(<64 x float> %src, i64 %linear_local_id, i64 %linear_id, <64 x float>* nocapture %accum)

; double
; CHECK: double @_Z20work_group_broadcastdmmPd(double %src, i64 %linear_local_id, i64 %linear_id, double* nocapture %accum)
; CHECK: <4 x double> @_Z20work_group_broadcastDv4_dmmPS_(<4 x double> %src, i64 %linear_local_id, i64 %linear_id, <4 x double>* nocapture %accum)
; CHECK: <8 x double> @_Z20work_group_broadcastDv8_dmmPS_(<8 x double> %src, i64 %linear_local_id, i64 %linear_id, <8 x double>* nocapture %accum)
; CHECK: <16 x double> @_Z20work_group_broadcastDv16_dmmPS_(<16 x double> %src, i64 %linear_local_id, i64 %linear_id, <16 x double>* nocapture %accum)
; CHECK: <32 x double> @_Z20work_group_broadcastDv32_dmmPS_(<32 x double> %src, i64 %linear_local_id, i64 %linear_id, <32 x double>* nocapture %accum)
; CHECK: <64 x double> @_Z20work_group_broadcastDv64_dmmPS_(<64 x double> %src, i64 %linear_local_id, i64 %linear_id, <64 x double>* nocapture %accum)
