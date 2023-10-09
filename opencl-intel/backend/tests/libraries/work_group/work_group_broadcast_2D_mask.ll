; REQUIRES: 64bit
; RUN: llvm-extract %libdir/clbltfnz0.rtl -rfunc work_group_broadcast -S -o - | FileCheck %s

; char
; CHECK: <4 x i8> @_Z22__work_group_broadcastDv4_cmmS_(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i8> {{.*}})
; CHECK: <8 x i8> @_Z22__work_group_broadcastDv8_cmmS_(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i8> {{.*}})
; CHECK: <16 x i8> @_Z22__work_group_broadcastDv16_cmmS_(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i8> {{.*}})
; CHECK: <32 x i8> @_Z22__work_group_broadcastDv32_cmmS_(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i8> {{.*}})
; CHECK: <64 x i8> @_Z22__work_group_broadcastDv64_cmmS_(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i8> {{.*}})

; unsigned char
; CHECK: <4 x i8> @_Z22__work_group_broadcastDv4_hmmS_(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i8> {{.*}})
; CHECK: <8 x i8> @_Z22__work_group_broadcastDv8_hmmS_(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i8> {{.*}})
; CHECK: <16 x i8> @_Z22__work_group_broadcastDv16_hmmS_(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i8> {{.*}})
; CHECK: <32 x i8> @_Z22__work_group_broadcastDv32_hmmS_(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i8> {{.*}})
; CHECK: <64 x i8> @_Z22__work_group_broadcastDv64_hmmS_(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i8> {{.*}})

; short
; CHECK: <4 x i16> @_Z22__work_group_broadcastDv4_smmS_(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i16> {{.*}})
; CHECK: <8 x i16> @_Z22__work_group_broadcastDv8_smmS_(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i16> {{.*}})
; CHECK: <16 x i16> @_Z22__work_group_broadcastDv16_smmS_(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i16> {{.*}})
; CHECK: <32 x i16> @_Z22__work_group_broadcastDv32_smmS_(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i16> {{.*}})
; CHECK: <64 x i16> @_Z22__work_group_broadcastDv64_smmS_(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i16> {{.*}})

; unsigned short
; CHECK: <4 x i16> @_Z22__work_group_broadcastDv4_tmmS_(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i16> {{.*}})
; CHECK: <8 x i16> @_Z22__work_group_broadcastDv8_tmmS_(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i16> {{.*}})
; CHECK: <16 x i16> @_Z22__work_group_broadcastDv16_tmmS_(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i16> {{.*}})
; CHECK: <32 x i16> @_Z22__work_group_broadcastDv32_tmmS_(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i16> {{.*}})
; CHECK: <64 x i16> @_Z22__work_group_broadcastDv64_tmmS_(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i16> {{.*}})

; int
; CHECK: <4 x i32> @_Z22__work_group_broadcastDv4_immS_(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z22__work_group_broadcastDv8_immS_(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z22__work_group_broadcastDv16_immS_(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z22__work_group_broadcastDv32_immS_(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z22__work_group_broadcastDv64_immS_(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned int
; CHECK: <4 x i32> @_Z22__work_group_broadcastDv4_jmmS_(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z22__work_group_broadcastDv8_jmmS_(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z22__work_group_broadcastDv16_jmmS_(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z22__work_group_broadcastDv32_jmmS_(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z22__work_group_broadcastDv64_jmmS_(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; long
; CHECK: <4 x i64> @_Z22__work_group_broadcastDv4_lmmS_(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i64> {{.*}})
; CHECK: <8 x i64> @_Z22__work_group_broadcastDv8_lmmS_(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i64> {{.*}})
; CHECK: <16 x i64> @_Z22__work_group_broadcastDv16_lmmS_(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i64> {{.*}})
; CHECK: <32 x i64> @_Z22__work_group_broadcastDv32_lmmS_(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i64> {{.*}})
; CHECK: <64 x i64> @_Z22__work_group_broadcastDv64_lmmS_(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i64> {{.*}})

; unsigned long
; CHECK: <4 x i64> @_Z22__work_group_broadcastDv4_mmmS_(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x i64> {{.*}})
; CHECK: <8 x i64> @_Z22__work_group_broadcastDv8_mmmS_(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x i64> {{.*}})
; CHECK: <16 x i64> @_Z22__work_group_broadcastDv16_mmmS_(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x i64> {{.*}})
; CHECK: <32 x i64> @_Z22__work_group_broadcastDv32_mmmS_(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x i64> {{.*}})
; CHECK: <64 x i64> @_Z22__work_group_broadcastDv64_mmmS_(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x i64> {{.*}})

; float
; CHECK: <4 x float> @_Z22__work_group_broadcastDv4_fmmS_(<4 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x float> {{.*}})
; CHECK: <8 x float> @_Z22__work_group_broadcastDv8_fmmS_(<8 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x float> {{.*}})
; CHECK: <16 x float> @_Z22__work_group_broadcastDv16_fmmS_(<16 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x float> {{.*}})
; CHECK: <32 x float> @_Z22__work_group_broadcastDv32_fmmS_(<32 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x float> {{.*}})
; CHECK: <64 x float> @_Z22__work_group_broadcastDv64_fmmS_(<64 x float> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x float> {{.*}})

; double
; CHECK: <4 x double> @_Z22__work_group_broadcastDv4_dmmS_(<4 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, <4 x double> {{.*}})
; CHECK: <8 x double> @_Z22__work_group_broadcastDv8_dmmS_(<8 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, <8 x double> {{.*}})
; CHECK: <16 x double> @_Z22__work_group_broadcastDv16_dmmS_(<16 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, <16 x double> {{.*}})
; CHECK: <32 x double> @_Z22__work_group_broadcastDv32_dmmS_(<32 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, <32 x double> {{.*}})
; CHECK: <64 x double> @_Z22__work_group_broadcastDv64_dmmS_(<64 x double> {{.*}}, i64 {{.*}}, i64 {{.*}}, <64 x double> {{.*}})
