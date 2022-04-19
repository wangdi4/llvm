// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -target-feature +avx512bw -fms-compatibility -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-windows -target-feature +avx512bw -fms-compatibility -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-linux -target-feature +avx512bw -fms-compatibility -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple i686-pc-win32 -target-feature +avx512bw -fms-compatibility -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s
// Check SVML calling conventions are assigned correctly when compiling 512-bit SVML functions.

typedef float __m512 __attribute__((__vector_size__(64), __aligned__(64)));
typedef double __m512d __attribute__((__vector_size__(64), __aligned__(64)));

typedef unsigned short __mmask16;

typedef struct __m512x2_t { __m512 v1; __m512 v2; } __m512x2;

int __svml_ssin_cout_rare_internal(const float* a, float* r);

// CHECK: define dso_local svml_unified_cc_512 <16 x float> @__svml_sinf16_mask_z0(<16 x float> noundef %{{.*}}, <16 x i1> noundef %{{.*}}, <16 x float> noundef %{{.*}})
__attribute__((intel_ocl_bicc))
__m512 __svml_sinf16_mask_z0(__m512 r, __mmask16 mask, __m512 a) {
  __m512 ret;
  float *_vapi_arg1 = (float *)&r;
  float *_vapi_res1 = (float *)&ret;
// CHECK: call i32 @__svml_ssin_cout_rare_internal(
  for (int m = 0; m < 16; m++)
    __svml_ssin_cout_rare_internal(_vapi_arg1 + m, _vapi_res1 + m);
  return ret;
}

typedef __attribute__((intel_ocl_bicc)) __m512 (*core_func_ptr)(__m512 r, __mmask16 mask, __m512 a);

core_func_ptr * __svml_sinf16_mask_chosen_core_func_get_ptr_internal(void);

__attribute__((visibility("hidden"))) extern core_func_ptr * __svml_sinf16_mask_chosen_core_func_get_ptr_internal(void);

// CHECK: define dso_local svml_unified_cc_512 <16 x float> @__svml_sinf16_mask_chosen_core_func_init_internal(<16 x float> noundef %{{.*}}, <16 x i1> noundef %{{.*}}, <16 x float> noundef %{{.*}})
__attribute__((intel_ocl_bicc))
__m512 __svml_sinf16_mask_chosen_core_func_init_internal(__m512 r, __mmask16 mask, __m512 a) {
// CHECK: call <16 x float> (<16 x float>, <16 x i1>, <16 x float>)** @__svml_sinf16_mask_chosen_core_func_get_ptr_internal()
  core_func_ptr *func_ptr = __svml_sinf16_mask_chosen_core_func_get_ptr_internal();
// CHECK: call svml_unified_cc_512 <16 x float> {{.*}}(<16 x float> noundef %{{.*}}, <16 x i1> noundef %{{.*}}, <16 x float> noundef %{{.*}})
  return (*func_ptr)(r, mask, a);
}

__declspec(align(64)) static core_func_ptr __svml_sinf16_mask_chosen_core_func = (core_func_ptr)(&__svml_sinf16_mask_chosen_core_func_init_internal);

// CHECK: define hidden <16 x float> (<16 x float>, <16 x i1>, <16 x float>)** @__svml_sinf16_mask_chosen_core_func_get_ptr_internal()
core_func_ptr *__svml_sinf16_mask_chosen_core_func_get_ptr_internal(void) {
  return &__svml_sinf16_mask_chosen_core_func;
}

// CHECK: define dso_local svml_unified_cc_512 <16 x float> @__svml_sinf16_mask(<16 x float> noundef %{{.*}}, <16 x i1> noundef %{{.*}}, <16 x float> noundef %{{.*}})
__attribute__((intel_ocl_bicc))
__m512 __svml_sinf16_mask(__m512 r, __mmask16 mask, __m512 a) {
  return (__svml_sinf16_mask_chosen_core_func)(r, mask, a);
}

// CHECK: define dso_local svml_unified_cc_512 <8 x double> @__svml_pow3o28_mask_l0(<8 x double> noundef %{{.*}}, <16 x i1> noundef %{{.*}}, <8 x double> noundef %{{.*}})
__attribute__((intel_ocl_bicc))
__m512d __svml_pow3o28_mask_l0(__m512d r, __mmask16 mask, __m512d a) {
    return a;
}

// CHECK: define dso_local svml_unified_cc_512 %struct.__m512x2_t @__svml_sincospif16_mask_z0(<16 x float> %{{.*}}, <16 x float> %{{.*}}, <16 x i1> noundef %{{.*}}, <16 x float> noundef %{{.*}})
__attribute__((intel_ocl_bicc))
__m512x2 __svml_sincospif16_mask_z0(__m512x2 r, __mmask16 mask, __m512 a) {
  return r;
}

// CHECK: define dso_local svml_unified_cc_512 %struct.__m512x2_t @__svml_cdivf16_mask_l0(<16 x float> %{{.*}}, <16 x float> %{{.*}}, <16 x i1> noundef %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}}, <16 x float> %{{.*}})
__attribute__((intel_ocl_bicc))
__m512x2 __svml_cdivf16_mask_l0(__m512x2 r, __mmask16 mask, __m512x2 a, __m512x2 b) {
  return a;
}
