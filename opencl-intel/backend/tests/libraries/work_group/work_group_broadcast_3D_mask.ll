; REQUIRES: 64bit
; RUN: llvm-extract %libdir/clbltfnz0.rtl -rfunc work_group_broadcast -S -o - | FileCheck %s

; char
; CHECK: <4 x i8> @_Z22__work_group_broadcastDv4_cmmmS_(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i8> {{.*}})
; CHECK: <8 x i8> @_Z22__work_group_broadcastDv8_cmmmS_(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i8> {{.*}})
; CHECK: <16 x i8> @_Z22__work_group_broadcastDv16_cmmmS_(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i8> {{.*}})
; CHECK: <32 x i8> @_Z22__work_group_broadcastDv32_cmmmS_(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i8> {{.*}})
; CHECK: <64 x i8> @_Z22__work_group_broadcastDv64_cmmmS_(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i8> {{.*}})

; unsigned char
; CHECK: <4 x i8> @_Z22__work_group_broadcastDv4_hmmmS_(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i8> {{.*}})
; CHECK: <8 x i8> @_Z22__work_group_broadcastDv8_hmmmS_(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i8> {{.*}})
; CHECK: <16 x i8> @_Z22__work_group_broadcastDv16_hmmmS_(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i8> {{.*}})
; CHECK: <32 x i8> @_Z22__work_group_broadcastDv32_hmmmS_(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i8> {{.*}})
; CHECK: <64 x i8> @_Z22__work_group_broadcastDv64_hmmmS_(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i8> {{.*}})

; short
; CHECK: <4 x i16> @_Z22__work_group_broadcastDv4_smmmS_(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i16> {{.*}})
; CHECK: <8 x i16> @_Z22__work_group_broadcastDv8_smmmS_(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i16> {{.*}})
; CHECK: <16 x i16> @_Z22__work_group_broadcastDv16_smmmS_(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i16> {{.*}})
; CHECK: <32 x i16> @_Z22__work_group_broadcastDv32_smmmS_(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i16> {{.*}})
; CHECK: <64 x i16> @_Z22__work_group_broadcastDv64_smmmS_(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i16> {{.*}})

; unsigned short
; CHECK: <4 x i16> @_Z22__work_group_broadcastDv4_tmmmS_(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i16> {{.*}})
; CHECK: <8 x i16> @_Z22__work_group_broadcastDv8_tmmmS_(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i16> {{.*}})
; CHECK: <16 x i16> @_Z22__work_group_broadcastDv16_tmmmS_(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i16> {{.*}})
; CHECK: <32 x i16> @_Z22__work_group_broadcastDv32_tmmmS_(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i16> {{.*}})
; CHECK: <64 x i16> @_Z22__work_group_broadcastDv64_tmmmS_(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i16> {{.*}})

; int
; CHECK: <4 x i32> @_Z22__work_group_broadcastDv4_immmS_(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z22__work_group_broadcastDv8_immmS_(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z22__work_group_broadcastDv16_immmS_(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z22__work_group_broadcastDv32_immmS_(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z22__work_group_broadcastDv64_immmS_(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned int
; CHECK: <4 x i32> @_Z22__work_group_broadcastDv4_jmmmS_(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z22__work_group_broadcastDv8_jmmmS_(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z22__work_group_broadcastDv16_jmmmS_(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z22__work_group_broadcastDv32_jmmmS_(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z22__work_group_broadcastDv64_jmmmS_(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; long
; CHECK: <4 x i64> @_Z22__work_group_broadcastDv4_lmmmS_(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i64> {{.*}})
; CHECK: <8 x i64> @_Z22__work_group_broadcastDv8_lmmmS_(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i64> {{.*}})
; CHECK: <16 x i64> @_Z22__work_group_broadcastDv16_lmmmS_(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i64> {{.*}})
; CHECK: <32 x i64> @_Z22__work_group_broadcastDv32_lmmmS_(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i64> {{.*}})
; CHECK: <64 x i64> @_Z22__work_group_broadcastDv64_lmmmS_(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i64> {{.*}})

; unsigned long
; CHECK: <4 x i64> @_Z22__work_group_broadcastDv4_mmmmS_(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i64> {{.*}})
; CHECK: <8 x i64> @_Z22__work_group_broadcastDv8_mmmmS_(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i64> {{.*}})
; CHECK: <16 x i64> @_Z22__work_group_broadcastDv16_mmmmS_(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i64> {{.*}})
; CHECK: <32 x i64> @_Z22__work_group_broadcastDv32_mmmmS_(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i64> {{.*}})
; CHECK: <64 x i64> @_Z22__work_group_broadcastDv64_mmmmS_(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i64> {{.*}})

; float
; CHECK: <4 x float> @_Z22__work_group_broadcastDv4_fmmmS_(<4 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x float> {{.*}})
; CHECK: <8 x float> @_Z22__work_group_broadcastDv8_fmmmS_(<8 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x float> {{.*}})
; CHECK: <16 x float> @_Z22__work_group_broadcastDv16_fmmmS_(<16 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x float> {{.*}})
; CHECK: <32 x float> @_Z22__work_group_broadcastDv32_fmmmS_(<32 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x float> {{.*}})
; CHECK: <64 x float> @_Z22__work_group_broadcastDv64_fmmmS_(<64 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x float> {{.*}})

; double
; CHECK: <4 x double> @_Z22__work_group_broadcastDv4_dmmmS_(<4 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x double> {{.*}})
; CHECK: <8 x double> @_Z22__work_group_broadcastDv8_dmmmS_(<8 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x double> {{.*}})
; CHECK: <16 x double> @_Z22__work_group_broadcastDv16_dmmmS_(<16 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x double> {{.*}})
; CHECK: <32 x double> @_Z22__work_group_broadcastDv32_dmmmS_(<32 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x double> {{.*}})
; CHECK: <64 x double> @_Z22__work_group_broadcastDv64_dmmmS_(<64 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x double> {{.*}})
