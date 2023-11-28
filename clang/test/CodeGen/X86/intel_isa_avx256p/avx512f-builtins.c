// REQUIRES: intel_feature_isa_avx256p_unsupported
// RUN: %clang_cc1 -flax-vector-conversions=none -ffreestanding %s -triple=x86_64-apple-darwin -target-feature +avx256p -emit-llvm -o - -Wall -Werror -Wsign-conversion | FileCheck %s
// RUN: %clang_cc1 -flax-vector-conversions=none -fms-extensions -fms-compatibility -ffreestanding %s -triple=x86_64-windows-msvc -target-feature +avx256p -emit-llvm -o - -Wall -Werror -Wsign-conversion | FileCheck %s
#include <immintrin.h>

__m128 test_mm_add_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_add_round_ss
  // CHECK: @llvm.x86.avx512.mask.add.ss.round
  return _mm_add_round_ss(__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_mask_add_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_add_round_ss
  // CHECK: @llvm.x86.avx512.mask.add.ss.round
  return _mm_mask_add_round_ss(__W,__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_maskz_add_round_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_add_round_ss
  // CHECK: @llvm.x86.avx512.mask.add.ss.round
  return _mm_maskz_add_round_ss(__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_mask_add_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_add_ss
  // CHECK-NOT: @llvm.x86.avx512.mask.add.ss.round
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: fadd float %{{.*}}, %{{.*}}
  // CHECK: insertelement <4 x float> %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, float %{{.*}}, float %{{.*}}
  // CHECK-NEXT: insertelement <4 x float> %{{.*}}, float %{{.*}}, i64 0
  return _mm_mask_add_ss(__W,__U,__A,__B); 
}
__m128 test_mm_maskz_add_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_add_ss
  // CHECK-NOT: @llvm.x86.avx512.mask.add.ss.round
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: fadd float %{{.*}}, %{{.*}}
  // CHECK: insertelement <4 x float> %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, float %{{.*}}, float %{{.*}}
  // CHECK-NEXT: insertelement <4 x float> %{{.*}}, float %{{.*}}, i64 0
  return _mm_maskz_add_ss(__U,__A,__B); 
}
__m128d test_mm_add_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_add_round_sd
  // CHECK: @llvm.x86.avx512.mask.add.sd.round
  return _mm_add_round_sd(__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_mask_add_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_add_round_sd
  // CHECK: @llvm.x86.avx512.mask.add.sd.round
  return _mm_mask_add_round_sd(__W,__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_maskz_add_round_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_add_round_sd
  // CHECK: @llvm.x86.avx512.mask.add.sd.round
  return _mm_maskz_add_round_sd(__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_mask_add_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_add_sd
  // CHECK-NOT: @llvm.x86.avx512.mask.add.sd.round
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: fadd double %{{.*}}, %{{.*}}
  // CHECK: insertelement <2 x double> {{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, double %{{.*}}, double %{{.*}}
  // CHECK-NEXT: insertelement <2 x double> %{{.*}}, double %{{.*}}, i64 0
  return _mm_mask_add_sd(__W,__U,__A,__B); 
}
__m128d test_mm_maskz_add_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_add_sd
  // CHECK-NOT: @llvm.x86.avx512.mask.add.sd.round
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: fadd double %{{.*}}, %{{.*}}
  // CHECK: insertelement <2 x double> {{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, double %{{.*}}, double %{{.*}}
  // CHECK-NEXT: insertelement <2 x double> %{{.*}}, double %{{.*}}, i64 0
  return _mm_maskz_add_sd(__U,__A,__B); 
}
__m128 test_mm_sub_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_sub_round_ss
  // CHECK: @llvm.x86.avx512.mask.sub.ss.round
  return _mm_sub_round_ss(__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_mask_sub_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_sub_round_ss
  // CHECK: @llvm.x86.avx512.mask.sub.ss.round
  return _mm_mask_sub_round_ss(__W,__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_maskz_sub_round_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_sub_round_ss
  // CHECK: @llvm.x86.avx512.mask.sub.ss.round
  return _mm_maskz_sub_round_ss(__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_mask_sub_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_sub_ss
  // CHECK-NOT: @llvm.x86.avx512.mask.sub.ss.round
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: fsub float %{{.*}}, %{{.*}}
  // CHECK: insertelement <4 x float> {{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, float %{{.*}}, float %{{.*}}
  // CHECK-NEXT: insertelement <4 x float> %{{.*}}, float %{{.*}}, i64 0
  return _mm_mask_sub_ss(__W,__U,__A,__B); 
}
__m128 test_mm_maskz_sub_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_sub_ss
  // CHECK-NOT: @llvm.x86.avx512.mask.sub.ss.round
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: fsub float %{{.*}}, %{{.*}}
  // CHECK: insertelement <4 x float> {{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, float %{{.*}}, float %{{.*}}
  // CHECK-NEXT: insertelement <4 x float> %{{.*}}, float %{{.*}}, i64 0
  return _mm_maskz_sub_ss(__U,__A,__B); 
}
__m128d test_mm_sub_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_sub_round_sd
  // CHECK: @llvm.x86.avx512.mask.sub.sd.round
  return _mm_sub_round_sd(__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_mask_sub_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_sub_round_sd
  // CHECK: @llvm.x86.avx512.mask.sub.sd.round
  return _mm_mask_sub_round_sd(__W,__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_maskz_sub_round_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_sub_round_sd
  // CHECK: @llvm.x86.avx512.mask.sub.sd.round
  return _mm_maskz_sub_round_sd(__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_mask_sub_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_sub_sd
  // CHECK-NOT: @llvm.x86.avx512.mask.sub.sd.round
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: fsub double %{{.*}}, %{{.*}}
  // CHECK: insertelement <2 x double> {{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, double %{{.*}}, double %{{.*}}
  // CHECK-NEXT: insertelement <2 x double> %{{.*}}, double %{{.*}}, i64 0
  return _mm_mask_sub_sd(__W,__U,__A,__B); 
}
__m128d test_mm_maskz_sub_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_sub_sd
  // CHECK-NOT: @llvm.x86.avx512.mask.sub.sd.round
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: fsub double %{{.*}}, %{{.*}}
  // CHECK: insertelement <2 x double> {{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, double %{{.*}}, double %{{.*}}
  // CHECK-NEXT: insertelement <2 x double> %{{.*}}, double %{{.*}}, i64 0
  return _mm_maskz_sub_sd(__U,__A,__B); 
}
__m128 test_mm_mul_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mul_round_ss
  // CHECK: @llvm.x86.avx512.mask.mul.ss.round
  return _mm_mul_round_ss(__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_mask_mul_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_mul_round_ss
  // CHECK: @llvm.x86.avx512.mask.mul.ss.round
  return _mm_mask_mul_round_ss(__W,__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_maskz_mul_round_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_mul_round_ss
  // CHECK: @llvm.x86.avx512.mask.mul.ss.round
  return _mm_maskz_mul_round_ss(__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_mask_mul_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_mul_ss
  // CHECK-NOT: @llvm.x86.avx512.mask.mul.ss.round
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: fmul float %{{.*}}, %{{.*}}
  // CHECK: insertelement <4 x float> {{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, float %{{.*}}, float %{{.*}}
  // CHECK-NEXT: insertelement <4 x float> %{{.*}}, float %{{.*}}, i64 0
  return _mm_mask_mul_ss(__W,__U,__A,__B); 
}
__m128 test_mm_maskz_mul_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_mul_ss
  // CHECK-NOT: @llvm.x86.avx512.mask.mul.ss.round
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: fmul float %{{.*}}, %{{.*}}
  // CHECK: insertelement <4 x float> {{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, float %{{.*}}, float %{{.*}}
  // CHECK-NEXT: insertelement <4 x float> %{{.*}}, float %{{.*}}, i64 0
  return _mm_maskz_mul_ss(__U,__A,__B); 
}
__m128d test_mm_mul_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mul_round_sd
  // CHECK: @llvm.x86.avx512.mask.mul.sd.round
  return _mm_mul_round_sd(__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_mask_mul_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_mul_round_sd
  // CHECK: @llvm.x86.avx512.mask.mul.sd.round
  return _mm_mask_mul_round_sd(__W,__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_maskz_mul_round_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_mul_round_sd
  // CHECK: @llvm.x86.avx512.mask.mul.sd.round
  return _mm_maskz_mul_round_sd(__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_mask_mul_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_mul_sd
  // CHECK-NOT: @llvm.x86.avx512.mask.mul.sd.round
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: fmul double %{{.*}}, %{{.*}}
  // CHECK: insertelement <2 x double> {{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, double %{{.*}}, double %{{.*}}
  // CHECK-NEXT: insertelement <2 x double> %{{.*}}, double %{{.*}}, i64 0
  return _mm_mask_mul_sd(__W,__U,__A,__B); 
}
__m128d test_mm_maskz_mul_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_mul_sd
  // CHECK-NOT: @llvm.x86.avx512.mask.mul.sd.round
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: fmul double %{{.*}}, %{{.*}}
  // CHECK: insertelement <2 x double> {{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, double %{{.*}}, double %{{.*}}
  // CHECK-NEXT: insertelement <2 x double> %{{.*}}, double %{{.*}}, i64 0
  return _mm_maskz_mul_sd(__U,__A,__B); 
}
__m128 test_mm_div_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_div_round_ss
  // CHECK: @llvm.x86.avx512.mask.div.ss.round
  return _mm_div_round_ss(__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_mask_div_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_div_round_ss
  // CHECK: @llvm.x86.avx512.mask.div.ss.round
  return _mm_mask_div_round_ss(__W,__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_maskz_div_round_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_div_round_ss
  // CHECK: @llvm.x86.avx512.mask.div.ss.round
  return _mm_maskz_div_round_ss(__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128 test_mm_mask_div_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_div_ss
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: fdiv float %{{.*}}, %{{.*}}
  // CHECK: insertelement <4 x float> %{{.*}}, float %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, float %{{.*}}, float %{{.*}}
  // CHECK-NEXT: insertelement <4 x float> %{{.*}}, float %{{.*}}, i64 0
  return _mm_mask_div_ss(__W,__U,__A,__B); 
}
__m128 test_mm_maskz_div_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_div_ss
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: fdiv float %{{.*}}, %{{.*}}
  // CHECK: insertelement <4 x float> %{{.*}}, float %{{.*}}, i32 0
  // CHECK: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, float %{{.*}}, float %{{.*}}
  // CHECK-NEXT: insertelement <4 x float> %{{.*}}, float %{{.*}}, i64 0
  return _mm_maskz_div_ss(__U,__A,__B); 
}
__m128d test_mm_div_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_div_round_sd
  // CHECK: @llvm.x86.avx512.mask.div.sd.round
  return _mm_div_round_sd(__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_mask_div_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_div_round_sd
  // CHECK: @llvm.x86.avx512.mask.div.sd.round
  return _mm_mask_div_round_sd(__W,__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_maskz_div_round_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_div_round_sd
  // CHECK: @llvm.x86.avx512.mask.div.sd.round
  return _mm_maskz_div_round_sd(__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
__m128d test_mm_mask_div_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_div_sd
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: fdiv double %{{.*}}, %{{.*}}
  // CHECK: insertelement <2 x double> %{{.*}}, double %{{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, double %{{.*}}, double %{{.*}}
  // CHECK-NEXT: insertelement <2 x double> %{{.*}}, double %{{.*}}, i64 0
  return _mm_mask_div_sd(__W,__U,__A,__B); 
}
__m128d test_mm_maskz_div_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_div_sd
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: fdiv double %{{.*}}, %{{.*}}
  // CHECK: insertelement <2 x double> %{{.*}}, double %{{.*}}, i32 0
  // CHECK: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 %{{.*}}, double %{{.*}}, double %{{.*}}
  // CHECK-NEXT: insertelement <2 x double> %{{.*}}, double %{{.*}}, i64 0
  return _mm_maskz_div_sd(__U,__A,__B); 
}
__m128 test_mm_max_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_max_round_ss
  // CHECK: @llvm.x86.avx512.mask.max.ss.round
  return _mm_max_round_ss(__A,__B,0x08); 
}
__m128 test_mm_mask_max_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_max_round_ss
  // CHECK: @llvm.x86.avx512.mask.max.ss.round
  return _mm_mask_max_round_ss(__W,__U,__A,__B,0x08); 
}
__m128 test_mm_maskz_max_round_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_max_round_ss
  // CHECK: @llvm.x86.avx512.mask.max.ss.round
  return _mm_maskz_max_round_ss(__U,__A,__B,0x08); 
}
__m128 test_mm_mask_max_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_max_ss
  // CHECK: @llvm.x86.avx512.mask.max.ss.round
  return _mm_mask_max_ss(__W,__U,__A,__B); 
}
__m128 test_mm_maskz_max_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_max_ss
  // CHECK: @llvm.x86.avx512.mask.max.ss.round
  return _mm_maskz_max_ss(__U,__A,__B); 
}
__m128d test_mm_max_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_max_round_sd
  // CHECK: @llvm.x86.avx512.mask.max.sd.round
  return _mm_max_round_sd(__A,__B,0x08); 
}
__m128d test_mm_mask_max_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_max_round_sd
  // CHECK: @llvm.x86.avx512.mask.max.sd.round
  return _mm_mask_max_round_sd(__W,__U,__A,__B,0x08); 
}
__m128d test_mm_maskz_max_round_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_max_round_sd
  // CHECK: @llvm.x86.avx512.mask.max.sd.round
  return _mm_maskz_max_round_sd(__U,__A,__B,0x08); 
}
__m128d test_mm_mask_max_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_max_sd
  // CHECK: @llvm.x86.avx512.mask.max.sd.round
  return _mm_mask_max_sd(__W,__U,__A,__B); 
}
__m128d test_mm_maskz_max_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_max_sd
  // CHECK: @llvm.x86.avx512.mask.max.sd.round
  return _mm_maskz_max_sd(__U,__A,__B); 
}
__m128 test_mm_min_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_min_round_ss
  // CHECK: @llvm.x86.avx512.mask.min.ss.round
  return _mm_min_round_ss(__A,__B,0x08); 
}
__m128 test_mm_mask_min_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_min_round_ss
  // CHECK: @llvm.x86.avx512.mask.min.ss.round
  return _mm_mask_min_round_ss(__W,__U,__A,__B,0x08); 
}
__m128 test_mm_maskz_min_round_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_min_round_ss
  // CHECK: @llvm.x86.avx512.mask.min.ss.round
  return _mm_maskz_min_round_ss(__U,__A,__B,0x08); 
}
__m128 test_mm_mask_min_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_min_ss
  // CHECK: @llvm.x86.avx512.mask.min.ss.round
  return _mm_mask_min_ss(__W,__U,__A,__B); 
}
__m128 test_mm_maskz_min_ss(__mmask8 __U, __m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_min_ss
  // CHECK: @llvm.x86.avx512.mask.min.ss.round
  return _mm_maskz_min_ss(__U,__A,__B); 
}
__m128d test_mm_min_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_min_round_sd
  // CHECK: @llvm.x86.avx512.mask.min.sd.round
  return _mm_min_round_sd(__A,__B,0x08); 
}
__m128d test_mm_mask_min_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_min_round_sd
  // CHECK: @llvm.x86.avx512.mask.min.sd.round
  return _mm_mask_min_round_sd(__W,__U,__A,__B,0x08); 
}
__m128d test_mm_maskz_min_round_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_min_round_sd
  // CHECK: @llvm.x86.avx512.mask.min.sd.round
  return _mm_maskz_min_round_sd(__U,__A,__B,0x08); 
}
__m128d test_mm_mask_min_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_min_sd
  // CHECK: @llvm.x86.avx512.mask.min.sd.round
  return _mm_mask_min_sd(__W,__U,__A,__B); 
}
__m128d test_mm_maskz_min_sd(__mmask8 __U, __m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_min_sd
  // CHECK: @llvm.x86.avx512.mask.min.sd.round
  return _mm_maskz_min_sd(__U,__A,__B); 
}
int test_mm_comi_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_comi_round_sd
  // CHECK: @llvm.x86.avx512.vcomi.sd
  return _mm_comi_round_sd(__A, __B, 5, _MM_FROUND_NO_EXC); 
}

