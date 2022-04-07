; REQUIRES: 64bit
; RUN: llvm-extract %libdir/clbltfnz0.rtl -rfunc work_group_broadcast -S -o - | FileCheck %s

; char
; CHECK: signext i8 @_Z20work_group_broadcastcm(i8 {{.*}}, i64 {{.*}})
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_cm(<4 x i8> {{.*}}, i64 {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_cm(<8 x i8> {{.*}}, i64 {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_cm(<16 x i8> {{.*}}, i64 {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_cm(<32 x i8> {{.*}}, i64 {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_cm(<64 x i8> {{.*}}, i64 {{.*}})

; unsigned char
; CHECK: zeroext i8 @_Z20work_group_broadcasthm(i8 {{.*}}, i64 {{.*}})
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_hm(<4 x i8> {{.*}}, i64 {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_hm(<8 x i8> {{.*}}, i64 {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_hm(<16 x i8> {{.*}}, i64 {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_hm(<32 x i8> {{.*}}, i64 {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_hm(<64 x i8> {{.*}}, i64 {{.*}})

; short
; CHECK: signext i16 @_Z20work_group_broadcastsm(i16 {{.*}}, i64 {{.*}})
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_sm(<4 x i16> {{.*}}, i64 {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_sm(<8 x i16> {{.*}}, i64 {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_sm(<16 x i16> {{.*}}, i64 {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_sm(<32 x i16> {{.*}}, i64 {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_sm(<64 x i16> {{.*}}, i64 {{.*}})

; unsigned short
; CHECK: zeroext i16 @_Z20work_group_broadcasttm(i16 {{.*}}, i64 {{.*}})
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_tm(<4 x i16> {{.*}}, i64 {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_tm(<8 x i16> {{.*}}, i64 {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_tm(<16 x i16> {{.*}}, i64 {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_tm(<32 x i16> {{.*}}, i64 {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_tm(<64 x i16> {{.*}}, i64 {{.*}})

; int
; CHECK: i32 @_Z20work_group_broadcastim(i32 {{.*}}, i64 {{.*}})
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_im(<4 x i32> {{.*}}, i64 {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_im(<8 x i32> {{.*}}, i64 {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_im(<16 x i32> {{.*}}, i64 {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_im(<32 x i32> {{.*}}, i64 {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_im(<64 x i32> {{.*}}, i64 {{.*}})

; unsigned int
; CHECK: i32 @_Z20work_group_broadcastjm(i32 {{.*}}, i64 {{.*}})
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_jm(<4 x i32> {{.*}}, i64 {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_jm(<8 x i32> {{.*}}, i64 {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_jm(<16 x i32> {{.*}}, i64 {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_jm(<32 x i32> {{.*}}, i64 {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_jm(<64 x i32> {{.*}}, i64 {{.*}})

; long
; CHECK: i64 @_Z20work_group_broadcastlm(i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_lm(<4 x i64> {{.*}}, i64 {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_lm(<8 x i64> {{.*}}, i64 {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_lm(<16 x i64> {{.*}}, i64 {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_lm(<32 x i64> {{.*}}, i64 {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_lm(<64 x i64> {{.*}}, i64 {{.*}})

; unsigned long
; CHECK: i64 @_Z20work_group_broadcastmm(i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_mm(<4 x i64> {{.*}}, i64 {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_mm(<8 x i64> {{.*}}, i64 {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_mm(<16 x i64> {{.*}}, i64 {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_mm(<32 x i64> {{.*}}, i64 {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_mm(<64 x i64> {{.*}}, i64 {{.*}})

; float
; CHECK: float @_Z20work_group_broadcastfm(float {{.*}}, i64 {{.*}})
; CHECK: <4 x float> @_Z20work_group_broadcastDv4_fm(<4 x float> {{.*}}, i64 {{.*}})
; CHECK: <8 x float> @_Z20work_group_broadcastDv8_fm(<8 x float> {{.*}}, i64 {{.*}})
; CHECK: <16 x float> @_Z20work_group_broadcastDv16_fm(<16 x float> {{.*}}, i64 {{.*}})
; CHECK: <32 x float> @_Z20work_group_broadcastDv32_fm(<32 x float> {{.*}}, i64 {{.*}})
; CHECK: <64 x float> @_Z20work_group_broadcastDv64_fm(<64 x float> {{.*}}, i64 {{.*}})

; double
; CHECK: double @_Z20work_group_broadcastdm(double {{.*}}, i64 {{.*}})
; CHECK: <4 x double> @_Z20work_group_broadcastDv4_dm(<4 x double> {{.*}}, i64 {{.*}})
; CHECK: <8 x double> @_Z20work_group_broadcastDv8_dm(<8 x double> {{.*}}, i64 {{.*}})
; CHECK: <16 x double> @_Z20work_group_broadcastDv16_dm(<16 x double> {{.*}}, i64 {{.*}})
; CHECK: <32 x double> @_Z20work_group_broadcastDv32_dm(<32 x double> {{.*}}, i64 {{.*}})
; CHECK: <64 x double> @_Z20work_group_broadcastDv64_dm(<64 x double> {{.*}}, i64 {{.*}})
