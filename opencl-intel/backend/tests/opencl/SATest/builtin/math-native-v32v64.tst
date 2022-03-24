; Check that v32 and v64(expand from v32) builtins are used.

; RUN: SATest -BUILD --config=%s.cfg -tsize=32 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefix=CHECK32
; RUN: SATest -BUILD --config=%s.cfg -tsize=64 -cpuarch="skx" -llvm-option=-print-after=vplan-vec 2>&1 | FileCheck %s -check-prefix=CHECK64
; RUN: SATest -BUILD --config=%s.cfg -tsize=32 -cpuarch="skx" --dump-llvm-file - | FileCheck %s
; RUN: SATest -BUILD --config=%s.cfg -tsize=64 -cpuarch="skx" --dump-llvm-file - | FileCheck %s

; CHECK32: call{{.*}} <32 x float> @_Z11native_acosDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_acoshDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z13native_acospiDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_asinDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_asinhDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z13native_asinpiDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_atanDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_atan2Dv32_fS_(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_atanhDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z13native_atanpiDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z14native_atan2piDv32_fS_(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_cbrtDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z10native_cosDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_coshDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_cospiDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_erfcDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z10native_erfDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z10native_expDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_exp2Dv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_exp10Dv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_expm1Dv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_fdimDv32_fS_(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_fmaxDv32_fS_(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_fmaxDv32_fS_(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_fminDv32_fS_(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_fminDv32_fS_(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_fmodDv32_fS_(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_hypotDv32_fS_(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_ilogbDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z10native_logDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_log2Dv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_log10Dv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_log1pDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z10native_powDv32_fS_(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_pownDv32_fDv32_i(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_powrDv32_fS_(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_rootnDv32_fDv32_i(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_rsqrtDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z10native_sinDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_sinhDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_sinpiDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_sqrtDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z10native_tanDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z11native_tanhDv32_f(<32 x float>
; CHECK32: call{{.*}} <32 x float> @_Z12native_tanpiDv32_f(<32 x float>

; CHECK64: call{{.*}} <64 x float> @_Z11native_acosDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_acoshDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z13native_acospiDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_asinDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_asinhDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z13native_asinpiDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_atanDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_atan2Dv64_fS_(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_atanhDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z13native_atanpiDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z14native_atan2piDv64_fS_(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_cbrtDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z10native_cosDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_coshDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_cospiDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_erfcDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z10native_erfDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z10native_expDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_exp2Dv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_exp10Dv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_expm1Dv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_fdimDv64_fS_(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_fmaxDv64_fS_(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_fmaxDv64_fS_(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_fminDv64_fS_(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_fminDv64_fS_(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_fmodDv64_fS_(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_hypotDv64_fS_(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_ilogbDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z10native_logDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_log2Dv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_log10Dv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_log1pDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z10native_powDv64_fS_(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_pownDv64_fDv64_i(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_powrDv64_fS_(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_rootnDv64_fDv64_i(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_rsqrtDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z10native_sinDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_sinhDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_sinpiDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_sqrtDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z10native_tanDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z11native_tanhDv64_f(<64 x float>
; CHECK64: call{{.*}} <64 x float> @_Z12native_tanpiDv64_f(<64 x float>

; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_acosf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_acoshf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_acospif32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_asinf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_asinhf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_asinpif32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_atanf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_atan2f32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_atanhf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_atanpif32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_atan2pif32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_cbrtf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_cosf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_coshf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_cospif32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_erfcf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_erff32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_expf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_exp2f32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_exp10f32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_expm1f32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_truncf32(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_sqrtf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_logf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_log2f32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_log10f32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_log1pf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_powf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_pownf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_powrf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_rootnf32_native(<32 x float>
; CHECK: call{{.*}} <16 x float> @llvm.x86.avx512.rsqrt14.ps.512(<16 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_sinf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_sinhf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_sinpif32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_sqrtf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_tanf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_tanhf32_native(<32 x float>
; CHECK: call{{.*}} intel_ocl_bicc_avx512 <32 x float> @__ocl_svml_{{[xz]}}0_tanpif32_native(<32 x float>