int test_mm_comi_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_comi_round_ss
  // CHECK: @llvm.x86.avx512.vcomi.ss
  return _mm_comi_round_ss(__A, __B, 5, _MM_FROUND_NO_EXC); 
}

__m128d test_mm_fixupimm_round_sd(__m128d __A, __m128d __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_fixupimm_round_sd
  // CHECK: @llvm.x86.avx512.mask.fixupimm
  return _mm_fixupimm_round_sd(__A, __B, __C, 5, 8); 
}

__m128d test_mm_mask_fixupimm_round_sd(__m128d __A, __mmask8 __U, __m128d __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_fixupimm_round_sd
  // CHECK: @llvm.x86.avx512.mask.fixupimm
  return _mm_mask_fixupimm_round_sd(__A, __U, __B, __C, 5, 8); 
}

__m128d test_mm_fixupimm_sd(__m128d __A, __m128d __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_fixupimm_sd
  // CHECK: @llvm.x86.avx512.mask.fixupimm
  return _mm_fixupimm_sd(__A, __B, __C, 5); 
}

__m128d test_mm_mask_fixupimm_sd(__m128d __A, __mmask8 __U, __m128d __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_fixupimm_sd
  // CHECK: @llvm.x86.avx512.mask.fixupimm
  return _mm_mask_fixupimm_sd(__A, __U, __B, __C, 5); 
}

__m128d test_mm_maskz_fixupimm_round_sd(__mmask8 __U, __m128d __A, __m128d __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_maskz_fixupimm_round_sd
  // CHECK: @llvm.x86.avx512.maskz.fixupimm
  return _mm_maskz_fixupimm_round_sd(__U, __A, __B, __C, 5, 8); 
}

__m128d test_mm_maskz_fixupimm_sd(__mmask8 __U, __m128d __A, __m128d __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_maskz_fixupimm_sd
  // CHECK: @llvm.x86.avx512.maskz.fixupimm
  return _mm_maskz_fixupimm_sd(__U, __A, __B, __C, 5); 
}

__m128 test_mm_fixupimm_round_ss(__m128 __A, __m128 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_fixupimm_round_ss
  // CHECK: @llvm.x86.avx512.mask.fixupimm
  return _mm_fixupimm_round_ss(__A, __B, __C, 5, 8); 
}

__m128 test_mm_mask_fixupimm_round_ss(__m128 __A, __mmask8 __U, __m128 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_fixupimm_round_ss
  // CHECK: @llvm.x86.avx512.mask.fixupimm
  return _mm_mask_fixupimm_round_ss(__A, __U, __B, __C, 5, 8); 
}

__m128 test_mm_fixupimm_ss(__m128 __A, __m128 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_fixupimm_ss
  // CHECK: @llvm.x86.avx512.mask.fixupimm
  return _mm_fixupimm_ss(__A, __B, __C, 5); 
}

