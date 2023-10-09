; REQUIRES: 64bit
; RUN: llvm-extract %libdir/clbltfnz0.rtl -rfunc work_group_broadcast -S -o - | FileCheck %s

; char
; CHECK: <4 x i8> @_Z22__work_group_broadcastDv4_cmS_(<4 x i8> {{.*}}, i64 {{.*}}, <4 x i8> {{.*}})
; CHECK: <8 x i8> @_Z22__work_group_broadcastDv8_cmS_(<8 x i8> {{.*}}, i64 {{.*}}, <8 x i8> {{.*}})
; CHECK: <16 x i8> @_Z22__work_group_broadcastDv16_cmS_(<16 x i8> {{.*}}, i64 {{.*}}, <16 x i8> {{.*}})
; CHECK: <32 x i8> @_Z22__work_group_broadcastDv32_cmS_(<32 x i8> {{.*}}, i64 {{.*}}, <32 x i8> {{.*}})
; CHECK: <64 x i8> @_Z22__work_group_broadcastDv64_cmS_(<64 x i8> {{.*}}, i64 {{.*}}, <64 x i8> {{.*}})

; unsigned char
; CHECK: <4 x i8> @_Z22__work_group_broadcastDv4_hmS_(<4 x i8> {{.*}}, i64 {{.*}}, <4 x i8> {{.*}})
; CHECK: <8 x i8> @_Z22__work_group_broadcastDv8_hmS_(<8 x i8> {{.*}}, i64 {{.*}}, <8 x i8> {{.*}})
; CHECK: <16 x i8> @_Z22__work_group_broadcastDv16_hmS_(<16 x i8> {{.*}}, i64 {{.*}}, <16 x i8> {{.*}})
; CHECK: <32 x i8> @_Z22__work_group_broadcastDv32_hmS_(<32 x i8> {{.*}}, i64 {{.*}}, <32 x i8> {{.*}})
; CHECK: <64 x i8> @_Z22__work_group_broadcastDv64_hmS_(<64 x i8> {{.*}}, i64 {{.*}}, <64 x i8> {{.*}})

; short
; CHECK: <4 x i16> @_Z22__work_group_broadcastDv4_smS_(<4 x i16> {{.*}}, i64 {{.*}}, <4 x i16> {{.*}})
; CHECK: <8 x i16> @_Z22__work_group_broadcastDv8_smS_(<8 x i16> {{.*}}, i64 {{.*}}, <8 x i16> {{.*}})
; CHECK: <16 x i16> @_Z22__work_group_broadcastDv16_smS_(<16 x i16> {{.*}}, i64 {{.*}}, <16 x i16> {{.*}})
; CHECK: <32 x i16> @_Z22__work_group_broadcastDv32_smS_(<32 x i16> {{.*}}, i64 {{.*}}, <32 x i16> {{.*}})
; CHECK: <64 x i16> @_Z22__work_group_broadcastDv64_smS_(<64 x i16> {{.*}}, i64 {{.*}}, <64 x i16> {{.*}})

; unsigned short
; CHECK: <4 x i16> @_Z22__work_group_broadcastDv4_tmS_(<4 x i16> {{.*}}, i64 {{.*}}, <4 x i16> {{.*}})
; CHECK: <8 x i16> @_Z22__work_group_broadcastDv8_tmS_(<8 x i16> {{.*}}, i64 {{.*}}, <8 x i16> {{.*}})
; CHECK: <16 x i16> @_Z22__work_group_broadcastDv16_tmS_(<16 x i16> {{.*}}, i64 {{.*}}, <16 x i16> {{.*}})
; CHECK: <32 x i16> @_Z22__work_group_broadcastDv32_tmS_(<32 x i16> {{.*}}, i64 {{.*}}, <32 x i16> {{.*}})
; CHECK: <64 x i16> @_Z22__work_group_broadcastDv64_tmS_(<64 x i16> {{.*}}, i64 {{.*}}, <64 x i16> {{.*}})

