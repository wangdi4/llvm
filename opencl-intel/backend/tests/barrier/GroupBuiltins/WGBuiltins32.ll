; RUN: %oclopt -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -verify -S < %s
;; This file is used as Built-in module to test work group built-in pass for 32bit modules
;; The only requirment is to contain a valid LLVM IR.

; Function Attrs: nounwind readnone
declare i32 @_Z19work_group_add_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z19work_group_add_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z19work_group_add_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z19work_group_add_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z19work_group_add_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z19work_group_add_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z19work_group_add_utiljj(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z19work_group_add_utilDv4_jS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z19work_group_add_utilDv8_jS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z19work_group_add_utilDv16_jS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z19work_group_add_utilDv32_jS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z19work_group_add_utilDv64_jS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z19work_group_add_utilll(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z19work_group_add_utilDv4_lS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z19work_group_add_utilDv8_lS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z19work_group_add_utilDv16_lS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z19work_group_add_utilDv32_lS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z19work_group_add_utilDv64_lS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare i64 @_Z19work_group_add_utilmm(i64 %src, i64 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z19work_group_add_utilDv4_mS_(<4 x i64> %src, <4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z19work_group_add_utilDv8_mS_(<8 x i64> %src, <8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z19work_group_add_utilDv16_mS_(<16 x i64> %src, <16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z19work_group_add_utilDv32_mS_(<32 x i64> %src, <32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z19work_group_add_utilDv64_mS_(<64 x i64> %src, <64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare float @_Z19work_group_add_utilff(float %src, float %accum) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z19work_group_add_utilDv4_fS_(<4 x float> %src, <4 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z19work_group_add_utilDv8_fS_(<8 x float> %src, <8 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z19work_group_add_utilDv16_fS_(<16 x float> %src, <16 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z19work_group_add_utilDv32_fS_(<32 x float> %src, <32 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z19work_group_add_utilDv64_fS_(<64 x float> %src, <64 x float> %accum) #3

; Function Attrs: nounwind readnone
declare double @_Z19work_group_add_utildd(double %src, double %accum) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z19work_group_add_utilDv4_dS_(<4 x double> %src, <4 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z19work_group_add_utilDv8_dS_(<8 x double> %src, <8 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z19work_group_add_utilDv16_dS_(<16 x double> %src, <16 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z19work_group_add_utilDv32_dS_(<32 x double> %src, <32 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z19work_group_add_utilDv64_dS_(<64 x double> %src, <64 x double> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z14work_group_alli(i32 %predicate) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z14work_group_allDv4_i(<4 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z14work_group_allDv8_i(<8 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z14work_group_allDv16_i(<16 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z14work_group_allDv32_i(<32 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z14work_group_allDv64_i(<64 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare i32 @_Z19work_group_all_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z25__finalize_work_group_allDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z25__finalize_work_group_allDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z25__finalize_work_group_allDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z25__finalize_work_group_allDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z25__finalize_work_group_allDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind
declare i32 @_Z14work_group_alliPi(i32 %predicate, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z14work_group_allDv4_iPS_(<4 x i32> %predicate, <4 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z19work_group_all_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind
declare <8 x i32> @_Z14work_group_allDv8_iPS_(<8 x i32> %predicate, <8 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z19work_group_all_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind
declare <16 x i32> @_Z14work_group_allDv16_iPS_(<16 x i32> %predicate, <16 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z19work_group_all_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind
declare <32 x i32> @_Z14work_group_allDv32_iPS_(<32 x i32> %predicate, <32 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z19work_group_all_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind
declare <64 x i32> @_Z14work_group_allDv64_iPS_(<64 x i32> %predicate, <64 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z19work_group_all_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z14work_group_anyi(i32 %predicate) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z14work_group_anyDv4_i(<4 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z14work_group_anyDv8_i(<8 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z14work_group_anyDv16_i(<16 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z14work_group_anyDv32_i(<32 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z14work_group_anyDv64_i(<64 x i32> %predicate) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z25__finalize_work_group_anyDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z19work_group_any_utilii(i32 %src, i32 %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z25__finalize_work_group_anyDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z25__finalize_work_group_anyDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z25__finalize_work_group_anyDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z25__finalize_work_group_anyDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind
declare i32 @_Z14work_group_anyiPi(i32 %predicate, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z14work_group_anyDv4_iPS_(<4 x i32> %predicate, <4 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z19work_group_any_utilDv4_iS_(<4 x i32> %src, <4 x i32> %accum) #3

; Function Attrs: nounwind
declare <8 x i32> @_Z14work_group_anyDv8_iPS_(<8 x i32> %predicate, <8 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z19work_group_any_utilDv8_iS_(<8 x i32> %src, <8 x i32> %accum) #3

; Function Attrs: nounwind
declare <16 x i32> @_Z14work_group_anyDv16_iPS_(<16 x i32> %predicate, <16 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z19work_group_any_utilDv16_iS_(<16 x i32> %src, <16 x i32> %accum) #3

; Function Attrs: nounwind
declare <32 x i32> @_Z14work_group_anyDv32_iPS_(<32 x i32> %predicate, <32 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z19work_group_any_utilDv32_iS_(<32 x i32> %src, <32 x i32> %accum) #3

; Function Attrs: nounwind
declare <64 x i32> @_Z14work_group_anyDv64_iPS_(<64 x i32> %predicate, <64 x i32>* %accum) #5

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z19work_group_any_utilDv64_iS_(<64 x i32> %src, <64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastim(i32 %src, i64 %local_id) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastij(i32 %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_ij(<4 x i32> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_ij(<8 x i32> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_ij(<16 x i32> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_ij(<32 x i32> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_ij(<64 x i32> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastjj(i32 %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_jj(<4 x i32> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_jj(<8 x i32> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_jj(<16 x i32> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_jj(<32 x i32> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_jj(<64 x i32> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastlj(i64 %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_lj(<4 x i64> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_lj(<8 x i64> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_lj(<16 x i64> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_lj(<32 x i64> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_lj(<64 x i64> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastmj(i64 %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_mj(<4 x i64> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_mj(<8 x i64> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_mj(<16 x i64> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_mj(<32 x i64> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_mj(<64 x i64> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare float @_Z20work_group_broadcastfj(float %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z20work_group_broadcastDv4_fj(<4 x float> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z20work_group_broadcastDv8_fj(<8 x float> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z20work_group_broadcastDv16_fj(<16 x float> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z20work_group_broadcastDv32_fj(<32 x float> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z20work_group_broadcastDv64_fj(<64 x float> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare double @_Z20work_group_broadcastdj(double %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z20work_group_broadcastDv4_dj(<4 x double> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z20work_group_broadcastDv8_dj(<8 x double> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z20work_group_broadcastDv16_dj(<16 x double> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z20work_group_broadcastDv32_dj(<32 x double> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z20work_group_broadcastDv64_dj(<64 x double> %src, i32 %local_id) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastijj(i32 %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_ijj(<4 x i32> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_ijj(<8 x i32> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_ijj(<16 x i32> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_ijj(<32 x i32> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_ijj(<64 x i32> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastjjj(i32 %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_jjj(<4 x i32> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_jjj(<8 x i32> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_jjj(<16 x i32> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_jjj(<32 x i32> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_jjj(<64 x i32> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastljj(i64 %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_ljj(<4 x i64> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_ljj(<8 x i64> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_ljj(<16 x i64> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_ljj(<32 x i64> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_ljj(<64 x i64> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastmjj(i64 %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_mjj(<4 x i64> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_mjj(<8 x i64> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_mjj(<16 x i64> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_mjj(<32 x i64> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_mjj(<64 x i64> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare float @_Z20work_group_broadcastfjj(float %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z20work_group_broadcastDv4_fjj(<4 x float> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z20work_group_broadcastDv8_fjj(<8 x float> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z20work_group_broadcastDv16_fjj(<16 x float> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z20work_group_broadcastDv32_fjj(<32 x float> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z20work_group_broadcastDv64_fjj(<64 x float> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare double @_Z20work_group_broadcastdjj(double %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z20work_group_broadcastDv4_djj(<4 x double> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z20work_group_broadcastDv8_djj(<8 x double> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z20work_group_broadcastDv16_djj(<16 x double> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z20work_group_broadcastDv32_djj(<32 x double> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z20work_group_broadcastDv64_djj(<64 x double> %src, i32 %local_id_x, i32 %local_id_y) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastimjj(i32 %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_ijjj(<4 x i32> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_ijjj(<8 x i32> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_ijjj(<16 x i32> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_ijjj(<32 x i32> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_ijjj(<64 x i32> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare i32 @_Z20work_group_broadcastjjjj(i32 %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z20work_group_broadcastDv4_jjjj(<4 x i32> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z20work_group_broadcastDv8_jjjj(<8 x i32> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z20work_group_broadcastDv16_jjjj(<16 x i32> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z20work_group_broadcastDv32_jjjj(<32 x i32> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z20work_group_broadcastDv64_jjjj(<64 x i32> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastljjj(i64 %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_ljjj(<4 x i64> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_ljjj(<8 x i64> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_ljjj(<16 x i64> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_ljjj(<32 x i64> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_ljjj(<64 x i64> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare i64 @_Z20work_group_broadcastmjjj(i64 %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z20work_group_broadcastDv4_mjjj(<4 x i64> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z20work_group_broadcastDv8_mjjj(<8 x i64> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z20work_group_broadcastDv16_mjjj(<16 x i64> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z20work_group_broadcastDv32_mjjj(<32 x i64> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z20work_group_broadcastDv64_mjjj(<64 x i64> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare float @_Z20work_group_broadcastfjjj(float %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z20work_group_broadcastDv4_fjjj(<4 x float> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z20work_group_broadcastDv8_fjjj(<8 x float> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z20work_group_broadcastDv16_fjjj(<16 x float> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z20work_group_broadcastDv32_fjjj(<32 x float> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z20work_group_broadcastDv64_fjjj(<64 x float> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare double @_Z20work_group_broadcastdjjj(double %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z20work_group_broadcastDv4_djjj(<4 x double> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z20work_group_broadcastDv8_djjj(<8 x double> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z20work_group_broadcastDv16_djjj(<16 x double> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z20work_group_broadcastDv32_djjj(<32 x double> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z20work_group_broadcastDv64_djjj(<64 x double> %src, i32 %local_id_x, i32 %local_id_y, i32 %local_id_z) #3

; Function Attrs: nounwind
declare i32 @_Z20work_group_broadcastijjPi(i32 %src, i32 %linear_local_id, i32 %linear_id, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z20work_group_broadcastDv4_ijjPS_(<4 x i32> %src, i32 %linear_local_id, i32 %linear_id, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z20work_group_broadcastDv8_ijjPS_(<8 x i32> %src, i32 %linear_local_id, i32 %linear_id, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z20work_group_broadcastDv16_ijjPS_(<16 x i32> %src, i32 %linear_local_id, i32 %linear_id, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z20work_group_broadcastDv32_ijjPS_(<32 x i32> %src, i32 %linear_local_id, i32 %linear_id, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z20work_group_broadcastDv64_ijjPS_(<64 x i32> %src, i32 %linear_local_id, i32 %linear_id, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z20work_group_broadcastjjjPj(i32 %src, i32 %linear_local_id, i32 %linear_id, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z20work_group_broadcastDv4_jjjPS_(<4 x i32> %src, i32 %linear_local_id, i32 %linear_id, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z20work_group_broadcastDv8_jjjPS_(<8 x i32> %src, i32 %linear_local_id, i32 %linear_id, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z20work_group_broadcastDv16_jjjPS_(<16 x i32> %src, i32 %linear_local_id, i32 %linear_id, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z20work_group_broadcastDv32_jjjPS_(<32 x i32> %src, i32 %linear_local_id, i32 %linear_id, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z20work_group_broadcastDv64_jjjPS_(<64 x i32> %src, i32 %linear_local_id, i32 %linear_id, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z20work_group_broadcastljjPl(i64 %src, i32 %linear_local_id, i32 %linear_id, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z20work_group_broadcastDv4_ljjPS_(<4 x i64> %src, i32 %linear_local_id, i32 %linear_id, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z20work_group_broadcastDv8_ljjPS_(<8 x i64> %src, i32 %linear_local_id, i32 %linear_id, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z20work_group_broadcastDv16_ljjPS_(<16 x i64> %src, i32 %linear_local_id, i32 %linear_id, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z20work_group_broadcastDv32_ljjPS_(<32 x i64> %src, i32 %linear_local_id, i32 %linear_id, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z20work_group_broadcastDv64_ljjPS_(<64 x i64> %src, i32 %linear_local_id, i32 %linear_id, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z20work_group_broadcastmjjPm(i64 %src, i32 %linear_local_id, i32 %linear_id, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z20work_group_broadcastDv4_mjjPS_(<4 x i64> %src, i32 %linear_local_id, i32 %linear_id, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z20work_group_broadcastDv8_mjjPS_(<8 x i64> %src, i32 %linear_local_id, i32 %linear_id, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z20work_group_broadcastDv16_mjjPS_(<16 x i64> %src, i32 %linear_local_id, i32 %linear_id, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z20work_group_broadcastDv32_mjjPS_(<32 x i64> %src, i32 %linear_local_id, i32 %linear_id, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z20work_group_broadcastDv64_mjjPS_(<64 x i64> %src, i32 %linear_local_id, i32 %linear_id, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z20work_group_broadcastfjjPf(float %src, i32 %linear_local_id, i32 %linear_id, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z20work_group_broadcastDv4_fjjPS_(<4 x float> %src, i32 %linear_local_id, i32 %linear_id, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z20work_group_broadcastDv8_fjjPS_(<8 x float> %src, i32 %linear_local_id, i32 %linear_id, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z20work_group_broadcastDv16_fjjPS_(<16 x float> %src, i32 %linear_local_id, i32 %linear_id, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z20work_group_broadcastDv32_fjjPS_(<32 x float> %src, i32 %linear_local_id, i32 %linear_id, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z20work_group_broadcastDv64_fjjPS_(<64 x float> %src, i32 %linear_local_id, i32 %linear_id, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z20work_group_broadcastdjjPd(double %src, i32 %linear_local_id, i32 %linear_id, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z20work_group_broadcastDv4_djjPS_(<4 x double> %src, i32 %linear_local_id, i32 %linear_id, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z20work_group_broadcastDv8_djjPS_(<8 x double> %src, i32 %linear_local_id, i32 %linear_id, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z20work_group_broadcastDv16_djjPS_(<16 x double> %src, i32 %linear_local_id, i32 %linear_id, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z20work_group_broadcastDv32_djjPS_(<32 x double> %src, i32 %linear_local_id, i32 %linear_id, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z20work_group_broadcastDv64_djjPS_(<64 x double> %src, i32 %linear_local_id, i32 %linear_id, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_addi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_addDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_addDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_addDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_addDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_addDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_addj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_addDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_addDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_addDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_addDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_addDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_addl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_addDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_addDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_addDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_addDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_addDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_addm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_addDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_addDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_addDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_addDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_addDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z21work_group_reduce_addf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z21work_group_reduce_addDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z21work_group_reduce_addDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z21work_group_reduce_addDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z21work_group_reduce_addDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z21work_group_reduce_addDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z21work_group_reduce_addd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z21work_group_reduce_addDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z21work_group_reduce_addDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z21work_group_reduce_addDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z21work_group_reduce_addDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z21work_group_reduce_addDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_addDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_addDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_addDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_addDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_addDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_addDv4_j(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_addDv8_j(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_addDv16_j(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_addDv32_j(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_addDv64_j(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_addDv4_l(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_addDv8_l(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_addDv16_l(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_addDv32_l(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_addDv64_l(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_addDv4_m(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_addDv8_m(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_addDv16_m(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_addDv32_m(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_addDv64_m(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z32__finalize_work_group_reduce_addDv4_f(<4 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z32__finalize_work_group_reduce_addDv8_f(<8 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z32__finalize_work_group_reduce_addDv16_f(<16 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z32__finalize_work_group_reduce_addDv32_f(<32 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z32__finalize_work_group_reduce_addDv64_f(<64 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z32__finalize_work_group_reduce_addDv4_d(<4 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z32__finalize_work_group_reduce_addDv8_d(<8 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z32__finalize_work_group_reduce_addDv16_d(<16 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z32__finalize_work_group_reduce_addDv32_d(<32 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z32__finalize_work_group_reduce_addDv64_d(<64 x double> %accum) #3

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_addiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_addDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_addDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_addDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_addDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_addDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_addjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_addDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_addDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_addDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_addDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_addDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_addlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_addDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_addDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_addDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_addDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_addDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_addmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_addDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_addDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_addDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_addDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_addDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z21work_group_reduce_addfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z21work_group_reduce_addDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z21work_group_reduce_addDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z21work_group_reduce_addDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z21work_group_reduce_addDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z21work_group_reduce_addDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z21work_group_reduce_adddPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z21work_group_reduce_addDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z21work_group_reduce_addDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z21work_group_reduce_addDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z21work_group_reduce_addDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z21work_group_reduce_addDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_maxi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_maxDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_maxDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_maxDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_maxDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_maxDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_maxj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_maxDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_maxDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_maxDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_maxDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_maxDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_maxl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_maxDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_maxDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_maxDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_maxDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_maxDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_maxm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_maxDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_maxDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_maxDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_maxDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_maxDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z21work_group_reduce_maxf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z21work_group_reduce_maxDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z21work_group_reduce_maxDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z21work_group_reduce_maxDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z21work_group_reduce_maxDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z21work_group_reduce_maxDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z21work_group_reduce_maxd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z21work_group_reduce_maxDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z21work_group_reduce_maxDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z21work_group_reduce_maxDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z21work_group_reduce_maxDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z21work_group_reduce_maxDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_maxDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_maxDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_maxDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_maxDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_maxDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_maxDv4_j(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_maxDv8_j(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_maxDv16_j(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_maxDv32_j(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_maxDv64_j(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_maxDv4_l(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_maxDv8_l(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_maxDv16_l(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_maxDv32_l(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_maxDv64_l(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_maxDv4_m(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_maxDv8_m(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_maxDv16_m(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_maxDv32_m(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_maxDv64_m(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z32__finalize_work_group_reduce_maxDv4_f(<4 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z32__finalize_work_group_reduce_maxDv8_f(<8 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z32__finalize_work_group_reduce_maxDv16_f(<16 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z32__finalize_work_group_reduce_maxDv32_f(<32 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z32__finalize_work_group_reduce_maxDv64_f(<64 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z32__finalize_work_group_reduce_maxDv4_d(<4 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z32__finalize_work_group_reduce_maxDv8_d(<8 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z32__finalize_work_group_reduce_maxDv16_d(<16 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z32__finalize_work_group_reduce_maxDv32_d(<32 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z32__finalize_work_group_reduce_maxDv64_d(<64 x double> %accum) #3

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_maxiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_maxDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_maxDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_maxDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_maxDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_maxDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_maxjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_maxDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_maxDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_maxDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_maxDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_maxDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_maxlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_maxDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_maxDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_maxDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_maxDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_maxDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_maxmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_maxDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_maxDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_maxDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_maxDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_maxDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z21work_group_reduce_maxfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z21work_group_reduce_maxDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z21work_group_reduce_maxDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z21work_group_reduce_maxDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z21work_group_reduce_maxDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z21work_group_reduce_maxDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z21work_group_reduce_maxdPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z21work_group_reduce_maxDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z21work_group_reduce_maxDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z21work_group_reduce_maxDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z21work_group_reduce_maxDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z21work_group_reduce_maxDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_mini(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_minDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_minDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_minDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_minDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_minDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z21work_group_reduce_minj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z21work_group_reduce_minDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z21work_group_reduce_minDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z21work_group_reduce_minDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z21work_group_reduce_minDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z21work_group_reduce_minDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_minl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_minDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_minDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_minDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_minDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_minDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z21work_group_reduce_minm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z21work_group_reduce_minDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z21work_group_reduce_minDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z21work_group_reduce_minDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z21work_group_reduce_minDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z21work_group_reduce_minDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z21work_group_reduce_minf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z21work_group_reduce_minDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z21work_group_reduce_minDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z21work_group_reduce_minDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z21work_group_reduce_minDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z21work_group_reduce_minDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z21work_group_reduce_mind(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z21work_group_reduce_minDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z21work_group_reduce_minDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z21work_group_reduce_minDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z21work_group_reduce_minDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z21work_group_reduce_minDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_minDv4_i(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_minDv8_i(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_minDv16_i(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_minDv32_i(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_minDv64_i(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z32__finalize_work_group_reduce_minDv4_j(<4 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z32__finalize_work_group_reduce_minDv8_j(<8 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z32__finalize_work_group_reduce_minDv16_j(<16 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z32__finalize_work_group_reduce_minDv32_j(<32 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z32__finalize_work_group_reduce_minDv64_j(<64 x i32> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_minDv4_l(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_minDv8_l(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_minDv16_l(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_minDv32_l(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_minDv64_l(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z32__finalize_work_group_reduce_minDv4_m(<4 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z32__finalize_work_group_reduce_minDv8_m(<8 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z32__finalize_work_group_reduce_minDv16_m(<16 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z32__finalize_work_group_reduce_minDv32_m(<32 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z32__finalize_work_group_reduce_minDv64_m(<64 x i64> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z32__finalize_work_group_reduce_minDv4_f(<4 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z32__finalize_work_group_reduce_minDv8_f(<8 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z32__finalize_work_group_reduce_minDv16_f(<16 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z32__finalize_work_group_reduce_minDv32_f(<32 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z32__finalize_work_group_reduce_minDv64_f(<64 x float> %accum) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z32__finalize_work_group_reduce_minDv4_d(<4 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z32__finalize_work_group_reduce_minDv8_d(<8 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z32__finalize_work_group_reduce_minDv16_d(<16 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z32__finalize_work_group_reduce_minDv32_d(<32 x double> %accum) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z32__finalize_work_group_reduce_minDv64_d(<64 x double> %accum) #3

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_miniPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_minDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_minDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_minDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_minDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_minDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z21work_group_reduce_minjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z21work_group_reduce_minDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z21work_group_reduce_minDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z21work_group_reduce_minDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z21work_group_reduce_minDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z21work_group_reduce_minDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_minlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_minDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_minDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_minDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_minDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_minDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z21work_group_reduce_minmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z21work_group_reduce_minDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z21work_group_reduce_minDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z21work_group_reduce_minDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z21work_group_reduce_minDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z21work_group_reduce_minDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z21work_group_reduce_minfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z21work_group_reduce_minDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z21work_group_reduce_minDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z21work_group_reduce_minDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z21work_group_reduce_minDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z21work_group_reduce_minDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z21work_group_reduce_mindPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z21work_group_reduce_minDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z21work_group_reduce_minDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z21work_group_reduce_minDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z21work_group_reduce_minDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z21work_group_reduce_minDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_addi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_addDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_addDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_addDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_addDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_addDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_addj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_addDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_addDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_addDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_addDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_addDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_addl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_addDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_addDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_addDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_addDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_addDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_addm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_addDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_addDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_addDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_addDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_addDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_exclusive_addf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_exclusive_addDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_exclusive_addDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_exclusive_addDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_exclusive_addDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_exclusive_addDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_exclusive_addd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_exclusive_addDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_exclusive_addDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_exclusive_addDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_exclusive_addDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_exclusive_addDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_addiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_addDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_addDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_addDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_addDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_addDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_addjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_addDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_addDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_addDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_addDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_addDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_addlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_addDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_addDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_addDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_addDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_addDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_addmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_addDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_addDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_addDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_addDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_addDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_exclusive_addfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_exclusive_addDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_exclusive_addDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_exclusive_addDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_exclusive_addDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_exclusive_addDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_exclusive_adddPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_exclusive_addDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_exclusive_addDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_exclusive_addDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_exclusive_addDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_exclusive_addDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_maxi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_maxDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_maxDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_maxDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_maxDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_maxj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_maxDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_maxDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_maxDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_maxDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_maxl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_maxDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_maxDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_maxDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_maxDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_maxDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_maxm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_maxDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_maxDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_maxDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_maxDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_maxDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_exclusive_maxf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_exclusive_maxDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_exclusive_maxDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_exclusive_maxDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_exclusive_maxDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_exclusive_maxDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_exclusive_maxd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_exclusive_maxDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_exclusive_maxDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_exclusive_maxDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_exclusive_maxDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_exclusive_maxDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_maxiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_maxDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_maxDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_maxDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_maxDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_maxjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_maxDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_maxDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_maxDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_maxDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_maxDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_maxlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_maxDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_maxDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_maxDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_maxDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_maxDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_maxmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_maxDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_maxDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_maxDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_maxDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_maxDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_exclusive_maxfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_exclusive_maxDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_exclusive_maxDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_exclusive_maxDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_exclusive_maxDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_exclusive_maxDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_exclusive_maxdPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_exclusive_maxDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_exclusive_maxDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_exclusive_maxDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_exclusive_maxDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_exclusive_maxDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_mini(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_minDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_minDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_minDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_minDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_minDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_exclusive_minj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_exclusive_minDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_exclusive_minDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_exclusive_minDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_exclusive_minDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_exclusive_minDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_minl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_minDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_minDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_minDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_minDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_minDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_exclusive_minm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_exclusive_minDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_exclusive_minDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_exclusive_minDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_exclusive_minDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_exclusive_minDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_exclusive_minf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_exclusive_minDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_exclusive_minDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_exclusive_minDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_exclusive_minDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_exclusive_minDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_exclusive_mind(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_exclusive_minDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_exclusive_minDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_exclusive_minDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_exclusive_minDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_exclusive_minDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_miniPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_minDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_minDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_minDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_minDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_minDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_exclusive_minjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_exclusive_minDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_exclusive_minDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_exclusive_minDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_exclusive_minDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_exclusive_minDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_minlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_minDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_minDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_minDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_minDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_minDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_exclusive_minmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_exclusive_minDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_exclusive_minDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_exclusive_minDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_exclusive_minDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_exclusive_minDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_exclusive_minfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_exclusive_minDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_exclusive_minDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_exclusive_minDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_exclusive_minDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_exclusive_minDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_exclusive_mindPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_exclusive_minDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_exclusive_minDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_exclusive_minDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_exclusive_minDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_exclusive_minDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_addi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_addDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_addDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_addDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_addDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_addDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_addj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_addDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_addDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_addDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_addDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_addDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_addl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_addDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_addDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_addDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_addDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_addDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_addm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_addDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_addDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_addDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_addDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_addDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_inclusive_addf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_inclusive_addDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_inclusive_addDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_inclusive_addDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_inclusive_addDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_inclusive_addDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_inclusive_addd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_inclusive_addDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_inclusive_addDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_inclusive_addDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_inclusive_addDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_inclusive_addDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_addiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_addDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_addDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_addDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_addDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_addDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_addjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_addDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_addDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_addDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_addDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_addDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_addlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_addDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_addDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_addDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_addDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_addDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_addmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_addDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_addDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_addDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_addDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_addDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_inclusive_addfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_inclusive_addDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_inclusive_addDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_inclusive_addDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_inclusive_addDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_inclusive_addDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_inclusive_adddPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_inclusive_addDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_inclusive_addDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_inclusive_addDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_inclusive_addDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_inclusive_addDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_maxi(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_maxDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_maxDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_maxDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_maxDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_maxDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_maxj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_maxDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_maxDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_maxDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_maxDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_maxDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_maxl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_maxDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_maxDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_maxDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_maxDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_maxDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_maxm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_maxDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_maxDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_maxDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_maxDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_maxDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_inclusive_maxf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_inclusive_maxDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_inclusive_maxDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_inclusive_maxDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_inclusive_maxDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_inclusive_maxDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_inclusive_maxd(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_inclusive_maxDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_inclusive_maxDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_inclusive_maxDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_inclusive_maxDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_inclusive_maxDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_maxiPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_maxDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_maxDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_maxDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_maxDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_maxDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_maxjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_maxDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_maxDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_maxDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_maxDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_maxDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_maxlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_maxDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_maxDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_maxDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_maxDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_maxDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_maxmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_maxDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_maxDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_maxDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_maxDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_maxDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_inclusive_maxfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_inclusive_maxDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_inclusive_maxDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_inclusive_maxDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_inclusive_maxDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_inclusive_maxDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_inclusive_maxdPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_inclusive_maxDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_inclusive_maxDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_inclusive_maxDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_inclusive_maxDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_inclusive_maxDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_mini(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_minDv4_i(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_minDv8_i(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_minDv16_i(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_minDv32_i(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_minDv64_i(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i32 @_Z29work_group_scan_inclusive_minj(i32 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i32> @_Z29work_group_scan_inclusive_minDv4_j(<4 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i32> @_Z29work_group_scan_inclusive_minDv8_j(<8 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i32> @_Z29work_group_scan_inclusive_minDv16_j(<16 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i32> @_Z29work_group_scan_inclusive_minDv32_j(<32 x i32> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i32> @_Z29work_group_scan_inclusive_minDv64_j(<64 x i32> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_minl(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_minDv4_l(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_minDv8_l(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_minDv16_l(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_minDv32_l(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_minDv64_l(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare i64 @_Z29work_group_scan_inclusive_minm(i64 %src) #3

; Function Attrs: nounwind readnone
declare <4 x i64> @_Z29work_group_scan_inclusive_minDv4_m(<4 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <8 x i64> @_Z29work_group_scan_inclusive_minDv8_m(<8 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <16 x i64> @_Z29work_group_scan_inclusive_minDv16_m(<16 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <32 x i64> @_Z29work_group_scan_inclusive_minDv32_m(<32 x i64> %src) #3

; Function Attrs: nounwind readnone
declare <64 x i64> @_Z29work_group_scan_inclusive_minDv64_m(<64 x i64> %src) #3

; Function Attrs: nounwind readnone
declare float @_Z29work_group_scan_inclusive_minf(float %src) #3

; Function Attrs: nounwind readnone
declare <4 x float> @_Z29work_group_scan_inclusive_minDv4_f(<4 x float> %src) #3

; Function Attrs: nounwind readnone
declare <8 x float> @_Z29work_group_scan_inclusive_minDv8_f(<8 x float> %src) #3

; Function Attrs: nounwind readnone
declare <16 x float> @_Z29work_group_scan_inclusive_minDv16_f(<16 x float> %src) #3

; Function Attrs: nounwind readnone
declare <32 x float> @_Z29work_group_scan_inclusive_minDv32_f(<32 x float> %src) #3

; Function Attrs: nounwind readnone
declare <64 x float> @_Z29work_group_scan_inclusive_minDv64_f(<64 x float> %src) #3

; Function Attrs: nounwind readnone
declare double @_Z29work_group_scan_inclusive_mind(double %src) #3

; Function Attrs: nounwind readnone
declare <4 x double> @_Z29work_group_scan_inclusive_minDv4_d(<4 x double> %src) #3

; Function Attrs: nounwind readnone
declare <8 x double> @_Z29work_group_scan_inclusive_minDv8_d(<8 x double> %src) #3

; Function Attrs: nounwind readnone
declare <16 x double> @_Z29work_group_scan_inclusive_minDv16_d(<16 x double> %src) #3

; Function Attrs: nounwind readnone
declare <32 x double> @_Z29work_group_scan_inclusive_minDv32_d(<32 x double> %src) #3

; Function Attrs: nounwind readnone
declare <64 x double> @_Z29work_group_scan_inclusive_minDv64_d(<64 x double> %src) #3

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_miniPi(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_minDv4_iPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_minDv8_iPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_minDv16_iPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_minDv32_iPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_minDv64_iPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i32 @_Z29work_group_scan_inclusive_minjPj(i32 %src, i32* %accum) #5

; Function Attrs: nounwind
declare <4 x i32> @_Z29work_group_scan_inclusive_minDv4_jPS_(<4 x i32> %src, <4 x i32>* %accum) #5

; Function Attrs: nounwind
declare <8 x i32> @_Z29work_group_scan_inclusive_minDv8_jPS_(<8 x i32> %src, <8 x i32>* %accum) #5

; Function Attrs: nounwind
declare <16 x i32> @_Z29work_group_scan_inclusive_minDv16_jPS_(<16 x i32> %src, <16 x i32>* %accum) #5

; Function Attrs: nounwind
declare <32 x i32> @_Z29work_group_scan_inclusive_minDv32_jPS_(<32 x i32> %src, <32 x i32>* %accum) #5

; Function Attrs: nounwind
declare <64 x i32> @_Z29work_group_scan_inclusive_minDv64_jPS_(<64 x i32> %src, <64 x i32>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_minlPl(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_minDv4_lPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_minDv8_lPS_(<8 x i64> %src, <8 x i64>* %accum) #5

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_minDv16_lPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_minDv32_lPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_minDv64_lPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare i64 @_Z29work_group_scan_inclusive_minmPm(i64 %src, i64* %accum) #5

; Function Attrs: nounwind
declare <4 x i64> @_Z29work_group_scan_inclusive_minDv4_mPS_(<4 x i64> %src, <4 x i64>* %accum) #5

; Function Attrs: nounwind
declare <8 x i64> @_Z29work_group_scan_inclusive_minDv8_mPS_(<8 x i64> %src, <8 x i64>* %accum) #5 

; Function Attrs: nounwind
declare <16 x i64> @_Z29work_group_scan_inclusive_minDv16_mPS_(<16 x i64> %src, <16 x i64>* %accum) #5

; Function Attrs: nounwind
declare <32 x i64> @_Z29work_group_scan_inclusive_minDv32_mPS_(<32 x i64> %src, <32 x i64>* %accum) #5

; Function Attrs: nounwind
declare <64 x i64> @_Z29work_group_scan_inclusive_minDv64_mPS_(<64 x i64> %src, <64 x i64>* %accum) #5

; Function Attrs: nounwind
declare float @_Z29work_group_scan_inclusive_minfPf(float %src, float* %accum) #5

; Function Attrs: nounwind
declare <4 x float> @_Z29work_group_scan_inclusive_minDv4_fPS_(<4 x float> %src, <4 x float>* %accum) #5

; Function Attrs: nounwind
declare <8 x float> @_Z29work_group_scan_inclusive_minDv8_fPS_(<8 x float> %src, <8 x float>* %accum) #5

; Function Attrs: nounwind
declare <16 x float> @_Z29work_group_scan_inclusive_minDv16_fPS_(<16 x float> %src, <16 x float>* %accum) #5

; Function Attrs: nounwind
declare <32 x float> @_Z29work_group_scan_inclusive_minDv32_fPS_(<32 x float> %src, <32 x float>* %accum) #5

; Function Attrs: nounwind
declare <64 x float> @_Z29work_group_scan_inclusive_minDv64_fPS_(<64 x float> %src, <64 x float>* %accum) #5

; Function Attrs: nounwind
declare double @_Z29work_group_scan_inclusive_mindPd(double %src, double* %accum) #5

; Function Attrs: nounwind
declare <4 x double> @_Z29work_group_scan_inclusive_minDv4_dPS_(<4 x double> %src, <4 x double>* %accum) #5

; Function Attrs: nounwind
declare <8 x double> @_Z29work_group_scan_inclusive_minDv8_dPS_(<8 x double> %src, <8 x double>* %accum) #5

; Function Attrs: nounwind
declare <16 x double> @_Z29work_group_scan_inclusive_minDv16_dPS_(<16 x double> %src, <16 x double>* %accum) #5

; Function Attrs: nounwind
declare <32 x double> @_Z29work_group_scan_inclusive_minDv32_dPS_(<32 x double> %src, <32 x double>* %accum) #5

; Function Attrs: nounwind
declare <64 x double> @_Z29work_group_scan_inclusive_minDv64_dPS_(<64 x double> %src, <64 x double>* %accum) #5


attributes #3 = { nounwind readnone }
attributes #5 = { nounwind }

; DEBUGIFY-NOT: WARNING
