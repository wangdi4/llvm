; REQUIRES: 64bit
; RUN: llvm-extract %libdir/clbltfnz0.rtl -rfunc work_group_broadcast -S -o - | FileCheck %s

; char
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_cmDv4_j(<4 x i8> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_cmDv8_j(<8 x i8> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_cmDv16_j(<16 x i8> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_cmDv32_j(<32 x i8> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_cmDv64_j(<64 x i8> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned char
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_hmDv4_j(<4 x i8> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_hmDv8_j(<8 x i8> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_hmDv16_j(<16 x i8> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_hmDv32_j(<32 x i8> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_hmDv64_j(<64 x i8> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; short
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_smDv4_j(<4 x i16> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_smDv8_j(<8 x i16> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_smDv16_j(<16 x i16> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_smDv32_j(<32 x i16> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_smDv64_j(<64 x i16> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned short
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_tmDv4_j(<4 x i16> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_tmDv8_j(<8 x i16> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_tmDv16_j(<16 x i16> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_tmDv32_j(<32 x i16> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_tmDv64_j(<64 x i16> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; int
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_imDv4_j(<4 x i32> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_imDv8_j(<8 x i32> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_imDv16_j(<16 x i32> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_imDv32_j(<32 x i32> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_imDv64_j(<64 x i32> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned int
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_jmS_(<4 x i32> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_jmS_(<8 x i32> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_jmS_(<16 x i32> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_jmS_(<32 x i32> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_jmS_(<64 x i32> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; long
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_lmDv4_j(<4 x i64> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_lmDv8_j(<8 x i64> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_lmDv16_j(<16 x i64> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_lmDv32_j(<32 x i64> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_lmDv64_j(<64 x i64> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; unsigned long
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_mmDv4_j(<4 x i64> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_mmDv8_j(<8 x i64> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_mmDv16_j(<16 x i64> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_mmDv32_j(<32 x i64> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_mmDv64_j(<64 x i64> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; float
; CHECK: <4 x float> @_Z20work_group_broadcastDv4_fmDv4_j(<4 x float> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x float> @_Z20work_group_broadcastDv8_fmDv8_j(<8 x float> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x float> @_Z20work_group_broadcastDv16_fmDv16_j(<16 x float> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x float> @_Z20work_group_broadcastDv32_fmDv32_j(<32 x float> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x float> @_Z20work_group_broadcastDv64_fmDv64_j(<64 x float> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})

; double
; CHECK: <4 x double> @_Z20work_group_broadcastDv4_dmDv4_j(<4 x double> {{.*}}, i64 {{.*}}, <4 x i32> {{.*}})
; CHECK: <8 x double> @_Z20work_group_broadcastDv8_dmDv8_j(<8 x double> {{.*}}, i64 {{.*}}, <8 x i32> {{.*}})
; CHECK: <16 x double> @_Z20work_group_broadcastDv16_dmDv16_j(<16 x double> {{.*}}, i64 {{.*}}, <16 x i32> {{.*}})
; CHECK: <32 x double> @_Z20work_group_broadcastDv32_dmDv32_j(<32 x double> {{.*}}, i64 {{.*}}, <32 x i32> {{.*}})
; CHECK: <64 x double> @_Z20work_group_broadcastDv64_dmDv64_j(<64 x double> {{.*}}, i64 {{.*}}, <64 x i32> {{.*}})
