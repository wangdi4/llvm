; REQUIRES: 64bit
; RUN: llvm-extract %libdir/clbltfnz0.rtl -rfunc work_group_broadcast -S -o - | FileCheck %s

; char
; CHECK: signext i8 @_Z20work_group_broadcastcmm(i8 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_cmm(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_cmm(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_cmm(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_cmm(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_cmm(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}})

; unsigned char
; CHECK: zeroext i8 @_Z20work_group_broadcasthmm(i8 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i8> @_Z20work_group_broadcastDv4_hmm(<4 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i8> @_Z20work_group_broadcastDv8_hmm(<8 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i8> @_Z20work_group_broadcastDv16_hmm(<16 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i8> @_Z20work_group_broadcastDv32_hmm(<32 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i8> @_Z20work_group_broadcastDv64_hmm(<64 x i8> {{.*}}, i64 {{.*}}, i64 {{.*}})

; short
; CHECK: signext i16 @_Z20work_group_broadcastsmm(i16 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_smm(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_smm(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_smm(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_smm(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_smm(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}})

; unsigned short
; CHECK: zeroext i16 @_Z20work_group_broadcasttmm(i16 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i16> @_Z20work_group_broadcastDv4_tmm(<4 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i16> @_Z20work_group_broadcastDv8_tmm(<8 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i16> @_Z20work_group_broadcastDv16_tmm(<16 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i16> @_Z20work_group_broadcastDv32_tmm(<32 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i16> @_Z20work_group_broadcastDv64_tmm(<64 x i16> {{.*}}, i64 {{.*}}, i64 {{.*}})

; int
; CHECK: i32 @_Z20work_group_broadcastimm(i32 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_imm(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_imm(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_imm(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_imm(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_imm(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}})

; unsigned int
; CHECK: i32 @_Z20work_group_broadcastjmm(i32 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i32> @_Z20work_group_broadcastDv4_jmm(<4 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i32> @_Z20work_group_broadcastDv8_jmm(<8 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i32> @_Z20work_group_broadcastDv16_jmm(<16 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i32> @_Z20work_group_broadcastDv32_jmm(<32 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i32> @_Z20work_group_broadcastDv64_jmm(<64 x i32> {{.*}}, i64 {{.*}}, i64 {{.*}})

; long
; CHECK: i64 @_Z20work_group_broadcastlmm(i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_lmm(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_lmm(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_lmm(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_lmm(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_lmm(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}})

; unsigned long
; CHECK: i64 @_Z20work_group_broadcastmmm(i64 {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x i64> @_Z20work_group_broadcastDv4_mmm(<4 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x i64> @_Z20work_group_broadcastDv8_mmm(<8 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x i64> @_Z20work_group_broadcastDv16_mmm(<16 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x i64> @_Z20work_group_broadcastDv32_mmm(<32 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x i64> @_Z20work_group_broadcastDv64_mmm(<64 x i64> {{.*}}, i64 {{.*}}, i64 {{.*}})

; float
; CHECK: float @_Z20work_group_broadcastfmm(float {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x float> @_Z20work_group_broadcastDv4_fmm(<4 x float> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x float> @_Z20work_group_broadcastDv8_fmm(<8 x float> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x float> @_Z20work_group_broadcastDv16_fmm(<16 x float> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x float> @_Z20work_group_broadcastDv32_fmm(<32 x float> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x float> @_Z20work_group_broadcastDv64_fmm(<64 x float> {{.*}}, i64 {{.*}}, i64 {{.*}})

; double
; CHECK: double @_Z20work_group_broadcastdmm(double {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <4 x double> @_Z20work_group_broadcastDv4_dmm(<4 x double> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <8 x double> @_Z20work_group_broadcastDv8_dmm(<8 x double> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <16 x double> @_Z20work_group_broadcastDv16_dmm(<16 x double> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <32 x double> @_Z20work_group_broadcastDv32_dmm(<32 x double> {{.*}}, i64 {{.*}}, i64 {{.*}})
; CHECK: <64 x double> @_Z20work_group_broadcastDv64_dmm(<64 x double> {{.*}}, i64 {{.*}}, i64 {{.*}})