; int
; CHECK: <4 x i32> @_Z22__work_group_broadcastDv4_imS_(<4 x i32> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z22__work_group_broadcastDv8_imS_(<8 x i32> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z22__work_group_broadcastDv16_imS_(<16 x i32> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z22__work_group_broadcastDv32_imS_(<32 x i32> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z22__work_group_broadcastDv64_imS_(<64 x i32> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned int
; CHECK: <4 x i32> @_Z22__work_group_broadcastDv4_jmS_(<4 x i32> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z22__work_group_broadcastDv8_jmS_(<8 x i32> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z22__work_group_broadcastDv16_jmS_(<16 x i32> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z22__work_group_broadcastDv32_jmS_(<32 x i32> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z22__work_group_broadcastDv64_jmS_(<64 x i32> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; long
; CHECK: <4 x i64> @_Z22__work_group_broadcastDv4_lmS_(<4 x i64> {{.*}}, i64 {{.*}}, <4 x i64> {{.*}})
; CHECK: <8 x i64> @_Z22__work_group_broadcastDv8_lmS_(<8 x i64> {{.*}}, i64 {{.*}}, <8 x i64> {{.*}})
; CHECK: <16 x i64> @_Z22__work_group_broadcastDv16_lmS_(<16 x i64> {{.*}}, i64 {{.*}}, <16 x i64> {{.*}})
; CHECK: <32 x i64> @_Z22__work_group_broadcastDv32_lmS_(<32 x i64> {{.*}}, i64 {{.*}}, <32 x i64> {{.*}})
; CHECK: <64 x i64> @_Z22__work_group_broadcastDv64_lmS_(<64 x i64> {{.*}}, i64 {{.*}}, <64 x i64> {{.*}})

; unsigned long
; CHECK: <4 x i64> @_Z22__work_group_broadcastDv4_mmS_(<4 x i64> {{.*}}, i64 {{.*}}, <4 x i64> {{.*}})
; CHECK: <8 x i64> @_Z22__work_group_broadcastDv8_mmS_(<8 x i64> {{.*}}, i64 {{.*}}, <8 x i64> {{.*}})
; CHECK: <16 x i64> @_Z22__work_group_broadcastDv16_mmS_(<16 x i64> {{.*}}, i64 {{.*}}, <16 x i64> {{.*}})
; CHECK: <32 x i64> @_Z22__work_group_broadcastDv32_mmS_(<32 x i64> {{.*}}, i64 {{.*}}, <32 x i64> {{.*}})
; CHECK: <64 x i64> @_Z22__work_group_broadcastDv64_mmS_(<64 x i64> {{.*}}, i64 {{.*}}, <64 x i64> {{.*}})

; float
; CHECK: <4 x float> @_Z22__work_group_broadcastDv4_fmS_(<4 x float> {{.*}}, i64 {{.*}}, <4 x float> {{.*}})
; CHECK: <8 x float> @_Z22__work_group_broadcastDv8_fmS_(<8 x float> {{.*}}, i64 {{.*}}, <8 x float> {{.*}})
; CHECK: <16 x float> @_Z22__work_group_broadcastDv16_fmS_(<16 x float> {{.*}}, i64 {{.*}}, <16 x float> {{.*}})
; CHECK: <32 x float> @_Z22__work_group_broadcastDv32_fmS_(<32 x float> {{.*}}, i64 {{.*}}, <32 x float> {{.*}})
; CHECK: <64 x float> @_Z22__work_group_broadcastDv64_fmS_(<64 x float> {{.*}}, i64 {{.*}}, <64 x float> {{.*}})

; double
; CHECK: <4 x double> @_Z22__work_group_broadcastDv4_dmS_(<4 x double> {{.*}}, i64 {{.*}}, <4 x double> {{.*}})
; CHECK: <8 x double> @_Z22__work_group_broadcastDv8_dmS_(<8 x double> {{.*}}, i64 {{.*}}, <8 x double> {{.*}})
; CHECK: <16 x double> @_Z22__work_group_broadcastDv16_dmS_(<16 x double> {{.*}}, i64 {{.*}}, <16 x double> {{.*}})
; CHECK: <32 x double> @_Z22__work_group_broadcastDv32_dmS_(<32 x double> {{.*}}, i64 {{.*}}, <32 x double> {{.*}})
; CHECK: <64 x double> @_Z22__work_group_broadcastDv64_dmS_(<64 x double> {{.*}}, i64 {{.*}}, <64 x double> {{.*}})