__m128 test_mm_mask_fixupimm_ss(__m128 __A, __mmask8 __U, __m128 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_mask_fixupimm_ss
  // CHECK: @llvm.x86.avx512.mask.fixupimm
  return _mm_mask_fixupimm_ss(__A, __U, __B, __C, 5); 
}

__m128 test_mm_maskz_fixupimm_round_ss(__mmask8 __U, __m128 __A, __m128 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_maskz_fixupimm_round_ss
  // CHECK: @llvm.x86.avx512.maskz.fixupimm
  return _mm_maskz_fixupimm_round_ss(__U, __A, __B, __C, 5, 8); 
}

__m128 test_mm_maskz_fixupimm_ss(__mmask8 __U, __m128 __A, __m128 __B, __m128i __C) {
  // CHECK-LABEL: @test_mm_maskz_fixupimm_ss
  // CHECK: @llvm.x86.avx512.maskz.fixupimm
  return _mm_maskz_fixupimm_ss(__U, __A, __B, __C, 5); 
}

__m128d test_mm_getexp_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_getexp_round_sd
  // CHECK: @llvm.x86.avx512.mask.getexp.sd
  return _mm_getexp_round_sd(__A, __B, 8); 
}

__m128d test_mm_getexp_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_getexp_sd
  // CHECK: @llvm.x86.avx512.mask.getexp.sd
  return _mm_getexp_sd(__A, __B); 
}

__m128 test_mm_getexp_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_getexp_round_ss
  // CHECK: @llvm.x86.avx512.mask.getexp.ss
  return _mm_getexp_round_ss(__A, __B, 8); 
}

__m128 test_mm_getexp_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_getexp_ss
  // CHECK: @llvm.x86.avx512.mask.getexp.ss
  return _mm_getexp_ss(__A, __B); 
}

__m128d test_mm_getmant_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_getmant_round_sd
  // CHECK: @llvm.x86.avx512.mask.getmant.sd
  return _mm_getmant_round_sd(__A, __B, _MM_MANT_NORM_1_2, _MM_MANT_SIGN_src, 8); 
}

__m128d test_mm_getmant_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_getmant_sd
  // CHECK: @llvm.x86.avx512.mask.getmant.sd
  return _mm_getmant_sd(__A, __B, _MM_MANT_NORM_1_2, _MM_MANT_SIGN_src); 
}

__m128 test_mm_getmant_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_getmant_round_ss
  // CHECK: @llvm.x86.avx512.mask.getmant.ss
  return _mm_getmant_round_ss(__A, __B, _MM_MANT_NORM_1_2, _MM_MANT_SIGN_src, 8); 
}

__m128 test_mm_getmant_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_getmant_ss
  // CHECK: @llvm.x86.avx512.mask.getmant.ss
  return _mm_getmant_ss(__A, __B, _MM_MANT_NORM_1_2, _MM_MANT_SIGN_src); 
}

__mmask16 test_mm512_kmov(__mmask16 __A) {
  // CHECK-LABEL: @test_mm512_kmov
  // CHECK: load i16, ptr %__A.addr.i, align 2
  return _mm512_kmov(__A); 
}

