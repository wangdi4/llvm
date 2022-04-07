; REQUIRES: 64bit
; RUN: llvm-extract %libdir/clbltfnz0.rtl -rfunc work_group_broadcast -S -o - | FileCheck %s

; char
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_cmmmDv4_j(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_cmmmDv8_j(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_cmmmDv16_j(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_cmmmDv32_j(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_cmmmDv64_j(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned char
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_hmmmDv4_j(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_hmmmDv8_j(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_hmmmDv16_j(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_hmmmDv32_j(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_hmmmDv64_j(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; short
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_smmmDv4_j(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_smmmDv8_j(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_smmmDv16_j(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_smmmDv32_j(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_smmmDv64_j(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned short
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_tmmmDv4_j(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_tmmmDv8_j(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_tmmmDv16_j(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_tmmmDv32_j(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_tmmmDv64_j(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; int
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_immmDv4_j(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_immmDv8_j(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_immmDv16_j(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_immmDv32_j(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_immmDv64_j(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned int
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_jmmmS_(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_jmmmS_(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_jmmmS_(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_jmmmS_(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_jmmmS_(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; long
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_lmmmDv4_j(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_lmmmDv8_j(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_lmmmDv16_j(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_lmmmDv32_j(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_lmmmDv64_j(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned long
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_mmmmDv4_j(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_mmmmDv8_j(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_mmmmDv16_j(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_mmmmDv32_j(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_mmmmDv64_j(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; float
; CHECK: <4 x float> @_Z20work_group_broadcastDv4_fmmmDv4_j(<4 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x float> @_Z20work_group_broadcastDv8_fmmmDv8_j(<8 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x float> @_Z20work_group_broadcastDv16_fmmmDv16_j(<16 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x float> @_Z20work_group_broadcastDv32_fmmmDv32_j(<32 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x float> @_Z20work_group_broadcastDv64_fmmmDv64_j(<64 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; double
; CHECK: <4 x double> @_Z20work_group_broadcastDv4_dmmmDv4_j(<4 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x double> @_Z20work_group_broadcastDv8_dmmmDv8_j(<8 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x double> @_Z20work_group_broadcastDv16_dmmmDv16_j(<16 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x double> @_Z20work_group_broadcastDv32_dmmmDv32_j(<32 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x double> @_Z20work_group_broadcastDv64_dmmmDv64_j(<64 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})
