; REQUIRES: 64bit
; RUN: llvm-extract %libdir/clbltfnz0.rtl -rfunc work_group_broadcast -S -o - | FileCheck %s

; char
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_cmmDv4_j(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_cmmDv8_j(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_cmmDv16_j(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_cmmDv32_j(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_cmmDv64_j(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned char
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_hmmDv4_j(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_hmmDv8_j(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_hmmDv16_j(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_hmmDv32_j(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_hmmDv64_j(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; short
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_smmDv4_j(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_smmDv8_j(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_smmDv16_j(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_smmDv32_j(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_smmDv64_j(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned short
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_tmmDv4_j(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_tmmDv8_j(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_tmmDv16_j(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_tmmDv32_j(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_tmmDv64_j(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; int
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_immDv4_j(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_immDv8_j(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_immDv16_j(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_immDv32_j(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_immDv64_j(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned int
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_jmmS_(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_jmmS_(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_jmmS_(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_jmmS_(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_jmmS_(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; long
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_lmmDv4_j(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_lmmDv8_j(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_lmmDv16_j(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_lmmDv32_j(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_lmmDv64_j(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned long
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_mmmDv4_j(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_mmmDv8_j(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_mmmDv16_j(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_mmmDv32_j(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_mmmDv64_j(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; float
; CHECK: <4 x float> @_Z20work_group_broadcastDv4_fmmDv4_j(<4 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x float> @_Z20work_group_broadcastDv8_fmmDv8_j(<8 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x float> @_Z20work_group_broadcastDv16_fmmDv16_j(<16 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x float> @_Z20work_group_broadcastDv32_fmmDv32_j(<32 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x float> @_Z20work_group_broadcastDv64_fmmDv64_j(<64 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; double
; CHECK: <4 x double> @_Z20work_group_broadcastDv4_dmmDv4_j(<4 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x double> @_Z20work_group_broadcastDv8_dmmDv8_j(<8 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x double> @_Z20work_group_broadcastDv16_dmmDv16_j(<16 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x double> @_Z20work_group_broadcastDv32_dmmDv32_j(<32 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x double> @_Z20work_group_broadcastDv64_dmmDv64_j(<64 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})