#if __x86_64__
long long test_mm_cvt_roundsd_si64(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvt_roundsd_si64
  // CHECK: @llvm.x86.avx512.vcvtsd2si64
  return _mm_cvt_roundsd_si64(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
#endif
int test_mm_cvt_roundsd_si32(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvt_roundsd_si32
  // CHECK: @llvm.x86.avx512.vcvtsd2si32
  return _mm_cvt_roundsd_si32(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

int test_mm_cvt_roundsd_i32(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvt_roundsd_i32
  // CHECK: @llvm.x86.avx512.vcvtsd2si32
  return _mm_cvt_roundsd_i32(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

unsigned test_mm_cvt_roundsd_u32(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvt_roundsd_u32
  // CHECK: @llvm.x86.avx512.vcvtsd2usi32
  return _mm_cvt_roundsd_u32(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

unsigned test_mm_cvtsd_u32(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvtsd_u32
  // CHECK: @llvm.x86.avx512.vcvtsd2usi32
  return _mm_cvtsd_u32(__A); 
}
#ifdef __x86_64__
unsigned long long test_mm_cvt_roundsd_u64(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvt_roundsd_u64
  // CHECK: @llvm.x86.avx512.vcvtsd2usi64
  return _mm_cvt_roundsd_u64(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

unsigned long long test_mm_cvtsd_u64(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvtsd_u64
  // CHECK: @llvm.x86.avx512.vcvtsd2usi64
  return _mm_cvtsd_u64(__A); 
}
#endif

int test_mm_cvt_roundss_si32(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvt_roundss_si32
  // CHECK: @llvm.x86.avx512.vcvtss2si32
  return _mm_cvt_roundss_si32(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

int test_mm_cvt_roundss_i32(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvt_roundss_i32
  // CHECK: @llvm.x86.avx512.vcvtss2si32
  return _mm_cvt_roundss_i32(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

#ifdef __x86_64__
long long test_mm_cvt_roundss_si64(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvt_roundss_si64
  // CHECK: @llvm.x86.avx512.vcvtss2si64
  return _mm_cvt_roundss_si64(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

long long test_mm_cvt_roundss_i64(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvt_roundss_i64
  // CHECK: @llvm.x86.avx512.vcvtss2si64
  return _mm_cvt_roundss_i64(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
#endif

unsigned test_mm_cvt_roundss_u32(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvt_roundss_u32
  // CHECK: @llvm.x86.avx512.vcvtss2usi32
  return _mm_cvt_roundss_u32(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

unsigned test_mm_cvtss_u32(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvtss_u32
  // CHECK: @llvm.x86.avx512.vcvtss2usi32
  return _mm_cvtss_u32(__A); 
}

#ifdef __x86_64__
unsigned long long test_mm_cvt_roundss_u64(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvt_roundss_u64
  // CHECK: @llvm.x86.avx512.vcvtss2usi64
  return _mm_cvt_roundss_u64(__A, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

unsigned long long test_mm_cvtss_u64(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvtss_u64
  // CHECK: @llvm.x86.avx512.vcvtss2usi64
  return _mm_cvtss_u64(__A); 
}
#endif

int test_mm_cvtt_roundsd_i32(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundsd_i32
  // CHECK: @llvm.x86.avx512.cvttsd2si
  return _mm_cvtt_roundsd_i32(__A, _MM_FROUND_NO_EXC);
}

int test_mm_cvtt_roundsd_si32(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundsd_si32
  // CHECK: @llvm.x86.avx512.cvttsd2si
  return _mm_cvtt_roundsd_si32(__A, _MM_FROUND_NO_EXC);
}

int test_mm_cvttsd_i32(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvttsd_i32
  // CHECK: @llvm.x86.avx512.cvttsd2si
  return _mm_cvttsd_i32(__A); 
}

#ifdef __x86_64__
long long test_mm_cvtt_roundsd_si64(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundsd_si64
  // CHECK: @llvm.x86.avx512.cvttsd2si64
  return _mm_cvtt_roundsd_si64(__A, _MM_FROUND_NO_EXC);
}

long long test_mm_cvtt_roundsd_i64(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundsd_i64
  // CHECK: @llvm.x86.avx512.cvttsd2si64
  return _mm_cvtt_roundsd_i64(__A, _MM_FROUND_NO_EXC);
}

long long test_mm_cvttsd_i64(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvttsd_i64
  // CHECK: @llvm.x86.avx512.cvttsd2si64
  return _mm_cvttsd_i64(__A); 
}
#endif

unsigned test_mm_cvtt_roundsd_u32(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundsd_u32
  // CHECK: @llvm.x86.avx512.cvttsd2usi
  return _mm_cvtt_roundsd_u32(__A, _MM_FROUND_NO_EXC);
}

unsigned test_mm_cvttsd_u32(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvttsd_u32
  // CHECK: @llvm.x86.avx512.cvttsd2usi
  return _mm_cvttsd_u32(__A); 
}

#ifdef __x86_64__
unsigned long long test_mm_cvtt_roundsd_u64(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundsd_u64
  // CHECK: @llvm.x86.avx512.cvttsd2usi64
  return _mm_cvtt_roundsd_u64(__A, _MM_FROUND_NO_EXC);
}

unsigned long long test_mm_cvttsd_u64(__m128d __A) {
  // CHECK-LABEL: @test_mm_cvttsd_u64
  // CHECK: @llvm.x86.avx512.cvttsd2usi64
  return _mm_cvttsd_u64(__A); 
}
#endif

int test_mm_cvtt_roundss_i32(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundss_i32
  // CHECK: @llvm.x86.avx512.cvttss2si
  return _mm_cvtt_roundss_i32(__A, _MM_FROUND_NO_EXC);
}

int test_mm_cvtt_roundss_si32(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundss_si32
  // CHECK: @llvm.x86.avx512.cvttss2si
  return _mm_cvtt_roundss_si32(__A, _MM_FROUND_NO_EXC);
}

int test_mm_cvttss_i32(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvttss_i32
  // CHECK: @llvm.x86.avx512.cvttss2si
  return _mm_cvttss_i32(__A); 
}

#ifdef __x86_64__
float test_mm_cvtt_roundss_i64(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundss_i64
  // CHECK: @llvm.x86.avx512.cvttss2si64
  return _mm_cvtt_roundss_i64(__A, _MM_FROUND_NO_EXC);
}

long long test_mm_cvtt_roundss_si64(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundss_si64
  // CHECK: @llvm.x86.avx512.cvttss2si64
  return _mm_cvtt_roundss_si64(__A, _MM_FROUND_NO_EXC);
}

long long test_mm_cvttss_i64(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvttss_i64
  // CHECK: @llvm.x86.avx512.cvttss2si64
  return _mm_cvttss_i64(__A); 
}
#endif

unsigned test_mm_cvtt_roundss_u32(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundss_u32
  // CHECK: @llvm.x86.avx512.cvttss2usi
  return _mm_cvtt_roundss_u32(__A, _MM_FROUND_NO_EXC);
}

unsigned test_mm_cvttss_u32(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvttss_u32
  // CHECK: @llvm.x86.avx512.cvttss2usi
  return _mm_cvttss_u32(__A); 
}

#ifdef __x86_64__
unsigned long long test_mm_cvtt_roundss_u64(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvtt_roundss_u64
  // CHECK: @llvm.x86.avx512.cvttss2usi64
  return _mm_cvtt_roundss_u64(__A, _MM_FROUND_NO_EXC);
}

unsigned long long test_mm_cvttss_u64(__m128 __A) {
  // CHECK-LABEL: @test_mm_cvttss_u64
  // CHECK: @llvm.x86.avx512.cvttss2usi64
  return _mm_cvttss_u64(__A); 
}
#endif

__m128d test_mm_roundscale_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_roundscale_round_sd
  // CHECK: @llvm.x86.avx512.mask.rndscale.sd
  return _mm_roundscale_round_sd(__A, __B, 3, _MM_FROUND_NO_EXC); 
}

__m128d test_mm_roundscale_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_roundscale_sd
  // CHECK: @llvm.x86.avx512.mask.rndscale.sd
  return _mm_roundscale_sd(__A, __B, 3); 
}

__m128d test_mm_mask_roundscale_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK: @llvm.x86.avx512.mask.rndscale.sd
    return _mm_mask_roundscale_sd(__W,__U,__A,__B,3);
}

__m128d test_mm_mask_roundscale_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK: @llvm.x86.avx512.mask.rndscale.sd
    return _mm_mask_roundscale_round_sd(__W,__U,__A,__B,3,_MM_FROUND_NO_EXC);
}

__m128d test_mm_maskz_roundscale_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK: @llvm.x86.avx512.mask.rndscale.sd
    return _mm_maskz_roundscale_sd(__U,__A,__B,3);
}

__m128d test_mm_maskz_roundscale_round_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK: @llvm.x86.avx512.mask.rndscale.sd
    return _mm_maskz_roundscale_round_sd(__U,__A,__B,3,_MM_FROUND_NO_EXC );
}

__m128 test_mm_roundscale_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_roundscale_round_ss
  // CHECK: @llvm.x86.avx512.mask.rndscale.ss
  return _mm_roundscale_round_ss(__A, __B, 3, _MM_FROUND_NO_EXC);
}

__m128 test_mm_roundscale_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_roundscale_ss
  // CHECK: @llvm.x86.avx512.mask.rndscale.ss
  return _mm_roundscale_ss(__A, __B, 3); 
}

__m128 test_mm_mask_roundscale_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_roundscale_ss
  // CHECK: @llvm.x86.avx512.mask.rndscale.ss
    return _mm_mask_roundscale_ss(__W,__U,__A,__B,3);
}

__m128 test_mm_maskz_roundscale_round_ss( __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_roundscale_round_ss
  // CHECK: @llvm.x86.avx512.mask.rndscale.ss
    return _mm_maskz_roundscale_round_ss(__U,__A,__B,3,_MM_FROUND_NO_EXC);
}

__m128 test_mm_maskz_roundscale_ss(__mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_roundscale_ss
  // CHECK: @llvm.x86.avx512.mask.rndscale.ss
    return _mm_maskz_roundscale_ss(__U,__A,__B,3);
}

__m128d test_mm_scalef_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_scalef_round_sd
  // CHECK: @llvm.x86.avx512.mask.scalef.sd(<2 x double> %{{.*}}, <2 x double> %{{.*}}, <2 x double> %2, i8 -1, i32 11)
  return _mm_scalef_round_sd(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_scalef_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_scalef_sd
  // CHECK: @llvm.x86.avx512.mask.scalef
  return _mm_scalef_sd(__A, __B); 
}

__m128d test_mm_mask_scalef_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_scalef_sd
  // CHECK: @llvm.x86.avx512.mask.scalef.sd
  return _mm_mask_scalef_sd(__W, __U, __A, __B);
}

__m128d test_mm_mask_scalef_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_scalef_round_sd
  // CHECK: @llvm.x86.avx512.mask.scalef.sd(<2 x double> %{{.*}}, <2 x double> %{{.*}}, <2 x double> %{{.*}}, i8 %{{.*}}, i32 11)
    return _mm_mask_scalef_round_sd(__W, __U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_maskz_scalef_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_maskz_scalef_sd
  // CHECK: @llvm.x86.avx512.mask.scalef.sd
    return _mm_maskz_scalef_sd(__U, __A, __B);
}

__m128d test_mm_maskz_scalef_round_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_maskz_scalef_round_sd
  // CHECK: @llvm.x86.avx512.mask.scalef.sd(<2 x double> %{{.*}}, <2 x double> %{{.*}}, <2 x double> %{{.*}}, i8 %{{.*}}, i32 11)
    return _mm_maskz_scalef_round_sd(__U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_scalef_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_scalef_round_ss
  // CHECK: @llvm.x86.avx512.mask.scalef.ss(<4 x float> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}, i8 -1, i32 11)
  return _mm_scalef_round_ss(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_scalef_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_scalef_ss
  // CHECK: @llvm.x86.avx512.mask.scalef.ss
  return _mm_scalef_ss(__A, __B); 
}

__m128 test_mm_mask_scalef_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_scalef_ss
  // CHECK: @llvm.x86.avx512.mask.scalef.ss
    return _mm_mask_scalef_ss(__W, __U, __A, __B);
}

__m128 test_mm_mask_scalef_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_scalef_round_ss
  // CHECK: @llvm.x86.avx512.mask.scalef.ss(<4 x float> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}, i8 %{{.*}}, i32 11)
    return _mm_mask_scalef_round_ss(__W, __U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_maskz_scalef_ss(__mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_scalef_ss
  // CHECK: @llvm.x86.avx512.mask.scalef.ss
    return _mm_maskz_scalef_ss(__U, __A, __B);
}

__m128 test_mm_maskz_scalef_round_ss(__mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_scalef_round_ss
  // CHECK: @llvm.x86.avx512.mask.scalef.ss(<4 x float> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}, i8 %{{.*}}, i32 11)
    return _mm_maskz_scalef_round_ss(__U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_sqrt_round_sd(__m128d __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_sqrt_round_sd
  // CHECK: call <2 x double> @llvm.x86.avx512.mask.sqrt.sd(<2 x double> %{{.*}}, <2 x double> %{{.*}}, <2 x double> %{{.*}}, i8 -1, i32 11)
  return _mm_sqrt_round_sd(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_sqrt_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_sqrt_sd
  // CHECK: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: call double @llvm.sqrt.f64(double %{{.*}})
  // CHECK-NEXT: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 {{.*}}, double {{.*}}, double {{.*}}
  // CHECK-NEXT: insertelement <2 x double> %{{.*}}, double {{.*}}, i64 0
  return _mm_mask_sqrt_sd(__W,__U,__A,__B);
}

__m128d test_mm_mask_sqrt_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_sqrt_round_sd
  // CHECK: call <2 x double> @llvm.x86.avx512.mask.sqrt.sd(<2 x double> %{{.*}}, <2 x double> %{{.*}}, <2 x double> %{{.*}}, i8 %{{.*}}, i32 11)
  return _mm_mask_sqrt_round_sd(__W,__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_maskz_sqrt_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_maskz_sqrt_sd
  // CHECK: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: call double @llvm.sqrt.f64(double %{{.*}})
  // CHECK-NEXT: extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 {{.*}}, double {{.*}}, double {{.*}}
  // CHECK-NEXT: insertelement <2 x double> %{{.*}}, double {{.*}}, i64 0
  return _mm_maskz_sqrt_sd(__U,__A,__B);
}

__m128d test_mm_maskz_sqrt_round_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_maskz_sqrt_round_sd
  // CHECK: call <2 x double> @llvm.x86.avx512.mask.sqrt.sd(<2 x double> %{{.*}}, <2 x double> %{{.*}}, <2 x double> %{{.*}}, i8 %{{.*}}, i32 11)
  return _mm_maskz_sqrt_round_sd(__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_sqrt_round_ss(__m128 __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_sqrt_round_ss
  // CHECK: call <4 x float> @llvm.x86.avx512.mask.sqrt.ss(<4 x float> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}, i8 -1, i32 11)
  return _mm_sqrt_round_ss(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_sqrt_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_sqrt_ss
  // CHECK: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: call float @llvm.sqrt.f32(float %{{.*}})
  // CHECK-NEXT: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 {{.*}}, float {{.*}}, float {{.*}}
  // CHECK-NEXT: insertelement <4 x float> %{{.*}}, float {{.*}}, i64 0
  return _mm_mask_sqrt_ss(__W,__U,__A,__B);
}

__m128 test_mm_mask_sqrt_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_sqrt_round_ss
  // CHECK: call <4 x float> @llvm.x86.avx512.mask.sqrt.ss(<4 x float> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}, i8 {{.*}}, i32 11)
  return _mm_mask_sqrt_round_ss(__W,__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_maskz_sqrt_ss(__mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_sqrt_ss
  // CHECK: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: call float @llvm.sqrt.f32(float %{{.*}})
  // CHECK-NEXT: extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: select i1 {{.*}}, float {{.*}}, float {{.*}}
  // CHECK-NEXT: insertelement <4 x float> %{{.*}}, float {{.*}}, i64 0
  return _mm_maskz_sqrt_ss(__U,__A,__B);
}

__m128 test_mm_maskz_sqrt_round_ss(__mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_sqrt_round_ss
  // CHECK: call <4 x float> @llvm.x86.avx512.mask.sqrt.ss(<4 x float> %{{.*}}, <4 x float> %{{.*}}, <4 x float> %{{.*}}, i8 {{.*}}, i32 11)
  return _mm_maskz_sqrt_round_ss(__U,__A,__B,_MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_rsqrt14_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_rsqrt14_sd
  // CHECK: @llvm.x86.avx512.rsqrt14.sd
  return _mm_mask_rsqrt14_sd(__W, __U, __A, __B);
}

__m128d test_mm_maskz_rsqrt14_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_maskz_rsqrt14_sd
  // CHECK: @llvm.x86.avx512.rsqrt14.sd
  return _mm_maskz_rsqrt14_sd(__U, __A, __B);
}

__m128 test_mm_mask_rsqrt14_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_rsqrt14_ss
  // CHECK: @llvm.x86.avx512.rsqrt14.ss
  return _mm_mask_rsqrt14_ss(__W, __U, __A, __B);
}

__m128 test_mm_maskz_rsqrt14_ss(__mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_rsqrt14_ss
  // CHECK: @llvm.x86.avx512.rsqrt14.ss
  return _mm_maskz_rsqrt14_ss(__U, __A, __B);
}

__m128d test_mm_mask_rcp14_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_rcp14_sd
  // CHECK: @llvm.x86.avx512.rcp14.sd
  return _mm_mask_rcp14_sd(__W, __U, __A, __B);
}

__m128d test_mm_maskz_rcp14_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_maskz_rcp14_sd
  // CHECK: @llvm.x86.avx512.rcp14.sd
  return _mm_maskz_rcp14_sd(__U, __A, __B);
}

__m128 test_mm_mask_rcp14_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_rcp14_ss
  // CHECK: @llvm.x86.avx512.rcp14.ss
  return _mm_mask_rcp14_ss(__W, __U, __A, __B);
}

__m128 test_mm_maskz_rcp14_ss(__mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_rcp14_ss
  // CHECK: @llvm.x86.avx512.rcp14.ss
  return _mm_maskz_rcp14_ss(__U, __A, __B);
}

__m128d test_mm_mask_getexp_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_getexp_sd
  // CHECK: @llvm.x86.avx512.mask.getexp.sd
  return _mm_mask_getexp_sd(__W, __U, __A, __B);
}

__m128d test_mm_mask_getexp_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_getexp_round_sd
  // CHECK: @llvm.x86.avx512.mask.getexp.sd
  return _mm_mask_getexp_round_sd(__W, __U, __A, __B, _MM_FROUND_NO_EXC);
}

__m128d test_mm_maskz_getexp_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_maskz_getexp_sd
  // CHECK: @llvm.x86.avx512.mask.getexp.sd
  return _mm_maskz_getexp_sd(__U, __A, __B);
}

__m128d test_mm_maskz_getexp_round_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_maskz_getexp_round_sd
  // CHECK: @llvm.x86.avx512.mask.getexp.sd
  return _mm_maskz_getexp_round_sd(__U, __A, __B, _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_getexp_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_getexp_ss
  // CHECK: @llvm.x86.avx512.mask.getexp.ss
  return _mm_mask_getexp_ss(__W, __U, __A, __B);
}

__m128 test_mm_mask_getexp_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_getexp_round_ss
  // CHECK: @llvm.x86.avx512.mask.getexp.ss
  return _mm_mask_getexp_round_ss(__W, __U, __A, __B, _MM_FROUND_NO_EXC);
}

__m128 test_mm_maskz_getexp_ss(__mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_getexp_ss
  // CHECK: @llvm.x86.avx512.mask.getexp.ss
  return _mm_maskz_getexp_ss(__U, __A, __B);
}

__m128 test_mm_maskz_getexp_round_ss(__mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_getexp_round_ss
  // CHECK: @llvm.x86.avx512.mask.getexp.ss
  return _mm_maskz_getexp_round_ss(__U, __A, __B, _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_getmant_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_getmant_sd
  // CHECK: @llvm.x86.avx512.mask.getmant.sd
  return _mm_mask_getmant_sd(__W, __U, __A, __B, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m128d test_mm_mask_getmant_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_getmant_round_sd
  // CHECK: @llvm.x86.avx512.mask.getmant.sd
  return _mm_mask_getmant_round_sd(__W, __U, __A, __B, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan, _MM_FROUND_NO_EXC);
}

__m128d test_mm_maskz_getmant_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_maskz_getmant_sd
  // CHECK: @llvm.x86.avx512.mask.getmant.sd
  return _mm_maskz_getmant_sd(__U, __A, __B, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m128d test_mm_maskz_getmant_round_sd(__mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_maskz_getmant_round_sd
  // CHECK: @llvm.x86.avx512.mask.getmant.sd
  return _mm_maskz_getmant_round_sd(__U, __A, __B, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan, _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_getmant_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_getmant_ss
  // CHECK: @llvm.x86.avx512.mask.getmant.ss
  return _mm_mask_getmant_ss(__W, __U, __A, __B, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m128 test_mm_mask_getmant_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_getmant_round_ss
  // CHECK: @llvm.x86.avx512.mask.getmant.ss
  return _mm_mask_getmant_round_ss(__W, __U, __A, __B, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan, _MM_FROUND_NO_EXC);
}

__m128 test_mm_maskz_getmant_ss(__mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_getmant_ss
  // CHECK: @llvm.x86.avx512.mask.getmant.ss
  return _mm_maskz_getmant_ss(__U, __A, __B, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan);
}

__m128 test_mm_maskz_getmant_round_ss(__mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_maskz_getmant_round_ss
  // CHECK: @llvm.x86.avx512.mask.getmant.ss
  return _mm_maskz_getmant_round_ss(__U, __A, __B, _MM_MANT_NORM_p5_2, _MM_MANT_SIGN_nan, _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_fmadd_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_fmadd_ss
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[A]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_mask_fmadd_ss(__W, __U, __A, __B);
}

__m128 test_mm_fmadd_round_ss(__m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_fmadd_round_ss
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[FMA]], i64 0
  return _mm_fmadd_round_ss(__A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_fmadd_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_fmadd_round_ss
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[A]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_mask_fmadd_round_ss(__W, __U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_maskz_fmadd_ss(__mmask8 __U, __m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_maskz_fmadd_ss
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float 0.000000e+00
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_maskz_fmadd_ss(__U, __A, __B, __C);
}

__m128 test_mm_maskz_fmadd_round_ss(__mmask8 __U, __m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_maskz_fmadd_round_ss
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float 0.000000e+00
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_maskz_fmadd_round_ss(__U, __A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask3_fmadd_ss(__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fmadd_ss
  // CHECK: [[A:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[ORIGC:%.+]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[C]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGC]], float [[SEL]], i64 0
  return _mm_mask3_fmadd_ss(__W, __X, __Y, __U);
}

__m128 test_mm_mask3_fmadd_round_ss(__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fmadd_round_ss
  // CHECK: [[A:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[ORIGC:%.+]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[C]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGC]], float [[SEL]], i64 0
  return _mm_mask3_fmadd_round_ss(__W, __X, __Y, __U, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_fmsub_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_fmsub_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[A]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_mask_fmsub_ss(__W, __U, __A, __B);
}

__m128 test_mm_fmsub_round_ss(__m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_fmsub_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[FMA]], i64 0
  return _mm_fmsub_round_ss(__A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_fmsub_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_fmsub_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[A]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_mask_fmsub_round_ss(__W, __U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_maskz_fmsub_ss(__mmask8 __U, __m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_maskz_fmsub_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float 0.000000e+00
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_maskz_fmsub_ss(__U, __A, __B, __C);
}

__m128 test_mm_maskz_fmsub_round_ss(__mmask8 __U, __m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_maskz_fmsub_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float 0.000000e+00
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_maskz_fmsub_round_ss(__U, __A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask3_fmsub_ss(__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fmsub_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> [[ORIGC:%.+]]
  // CHECK: [[A:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: [[C2:%.+]] = extractelement <4 x float> [[ORIGC]], i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[C2]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGC]], float [[SEL]], i64 0
  return _mm_mask3_fmsub_ss(__W, __X, __Y, __U);
}

__m128 test_mm_mask3_fmsub_round_ss(__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fmsub_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> [[ORIGC:%.+]]
  // CHECK: [[A:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: [[C2:%.+]] = extractelement <4 x float> [[ORIGC]], i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[C2]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGC]], float [[SEL]], i64 0
  return _mm_mask3_fmsub_round_ss(__W, __X, __Y, __U, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_fnmadd_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_fnmadd_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[A]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_mask_fnmadd_ss(__W, __U, __A, __B);
}

__m128 test_mm_fnmadd_round_ss(__m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_fnmadd_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[FMA]], i64 0
  return _mm_fnmadd_round_ss(__A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_fnmadd_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_fnmadd_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[A]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_mask_fnmadd_round_ss(__W, __U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_maskz_fnmadd_ss(__mmask8 __U, __m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_maskz_fnmadd_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float 0.000000e+00
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_maskz_fnmadd_ss(__U, __A, __B, __C);
}

__m128 test_mm_maskz_fnmadd_round_ss(__mmask8 __U, __m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_maskz_fnmadd_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float 0.000000e+00
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_maskz_fnmadd_round_ss(__U, __A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask3_fnmadd_ss(__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fnmadd_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[ORIGC:%.+]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[C]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGC]], float [[SEL]], i64 0
  return _mm_mask3_fnmadd_ss(__W, __X, __Y, __U);
}

__m128 test_mm_mask3_fnmadd_round_ss(__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fnmadd_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[ORIGC:%.+]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[C]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGC]], float [[SEL]], i64 0
  return _mm_mask3_fnmadd_round_ss(__W, __X, __Y, __U, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_fnmsub_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_fnmsub_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[A]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_mask_fnmsub_ss(__W, __U, __A, __B);
}

__m128 test_mm_fnmsub_round_ss(__m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_fnmsub_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[FMA]], i64 0
  return _mm_fnmsub_round_ss(__A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_fnmsub_round_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128 __B){
  // CHECK-LABEL: @test_mm_mask_fnmsub_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[A]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_mask_fnmsub_round_ss(__W, __U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_maskz_fnmsub_ss(__mmask8 __U, __m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_maskz_fnmsub_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float 0.000000e+00
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_maskz_fnmsub_ss(__U, __A, __B, __C);
}

__m128 test_mm_maskz_fnmsub_round_ss(__mmask8 __U, __m128 __A, __m128 __B, __m128 __C){
  // CHECK-LABEL: @test_mm_maskz_fnmsub_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <4 x float> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float 0.000000e+00
  // CHECK-NEXT: insertelement <4 x float> [[ORIGA]], float [[SEL]], i64 0
  return _mm_maskz_fnmsub_round_ss(__U, __A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask3_fnmsub_ss(__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fnmsub_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <4 x float> [[ORIGC:%.+]]
  // CHECK: [[A:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.fma.f32(float [[A]], float [[B]], float [[C]])
  // CHECK-NEXT: [[C2:%.+]] = extractelement <4 x float> [[ORIGC]], i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[C2]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGC]], float [[SEL]], i64 0
  return _mm_mask3_fnmsub_ss(__W, __X, __Y, __U);
}

__m128 test_mm_mask3_fnmsub_round_ss(__m128 __W, __m128 __X, __m128 __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fnmsub_round_ss
  // CHECK: [[NEG:%.+]] = fneg <4 x float> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <4 x float> [[ORIGC:%.+]]
  // CHECK: [[A:%.+]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <4 x float> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <4 x float> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call float @llvm.x86.avx512.vfmadd.f32(float [[A]], float [[B]], float [[C]], i32 11)
  // CHECK-NEXT: [[C2:%.+]] = extractelement <4 x float> [[ORIGC]], i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, float [[FMA]], float [[C2]]
  // CHECK-NEXT: insertelement <4 x float> [[ORIGC]], float [[SEL]], i64 0
  return _mm_mask3_fnmsub_round_ss(__W, __X, __Y, __U, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_fmadd_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_fmadd_sd
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[A]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_mask_fmadd_sd(__W, __U, __A, __B);
}

__m128d test_mm_fmadd_round_sd(__m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_fmadd_round_sd
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[FMA]], i64 0
  return _mm_fmadd_round_sd(__A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_fmadd_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_fmadd_round_sd
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[A]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_mask_fmadd_round_sd(__W, __U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_maskz_fmadd_sd(__mmask8 __U, __m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_maskz_fmadd_sd
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double 0.000000e+00
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_maskz_fmadd_sd(__U, __A, __B, __C);
}

__m128d test_mm_maskz_fmadd_round_sd(__mmask8 __U, __m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_maskz_fmadd_round_sd
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double 0.000000e+00
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_maskz_fmadd_round_sd(__U, __A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask3_fmadd_sd(__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fmadd_sd
  // CHECK: [[A:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[ORIGC:%.+]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[C]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGC]], double [[SEL]], i64 0
  return _mm_mask3_fmadd_sd(__W, __X, __Y, __U);
}

__m128d test_mm_mask3_fmadd_round_sd(__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fmadd_round_sd
  // CHECK: [[A:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[ORIGC:%.+]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[C]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGC]], double [[SEL]], i64 0
  return _mm_mask3_fmadd_round_sd(__W, __X, __Y, __U, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_fmsub_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_fmsub_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[A]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_mask_fmsub_sd(__W, __U, __A, __B);
}

__m128d test_mm_fmsub_round_sd(__m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_fmsub_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[FMA]], i64 0
  return _mm_fmsub_round_sd(__A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_fmsub_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_fmsub_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[A]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_mask_fmsub_round_sd(__W, __U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_maskz_fmsub_sd(__mmask8 __U, __m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_maskz_fmsub_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double 0.000000e+00
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_maskz_fmsub_sd(__U, __A, __B, __C);
}

__m128d test_mm_maskz_fmsub_round_sd(__mmask8 __U, __m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_maskz_fmsub_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double 0.000000e+00
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_maskz_fmsub_round_sd(__U, __A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask3_fmsub_sd(__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fmsub_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> [[ORIGC:%.+]]
  // CHECK: [[A:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: [[C2:%.+]] = extractelement <2 x double> [[ORIGC]], i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[C2]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGC]], double [[SEL]], i64 0
  return _mm_mask3_fmsub_sd(__W, __X, __Y, __U);
}

__m128d test_mm_mask3_fmsub_round_sd(__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fmsub_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> [[ORIGC:%.+]]
  // CHECK: [[A:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: [[C2:%.+]] = extractelement <2 x double> [[ORIGC]], i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[C2]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGC]], double [[SEL]], i64 0
  return _mm_mask3_fmsub_round_sd(__W, __X, __Y, __U, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_fnmadd_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_fnmadd_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[A]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_mask_fnmadd_sd(__W, __U, __A, __B);
}

__m128d test_mm_fnmadd_round_sd(__m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_fnmadd_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[FMA]], i64 0
  return _mm_fnmadd_round_sd(__A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_fnmadd_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_fnmadd_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[A]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_mask_fnmadd_round_sd(__W, __U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_maskz_fnmadd_sd(__mmask8 __U, __m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_maskz_fnmadd_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double 0.000000e+00
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_maskz_fnmadd_sd(__U, __A, __B, __C);
}

__m128d test_mm_maskz_fnmadd_round_sd(__mmask8 __U, __m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_maskz_fnmadd_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.+]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double 0.000000e+00
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_maskz_fnmadd_round_sd(__U, __A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask3_fnmadd_sd(__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fnmadd_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[ORIGC:%.+]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[C]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGC]], double [[SEL]], i64 0
  return _mm_mask3_fnmadd_sd(__W, __X, __Y, __U);
}

__m128d test_mm_mask3_fnmadd_round_sd(__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fnmadd_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[ORIGC:%.+]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[C]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGC]], double [[SEL]], i64 0
  return _mm_mask3_fnmadd_round_sd(__W, __X, __Y, __U, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_fnmsub_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_fnmsub_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[A]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_mask_fnmsub_sd(__W, __U, __A, __B);
}

__m128d test_mm_fnmsub_round_sd(__m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_fnmsub_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[FMA]], i64 0
  return _mm_fnmsub_round_sd(__A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_fnmsub_round_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128d __B){
  // CHECK-LABEL: @test_mm_mask_fnmsub_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[A]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_mask_fnmsub_round_sd(__W, __U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_maskz_fnmsub_sd(__mmask8 __U, __m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_maskz_fnmsub_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double 0.000000e+00
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_maskz_fnmsub_sd(__U, __A, __B, __C);
}

__m128d test_mm_maskz_fnmsub_round_sd(__mmask8 __U, __m128d __A, __m128d __B, __m128d __C){
  // CHECK-LABEL: @test_mm_maskz_fnmsub_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[A:%.+]] = extractelement <2 x double> [[ORIGA:%.]], i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double 0.000000e+00
  // CHECK-NEXT: insertelement <2 x double> [[ORIGA]], double [[SEL]], i64 0
  return _mm_maskz_fnmsub_round_sd(__U, __A, __B, __C, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask3_fnmsub_sd(__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fnmsub_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <2 x double> [[ORIGC:%.+]]
  // CHECK: [[A:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.fma.f64(double [[A]], double [[B]], double [[C]])
  // CHECK-NEXT: [[C2:%.+]] = extractelement <2 x double> [[ORIGC]], i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[C2]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGC]], double [[SEL]], i64 0
  return _mm_mask3_fnmsub_sd(__W, __X, __Y, __U);
}

__m128d test_mm_mask3_fnmsub_round_sd(__m128d __W, __m128d __X, __m128d __Y, __mmask8 __U){
  // CHECK-LABEL: @test_mm_mask3_fnmsub_round_sd
  // CHECK: [[NEG:%.+]] = fneg <2 x double> %{{.*}}
  // CHECK: [[NEG2:%.+]] = fneg <2 x double> [[ORIGC:%.+]]
  // CHECK: [[A:%.+]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: [[B:%.+]] = extractelement <2 x double> [[NEG]], i64 0
  // CHECK-NEXT: [[C:%.+]] = extractelement <2 x double> [[NEG2]], i64 0
  // CHECK-NEXT: [[FMA:%.+]] = call double @llvm.x86.avx512.vfmadd.f64(double [[A]], double [[B]], double [[C]], i32 11)
  // CHECK-NEXT: [[C2:%.+]] = extractelement <2 x double> [[ORIGC]], i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.+]] = select i1 %{{.*}}, double [[FMA]], double [[C2]]
  // CHECK-NEXT: insertelement <2 x double> [[ORIGC]], double [[SEL]], i64 0
  return _mm_mask3_fnmsub_round_sd(__W, __X, __Y, __U, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__mmask8 test_mm_cmp_round_ss_mask(__m128 __X, __m128 __Y) {
  // CHECK-LABEL: @test_mm_cmp_round_ss_mask
  // CHECK: @llvm.x86.avx512.mask.cmp
  return _mm_cmp_round_ss_mask(__X, __Y, _CMP_NLT_US, _MM_FROUND_NO_EXC);
}

__mmask8 test_mm_mask_cmp_round_ss_mask(__mmask8 __M, __m128 __X, __m128 __Y) {
  // CHECK-LABEL: @test_mm_mask_cmp_round_ss_mask
  // CHECK: @llvm.x86.avx512.mask.cmp
  return _mm_mask_cmp_round_ss_mask(__M, __X, __Y, _CMP_NLT_US, _MM_FROUND_NO_EXC);
}

__mmask8 test_mm_cmp_ss_mask(__m128 __X, __m128 __Y) {
  // CHECK-LABEL: @test_mm_cmp_ss_mask
  // CHECK: @llvm.x86.avx512.mask.cmp
  return _mm_cmp_ss_mask(__X, __Y, _CMP_NLT_US);
}

__mmask8 test_mm_mask_cmp_ss_mask(__mmask8 __M, __m128 __X, __m128 __Y) {
  // CHECK-LABEL: @test_mm_mask_cmp_ss_mask
  // CHECK: @llvm.x86.avx512.mask.cmp
  return _mm_mask_cmp_ss_mask(__M, __X, __Y, _CMP_NLT_US);
}

__mmask8 test_mm_cmp_round_sd_mask(__m128d __X, __m128d __Y) {
  // CHECK-LABEL: @test_mm_cmp_round_sd_mask
  // CHECK: @llvm.x86.avx512.mask.cmp
  return _mm_cmp_round_sd_mask(__X, __Y, _CMP_NLT_US, _MM_FROUND_NO_EXC);
}

__mmask8 test_mm_mask_cmp_round_sd_mask(__mmask8 __M, __m128d __X, __m128d __Y) {
  // CHECK-LABEL: @test_mm_mask_cmp_round_sd_mask
  // CHECK: @llvm.x86.avx512.mask.cmp
  return _mm_mask_cmp_round_sd_mask(__M, __X, __Y, _CMP_NLT_US, _MM_FROUND_NO_EXC);
}

__mmask8 test_mm_cmp_sd_mask(__m128d __X, __m128d __Y) {
  // CHECK-LABEL: @test_mm_cmp_sd_mask
  // CHECK: @llvm.x86.avx512.mask.cmp
  return _mm_cmp_sd_mask(__X, __Y, _CMP_NLT_US);
}

__mmask8 test_mm_mask_cmp_sd_mask(__mmask8 __M, __m128d __X, __m128d __Y) {
  // CHECK-LABEL: @test_mm_mask_cmp_sd_mask
  // CHECK: @llvm.x86.avx512.mask.cmp
  return _mm_mask_cmp_sd_mask(__M, __X, __Y, _CMP_NLT_US);
}

__m128 test_mm_cvt_roundsd_ss(__m128 __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_cvt_roundsd_ss
  // CHECK: @llvm.x86.avx512.mask.cvtsd2ss.round
  return _mm_cvt_roundsd_ss(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_mask_cvt_roundsd_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_cvt_roundsd_ss
  // CHECK: @llvm.x86.avx512.mask.cvtsd2ss.round
  return _mm_mask_cvt_roundsd_ss(__W, __U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_maskz_cvt_roundsd_ss(__mmask8 __U, __m128 __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_cvt_roundsd_ss
  // CHECK: @llvm.x86.avx512.mask.cvtsd2ss.round
  return _mm_maskz_cvt_roundsd_ss(__U, __A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

#ifdef __x86_64__
__m128d test_mm_cvt_roundi64_sd(__m128d __A, long long __B) {
  // CHECK-LABEL: @test_mm_cvt_roundi64_sd
  // CHECK: @llvm.x86.avx512.cvtsi2sd64
  return _mm_cvt_roundi64_sd(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_cvt_roundsi64_sd(__m128d __A, long long __B) {
  // CHECK-LABEL: @test_mm_cvt_roundsi64_sd
  // CHECK: @llvm.x86.avx512.cvtsi2sd64
  return _mm_cvt_roundsi64_sd(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
#endif

__m128 test_mm_cvt_roundsi32_ss(__m128 __A, int __B) {
  // CHECK-LABEL: @test_mm_cvt_roundsi32_ss
  // CHECK: @llvm.x86.avx512.cvtsi2ss32
  return _mm_cvt_roundsi32_ss(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_cvt_roundi32_ss(__m128 __A, int __B) {
  // CHECK-LABEL: @test_mm_cvt_roundi32_ss
  // CHECK: @llvm.x86.avx512.cvtsi2ss32
  return _mm_cvt_roundi32_ss(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

#ifdef __x86_64__
__m128 test_mm_cvt_roundsi64_ss(__m128 __A, long long __B) {
  // CHECK-LABEL: @test_mm_cvt_roundsi64_ss
  // CHECK: @llvm.x86.avx512.cvtsi2ss64
  return _mm_cvt_roundsi64_ss(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_cvt_roundi64_ss(__m128 __A, long long __B) {
  // CHECK-LABEL: @test_mm_cvt_roundi64_ss
  // CHECK: @llvm.x86.avx512.cvtsi2ss64
  return _mm_cvt_roundi64_ss(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}
#endif

__m128d test_mm_cvt_roundss_sd(__m128d __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_cvt_roundss_sd
  // CHECK: @llvm.x86.avx512.mask.cvtss2sd.round
  return _mm_cvt_roundss_sd(__A, __B, _MM_FROUND_NO_EXC);
}

__m128d test_mm_mask_cvt_roundss_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_cvt_roundss_sd
  // CHECK: @llvm.x86.avx512.mask.cvtss2sd.round
  return _mm_mask_cvt_roundss_sd(__W, __U, __A, __B, _MM_FROUND_NO_EXC);
}

__m128d test_mm_maskz_cvt_roundss_sd( __mmask8 __U, __m128d __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_cvt_roundss_sd
  // CHECK: @llvm.x86.avx512.mask.cvtss2sd.round
  return _mm_maskz_cvt_roundss_sd( __U, __A, __B, _MM_FROUND_NO_EXC);
}

__m128d test_mm_cvtu32_sd(__m128d __A, unsigned __B) {
  // CHECK-LABEL: @test_mm_cvtu32_sd
  // CHECK: uitofp i32 %{{.*}} to double
  // CHECK: insertelement <2 x double> %{{.*}}, double %{{.*}}, i32 0
  return _mm_cvtu32_sd(__A, __B); 
}

#ifdef __x86_64__
__m128d test_mm_cvt_roundu64_sd(__m128d __A, unsigned long long __B) {
  // CHECK-LABEL: @test_mm_cvt_roundu64_sd
  // CHECK: @llvm.x86.avx512.cvtusi642sd
  return _mm_cvt_roundu64_sd(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128d test_mm_cvtu64_sd(__m128d __A, unsigned long long __B) {
  // CHECK-LABEL: @test_mm_cvtu64_sd
  // CHECK: uitofp i64 %{{.*}} to double
  // CHECK: insertelement <2 x double> %{{.*}}, double %{{.*}}, i32 0
  return _mm_cvtu64_sd(__A, __B); 
}
#endif

__m128 test_mm_cvt_roundu32_ss(__m128 __A, unsigned __B) {
  // CHECK-LABEL: @test_mm_cvt_roundu32_ss
  // CHECK: @llvm.x86.avx512.cvtusi2ss
  return _mm_cvt_roundu32_ss(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_cvtu32_ss(__m128 __A, unsigned __B) {
  // CHECK-LABEL: @test_mm_cvtu32_ss
  // CHECK: uitofp i32 %{{.*}} to float
  // CHECK: insertelement <4 x float> %{{.*}}, float %{{.*}}, i32 0
  return _mm_cvtu32_ss(__A, __B); 
}

#ifdef __x86_64__
__m128 test_mm_cvt_roundu64_ss(__m128 __A, unsigned long long __B) {
  // CHECK-LABEL: @test_mm_cvt_roundu64_ss
  // CHECK: @llvm.x86.avx512.cvtusi642ss
    return _mm_cvt_roundu64_ss(__A, __B, _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
}

__m128 test_mm_cvtu64_ss(__m128 __A, unsigned long long __B) {
  // CHECK-LABEL: @test_mm_cvtu64_ss
  // CHECK: uitofp i64 %{{.*}} to float
  // CHECK: insertelement <4 x float> %{{.*}}, float %{{.*}}, i32 0
  return _mm_cvtu64_ss(__A, __B); 
}
#endif

int test_mm_cvtss_i32(__m128 A) {
  // CHECK-LABEL: test_mm_cvtss_i32
  // CHECK: call i32 @llvm.x86.sse.cvtss2si(<4 x float> %{{.*}})
  return _mm_cvtss_i32(A);
}

#ifdef __x86_64__
long long test_mm_cvtss_i64(__m128 A) {
  // CHECK-LABEL: test_mm_cvtss_i64
  // CHECK: call i64 @llvm.x86.sse.cvtss2si64(<4 x float> %{{.*}})
  return _mm_cvtss_i64(A);
}
#endif

__m128d test_mm_cvti32_sd(__m128d A, int B) {
  // CHECK-LABEL: test_mm_cvti32_sd
  // CHECK: sitofp i32 %{{.*}} to double
  // CHECK: insertelement <2 x double> %{{.*}}, double %{{.*}}, i32 0
  return _mm_cvti32_sd(A, B);
}

#ifdef __x86_64__
__m128d test_mm_cvti64_sd(__m128d A, long long B) {
  // CHECK-LABEL: test_mm_cvti64_sd
  // CHECK: sitofp i64 %{{.*}} to double
  // CHECK: insertelement <2 x double> %{{.*}}, double %{{.*}}, i32 0
  return _mm_cvti64_sd(A, B);
}
#endif

__m128 test_mm_cvti32_ss(__m128 A, int B) {
  // CHECK-LABEL: test_mm_cvti32_ss
  // CHECK: sitofp i32 %{{.*}} to float
  // CHECK: insertelement <4 x float> %{{.*}}, float %{{.*}}, i32 0
  return _mm_cvti32_ss(A, B);
}

#ifdef __x86_64__
__m128 test_mm_cvti64_ss(__m128 A, long long B) {
  // CHECK-LABEL: test_mm_cvti64_ss
  // CHECK: sitofp i64 %{{.*}} to float
  // CHECK: insertelement <4 x float> %{{.*}}, float %{{.*}}, i32 0
  return _mm_cvti64_ss(A, B);
}
#endif

int test_mm_cvtsd_i32(__m128d A) {
  // CHECK-LABEL: test_mm_cvtsd_i32
  // CHECK: call i32 @llvm.x86.sse2.cvtsd2si(<2 x double> %{{.*}})
  return _mm_cvtsd_i32(A);
}

#ifdef __x86_64__
long long test_mm_cvtsd_i64(__m128d A) {
  // CHECK-LABEL: test_mm_cvtsd_i64
  // CHECK: call i64 @llvm.x86.sse2.cvtsd2si64(<2 x double> %{{.*}})
  return _mm_cvtsd_i64(A);
}
#endif

__m128d test_mm_mask_cvtss_sd(__m128d __W, __mmask8 __U, __m128d __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_mask_cvtss_sd
  // CHECK: @llvm.x86.avx512.mask.cvtss2sd.round
  return _mm_mask_cvtss_sd(__W, __U, __A, __B); 
}

__m128d test_mm_maskz_cvtss_sd( __mmask8 __U, __m128d __A, __m128 __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtss_sd
  // CHECK: @llvm.x86.avx512.mask.cvtss2sd.round
  return _mm_maskz_cvtss_sd( __U, __A, __B); 
}

__m128 test_mm_mask_cvtsd_ss(__m128 __W, __mmask8 __U, __m128 __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_mask_cvtsd_ss
  // CHECK: @llvm.x86.avx512.mask.cvtsd2ss.round
  return _mm_mask_cvtsd_ss(__W, __U, __A, __B); 
}

__m128 test_mm_maskz_cvtsd_ss(__mmask8 __U, __m128 __A, __m128d __B) {
  // CHECK-LABEL: @test_mm_maskz_cvtsd_ss
  // CHECK: @llvm.x86.avx512.mask.cvtsd2ss.round
  return _mm_maskz_cvtsd_ss(__U, __A, __B); 
}

__mmask16 test_mm512_int2mask(int __a)
{
  // CHECK-LABEL: test_mm512_int2mask
  // CHECK: trunc i32 %{{.*}} to i16
  return _mm512_int2mask(__a);
}

int test_mm512_mask2int(__mmask16 __a)
{
  // CHECK-LABEL: test_mm512_mask2int
  // CHECK: zext i16 %{{.*}} to i32
  return _mm512_mask2int(__a);
}

__m128 test_mm_mask_move_ss (__m128 __W, __mmask8 __U, __m128 __A, __m128 __B)
{
  // CHECK-LABEL: @test_mm_mask_move_ss
  // CHECK: [[EXT:%.*]] = extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: insertelement <4 x float> %{{.*}}, float [[EXT]], i32 0
  // CHECK: [[A:%.*]] = extractelement <4 x float> [[VEC:%.*]], i64 0
  // CHECK-NEXT: [[B:%.*]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.*]] = select i1 %{{.*}}, float [[A]], float [[B]]
  // CHECK-NEXT: insertelement <4 x float> [[VEC]], float [[SEL]], i64 0
  return _mm_mask_move_ss ( __W,  __U,  __A,  __B);
}

__m128 test_mm_maskz_move_ss (__mmask8 __U, __m128 __A, __m128 __B)
{
  // CHECK-LABEL: @test_mm_maskz_move_ss
  // CHECK: [[EXT:%.*]] = extractelement <4 x float> %{{.*}}, i32 0
  // CHECK: insertelement <4 x float> %{{.*}}, float [[EXT]], i32 0
  // CHECK: [[A:%.*]] = extractelement <4 x float> [[VEC:%.*]], i64 0
  // CHECK-NEXT: [[B:%.*]] = extractelement <4 x float> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.*]] = select i1 %{{.*}}, float [[A]], float [[B]]
  // CHECK-NEXT: insertelement <4 x float> [[VEC]], float [[SEL]], i64 0
  return _mm_maskz_move_ss (__U, __A, __B);
}

__m128d test_mm_mask_move_sd (__m128d __W, __mmask8 __U, __m128d __A, __m128d __B)
{
  // CHECK-LABEL: @test_mm_mask_move_sd
  // CHECK: [[EXT:%.*]] = extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: insertelement <2 x double> %{{.*}}, double [[EXT]], i32 0
  // CHECK: [[A:%.*]] = extractelement <2 x double> [[VEC:%.*]], i64 0
  // CHECK-NEXT: [[B:%.*]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.*]] = select i1 %{{.*}}, double [[A]], double [[B]]
  // CHECK-NEXT: insertelement <2 x double> [[VEC]], double [[SEL]], i64 0
  return _mm_mask_move_sd ( __W,  __U,  __A,  __B);
}

__m128d test_mm_maskz_move_sd (__mmask8 __U, __m128d __A, __m128d __B)
{
  // CHECK-LABEL: @test_mm_maskz_move_sd
  // CHECK: [[EXT:%.*]] = extractelement <2 x double> %{{.*}}, i32 0
  // CHECK: insertelement <2 x double> %{{.*}}, double [[EXT]], i32 0
  // CHECK: [[A:%.*]] = extractelement <2 x double> [[VEC:%.*]], i64 0
  // CHECK-NEXT: [[B:%.*]] = extractelement <2 x double> %{{.*}}, i64 0
  // CHECK-NEXT: bitcast i8 %{{.*}} to <8 x i1>
  // CHECK-NEXT: extractelement <8 x i1> %{{.*}}, i64 0
  // CHECK-NEXT: [[SEL:%.*]] = select i1 %13, double [[A]], double [[B]]
  // CHECK-NEXT: insertelement <2 x double> [[VEC]], double [[SEL]], i64 0
  return _mm_maskz_move_sd (__U, __A, __B);
}

void test_mm_mask_store_ss(float * __P, __mmask8 __U, __m128 __A)
{
  // CHECK-LABEL: @test_mm_mask_store_ss
  // CHECK: call void @llvm.masked.store.v4f32.p0(<4 x float> %{{.*}}, ptr %{{.*}}, i32 1, <4 x i1> %{{.*}})
  _mm_mask_store_ss(__P, __U, __A);
}

void test_mm_mask_store_sd(double * __P, __mmask8 __U, __m128d __A)
{
  // CHECK-LABEL: @test_mm_mask_store_sd
  // CHECK: call void @llvm.masked.store.v2f64.p0(<2 x double> %{{.*}}, ptr %{{.*}}, i32 1, <2 x i1> %{{.*}})
  _mm_mask_store_sd(__P, __U, __A);
}

__m128 test_mm_mask_load_ss(__m128 __A, __mmask8 __U, const float* __W)
{
  // CHECK-LABEL: @test_mm_mask_load_ss
  // CHECK: call <4 x float> @llvm.masked.load.v4f32.p0(ptr %{{.*}}, i32 1, <4 x i1> %{{.*}}, <4 x float> %{{.*}})
  return _mm_mask_load_ss(__A, __U, __W);
}

__m128 test_mm_maskz_load_ss (__mmask8 __U, const float * __W)
{
  // CHECK-LABEL: @test_mm_maskz_load_ss
  // CHECK: call <4 x float> @llvm.masked.load.v4f32.p0(ptr %{{.*}}, i32 1, <4 x i1> %{{.*}}, <4 x float> %{{.*}})
  return _mm_maskz_load_ss (__U, __W);
}

__m128d test_mm_mask_load_sd (__m128d __A, __mmask8 __U, const double * __W)
{
  // CHECK-LABEL: @test_mm_mask_load_sd
  // CHECK: call <2 x double> @llvm.masked.load.v2f64.p0(ptr %{{.*}}, i32 1, <2 x i1> %{{.*}}, <2 x double> %{{.*}})
  return _mm_mask_load_sd (__A, __U, __W);
}

__m128d test_mm_maskz_load_sd (__mmask8 __U, const double * __W)
{
  // CHECK-LABEL: @test_mm_maskz_load_sd
  // CHECK: call <2 x double> @llvm.masked.load.v2f64.p0(ptr %{{.*}}, i32 1, <2 x i1> %{{.*}}, <2 x double> %{{.*}})
  return _mm_maskz_load_sd (__U, __W);
}
