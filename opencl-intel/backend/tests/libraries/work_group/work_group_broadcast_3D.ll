; REQUIRES: 64bit
; RUN: llvm-extract %libdir/clbltfnz0.rtl -rfunc work_group_broadcast -S -o - | FileCheck %s

; char
; CHECK: signext i8 @_Z20work_group_broadcastcmmm(i8 {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_cmmm(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_cmmm(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_cmmm(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_cmmm(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_cmmm(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})

; unsigned char
; CHECK: zeroext i8 @_Z20work_group_broadcasthmmm(i8 {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_hmmm(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_hmmm(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_hmmm(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_hmmm(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_hmmm(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})

; short
; CHECK: signext i16 @_Z20work_group_broadcastsmmm(i16 {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_smmm(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_smmm(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_smmm(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_smmm(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_smmm(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})

; unsigned short
; CHECK: zeroext i16 @_Z20work_group_broadcasttmmm(i16 {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_tmmm(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_tmmm(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_tmmm(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_tmmm(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_tmmm(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})

; int
; CHECK: i32 @_Z20work_group_broadcastimmm(i32 {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_immm(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_immm(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_immm(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_immm(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_immm(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})

; unsigned int
; CHECK: i32 @_Z20work_group_broadcastjmmm(i32 {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_jmmm(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_jmmm(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_jmmm(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_jmmm(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_jmmm(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})

; long
; CHECK: i64 @_Z20work_group_broadcastlmmm(i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_lmmm(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_lmmm(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_lmmm(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_lmmm(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_lmmm(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})

; unsigned long
; CHECK: i64 @_Z20work_group_broadcastmmmm(i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_mmmm(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_mmmm(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_mmmm(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_mmmm(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_mmmm(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})

; float
; CHECK: float @_Z20work_group_broadcastfmmm(float {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x float> @_Z20work_group_broadcastDv4_fmmm(<4 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x float> @_Z20work_group_broadcastDv8_fmmm(<8 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x float> @_Z20work_group_broadcastDv16_fmmm(<16 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x float> @_Z20work_group_broadcastDv32_fmmm(<32 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x float> @_Z20work_group_broadcastDv64_fmmm(<64 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})

; double
; CHECK: double @_Z20work_group_broadcastdmmm(double {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x double> @_Z20work_group_broadcastDv4_dmmm(<4 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x double> @_Z20work_group_broadcastDv8_dmmm(<8 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x double> @_Z20work_group_broadcastDv16_dmmm(<16 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x double> @_Z20work_group_broadcastDv32_dmmm(<32 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x double> @_Z20work_group_broadcastDv64_dmmm(<64 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
