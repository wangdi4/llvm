; INTEL_CUSTOMIZATION:
; This test checks that Global FMA optimizes an arbitrary (1 of ~40k) test case.
; For the input expression:
;   +a*b*c*d+a*b*c*e+a*d*f*g+a*e*f*g+b*c*d+b*c*e+d*f*g+e*f*g+b*c+f*g;
; the output code must have only 5 arithmetic instructions:
;   F0=f*g; F1=d+e; F2=b*c+F0; F3=F1*a+F1; F4=F3*F2+F2;

; The tests are started with AVX2 and with SKX flags. The generated code is
; different now because the FMA form selection optimization is not enabled for
; SKX opcodes. the code should become identical for AVX2 and SKX after the
; FMA form selection is enabled for SKX.

; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mcpu=core-avx2 -fp-contract=fast -enable-unsafe-fp-math | FileCheck --check-prefix=ALL --check-prefix=AVX2 %s
; RUN: llc < %s -mtriple=x86_64-unknown-unknown -mcpu=skx       -fp-contract=fast -enable-unsafe-fp-math | FileCheck --check-prefix=ALL --check-prefix=AVX512 %s

; These attributes are used for functions testing scalar and 128/256 bit types.
attributes #0 = { nounwind "target-features"="+avx2,+fma" }

; These attributes are used for functions testing 512 bit types.
attributes #1 = { nounwind "target-cpu"="skx" "target-features"="+avx512f,+fma" }

@a32 = common global float 0.000000e+00, align 4
@b32 = common global float 0.000000e+00, align 4
@c32 = common global float 0.000000e+00, align 4
@d32 = common global float 0.000000e+00, align 4
@e32 = common global float 0.000000e+00, align 4
@f32 = common global float 0.000000e+00, align 4
@g32 = common global float 0.000000e+00, align 4
@dst32 = common global float 0.000000e+00, align 4
@h32 = common global float 0.000000e+00, align 4
@i32 = common global float 0.000000e+00, align 4

define void @func32() #0 {
; ALL-LABEL: func32:
; ALL:       # BB#0: # %entry
; ALL-NEXT:    vmovss {{.*#+}} xmm0 = mem[0],zero,zero,zero
; ALL-NEXT:    vmovss {{.*#+}} xmm1 = mem[0],zero,zero,zero
; ALL-NEXT:    vmovss {{.*#+}} xmm2 = mem[0],zero,zero,zero
; ALL-NEXT:    vmulss {{.*}}(%rip), %xmm0, %xmm0
; ALL-NEXT:    vaddss {{.*}}(%rip), %xmm1, %xmm1
; ALL-NEXT:    vfmadd231ss {{.*}}(%rip), %xmm2, %xmm0
; ALL-NEXT:    vfmadd132ss {{.*}}(%rip), %xmm1, %xmm1
; ALL-NEXT:    vfmadd213ss %xmm0, %xmm0, %xmm1
; ALL-NEXT:    vmovss %xmm1, {{.*}}(%rip)
; ALL-NEXT:    retq
entry:
  %load_a = load float, float* @a32, align 4
  %load_b = load float, float* @b32, align 4
  %mul = fmul fast float %load_a, %load_b
  %load_c = load float, float* @c32, align 4
  %mul1 = fmul fast float %mul, %load_c
  %load_d = load float, float* @d32, align 4
  %mul2 = fmul fast float %mul1, %load_d
  %load_e = load float, float* @e32, align 4
  %mul5 = fmul fast float %mul1, %load_e
  %add = fadd fast float %mul2, %mul5
  %mul6 = fmul fast float %load_a, %load_d
  %load_f = load float, float* @f32, align 4
  %mul7 = fmul fast float %mul6, %load_f
  %load_g = load float, float* @g32, align 4
  %mul8 = fmul fast float %mul7, %load_g
  %add9 = fadd fast float %add, %mul8
  %mul10 = fmul fast float %load_a, %load_e
  %mul11 = fmul fast float %mul10, %load_f
  %mul12 = fmul fast float %mul11, %load_g
  %add13 = fadd fast float %add9, %mul12
  %mul14 = fmul fast float %load_b, %load_c
  %mul15 = fmul fast float %mul14, %load_d
  %add16 = fadd fast float %add13, %mul15
  %mul18 = fmul fast float %mul14, %load_e
  %add19 = fadd fast float %add16, %mul18
  %mul20 = fmul fast float %load_d, %load_f
  %mul21 = fmul fast float %mul20, %load_g
  %add22 = fadd fast float %add19, %mul21
  %mul23 = fmul fast float %load_e, %load_f
  %mul24 = fmul fast float %mul23, %load_g
  %add25 = fadd fast float %add22, %mul24
  %add27 = fadd fast float %add25, %mul14
  %mul28 = fmul fast float %load_f, %load_g
  %add29 = fadd fast float %add27, %mul28
  store float %add29, float* @dst32, align 4
  ret void
}

@a64 = common global double 0.000000e+00, align 8
@b64 = common global double 0.000000e+00, align 8
@c64 = common global double 0.000000e+00, align 8
@d64 = common global double 0.000000e+00, align 8
@e64 = common global double 0.000000e+00, align 8
@f64 = common global double 0.000000e+00, align 8
@g64 = common global double 0.000000e+00, align 8
@dst64 = common global double 0.000000e+00, align 8
@h64 = common global double 0.000000e+00, align 8
@i64 = common global double 0.000000e+00, align 8

define void @func64() #0 {
; ALL-LABEL: func64:
; ALL:       # BB#0: # %entry
; ALL-NEXT:    vmovsd {{.*#+}} xmm0 = mem[0],zero
; ALL-NEXT:    vmovsd {{.*#+}} xmm1 = mem[0],zero
; ALL-NEXT:    vmovsd {{.*#+}} xmm2 = mem[0],zero
; ALL-NEXT:    vmulsd {{.*}}(%rip), %xmm0, %xmm0
; ALL-NEXT:    vaddsd {{.*}}(%rip), %xmm1, %xmm1
; ALL-NEXT:    vfmadd231sd {{.*}}(%rip), %xmm2, %xmm0
; ALL-NEXT:    vfmadd132sd {{.*}}(%rip), %xmm1, %xmm1
; ALL-NEXT:    vfmadd213sd %xmm0, %xmm0, %xmm1
; ALL-NEXT:    vmovsd %xmm1, {{.*}}(%rip)
; ALL-NEXT:    retq
entry:
  %load_a = load double, double* @a64, align 8
  %load_b = load double, double* @b64, align 8
  %mul = fmul fast double %load_a, %load_b
  %load_c = load double, double* @c64, align 8
  %mul1 = fmul fast double %mul, %load_c
  %load_d = load double, double* @d64, align 8
  %mul2 = fmul fast double %mul1, %load_d
  %load_e = load double, double* @e64, align 8
  %mul5 = fmul fast double %mul1, %load_e
  %add = fadd fast double %mul2, %mul5
  %mul6 = fmul fast double %load_a, %load_d
  %load_f = load double, double* @f64, align 8
  %mul7 = fmul fast double %mul6, %load_f
  %load_g = load double, double* @g64, align 8
  %mul8 = fmul fast double %mul7, %load_g
  %add9 = fadd fast double %add, %mul8
  %mul10 = fmul fast double %load_a, %load_e
  %mul11 = fmul fast double %mul10, %load_f
  %mul12 = fmul fast double %mul11, %load_g
  %add13 = fadd fast double %add9, %mul12
  %mul14 = fmul fast double %load_b, %load_c
  %mul15 = fmul fast double %mul14, %load_d
  %add16 = fadd fast double %add13, %mul15
  %mul18 = fmul fast double %mul14, %load_e
  %add19 = fadd fast double %add16, %mul18
  %mul20 = fmul fast double %load_d, %load_f
  %mul21 = fmul fast double %mul20, %load_g
  %add22 = fadd fast double %add19, %mul21
  %mul23 = fmul fast double %load_e, %load_f
  %mul24 = fmul fast double %mul23, %load_g
  %add25 = fadd fast double %add22, %mul24
  %add27 = fadd fast double %add25, %mul14
  %mul28 = fmul fast double %load_f, %load_g
  %add29 = fadd fast double %add27, %mul28
  store double %add29, double* @dst64, align 8
  ret void
}

@a32x4 = common global <4 x float> zeroinitializer, align 16
@b32x4 = common global <4 x float> zeroinitializer, align 16
@c32x4 = common global <4 x float> zeroinitializer, align 16
@d32x4 = common global <4 x float> zeroinitializer, align 16
@e32x4 = common global <4 x float> zeroinitializer, align 16
@f32x4 = common global <4 x float> zeroinitializer, align 16
@g32x4 = common global <4 x float> zeroinitializer, align 16
@dst32x4 = common global <4 x float> zeroinitializer, align 16
@h32x4 = common global <4 x float> zeroinitializer, align 16
@i32x4 = common global <4 x float> zeroinitializer, align 16

define void @func32x4() #0 {
; ALL-LABEL: func32x4:
; ALL:       # BB#0: # %entry
; ALL-NEXT:    vmovaps {{.*}}(%rip), %xmm0
; ALL-NEXT:    vmovaps {{.*}}(%rip), %xmm1
; ALL-NEXT:    vmovaps {{.*}}(%rip), %xmm2
; ALL-NEXT:    vmulps {{.*}}(%rip), %xmm0, %xmm0
; ALL-NEXT:    vaddps {{.*}}(%rip), %xmm1, %xmm1
; ALL-NEXT:    vfmadd231ps {{.*}}(%rip), %xmm2, %xmm0
; ALL-NEXT:    vfmadd132ps {{.*}}(%rip), %xmm1, %xmm1
; ALL-NEXT:    vfmadd213ps %xmm0, %xmm0, %xmm1
; ALL-NEXT:    vmovaps %xmm1, {{.*}}(%rip)
; ALL-NEXT:    retq
entry:
  %load_a = load <4 x float>, <4 x float>* @a32x4, align 16
  %load_b = load <4 x float>, <4 x float>* @b32x4, align 16
  %mul = fmul fast <4 x float> %load_a, %load_b
  %load_c = load <4 x float>, <4 x float>* @c32x4, align 16
  %mul1 = fmul fast <4 x float> %mul, %load_c
  %load_d = load <4 x float>, <4 x float>* @d32x4, align 16
  %mul2 = fmul fast <4 x float> %mul1, %load_d
  %load_e = load <4 x float>, <4 x float>* @e32x4, align 16
  %mul5 = fmul fast <4 x float> %mul1, %load_e
  %add = fadd fast <4 x float> %mul2, %mul5
  %mul6 = fmul fast <4 x float> %load_a, %load_d
  %load_f = load <4 x float>, <4 x float>* @f32x4, align 16
  %mul7 = fmul fast <4 x float> %mul6, %load_f
  %load_g = load <4 x float>, <4 x float>* @g32x4, align 16
  %mul8 = fmul fast <4 x float> %mul7, %load_g
  %add9 = fadd fast <4 x float> %add, %mul8
  %mul10 = fmul fast <4 x float> %load_a, %load_e
  %mul11 = fmul fast <4 x float> %mul10, %load_f
  %mul12 = fmul fast <4 x float> %mul11, %load_g
  %add13 = fadd fast <4 x float> %add9, %mul12
  %mul14 = fmul fast <4 x float> %load_b, %load_c
  %mul15 = fmul fast <4 x float> %mul14, %load_d
  %add16 = fadd fast <4 x float> %add13, %mul15
  %mul18 = fmul fast <4 x float> %mul14, %load_e
  %add19 = fadd fast <4 x float> %add16, %mul18
  %mul20 = fmul fast <4 x float> %load_d, %load_f
  %mul21 = fmul fast <4 x float> %mul20, %load_g
  %add22 = fadd fast <4 x float> %add19, %mul21
  %mul23 = fmul fast <4 x float> %load_e, %load_f
  %mul24 = fmul fast <4 x float> %mul23, %load_g
  %add25 = fadd fast <4 x float> %add22, %mul24
  %add27 = fadd fast <4 x float> %add25, %mul14
  %mul28 = fmul fast <4 x float> %load_f, %load_g
  %add29 = fadd fast <4 x float> %add27, %mul28
  store <4 x float> %add29, <4 x float>* @dst32x4, align 16
  ret void
}

@a64x2 = common global <2 x double> zeroinitializer, align 16
@b64x2 = common global <2 x double> zeroinitializer, align 16
@c64x2 = common global <2 x double> zeroinitializer, align 16
@d64x2 = common global <2 x double> zeroinitializer, align 16
@e64x2 = common global <2 x double> zeroinitializer, align 16
@f64x2 = common global <2 x double> zeroinitializer, align 16
@g64x2 = common global <2 x double> zeroinitializer, align 16
@dst64x2 = common global <2 x double> zeroinitializer, align 16
@h64x2 = common global <2 x double> zeroinitializer, align 16
@i64x2 = common global <2 x double> zeroinitializer, align 16

define void @func64x2() #0 {
; ALL-LABEL: func64x2:
; ALL:       # BB#0: # %entry
; ALL-NEXT:    vmovapd {{.*}}(%rip), %xmm0
; ALL-NEXT:    vmovapd {{.*}}(%rip), %xmm1
; ALL-NEXT:    vmovapd {{.*}}(%rip), %xmm2
; ALL-NEXT:    vmulpd {{.*}}(%rip), %xmm0, %xmm0
; ALL-NEXT:    vaddpd {{.*}}(%rip), %xmm1, %xmm1
; ALL-NEXT:    vfmadd231pd {{.*}}(%rip), %xmm2, %xmm0
; ALL-NEXT:    vfmadd132pd {{.*}}(%rip), %xmm1, %xmm1
; ALL-NEXT:    vfmadd213pd %xmm0, %xmm0, %xmm1
; ALL-NEXT:    vmovapd %xmm1, {{.*}}(%rip)
; ALL-NEXT:    retq
entry:
  %load_a = load <2 x double>, <2 x double>* @a64x2, align 16
  %load_b = load <2 x double>, <2 x double>* @b64x2, align 16
  %mul = fmul fast <2 x double> %load_a, %load_b
  %load_c = load <2 x double>, <2 x double>* @c64x2, align 16
  %mul1 = fmul fast <2 x double> %mul, %load_c
  %load_d = load <2 x double>, <2 x double>* @d64x2, align 16
  %mul2 = fmul fast <2 x double> %mul1, %load_d
  %load_e = load <2 x double>, <2 x double>* @e64x2, align 16
  %mul5 = fmul fast <2 x double> %mul1, %load_e
  %add = fadd fast <2 x double> %mul2, %mul5
  %mul6 = fmul fast <2 x double> %load_a, %load_d
  %load_f = load <2 x double>, <2 x double>* @f64x2, align 16
  %mul7 = fmul fast <2 x double> %mul6, %load_f
  %load_g = load <2 x double>, <2 x double>* @g64x2, align 16
  %mul8 = fmul fast <2 x double> %mul7, %load_g
  %add9 = fadd fast <2 x double> %add, %mul8
  %mul10 = fmul fast <2 x double> %load_a, %load_e
  %mul11 = fmul fast <2 x double> %mul10, %load_f
  %mul12 = fmul fast <2 x double> %mul11, %load_g
  %add13 = fadd fast <2 x double> %add9, %mul12
  %mul14 = fmul fast <2 x double> %load_b, %load_c
  %mul15 = fmul fast <2 x double> %mul14, %load_d
  %add16 = fadd fast <2 x double> %add13, %mul15
  %mul18 = fmul fast <2 x double> %mul14, %load_e
  %add19 = fadd fast <2 x double> %add16, %mul18
  %mul20 = fmul fast <2 x double> %load_d, %load_f
  %mul21 = fmul fast <2 x double> %mul20, %load_g
  %add22 = fadd fast <2 x double> %add19, %mul21
  %mul23 = fmul fast <2 x double> %load_e, %load_f
  %mul24 = fmul fast <2 x double> %mul23, %load_g
  %add25 = fadd fast <2 x double> %add22, %mul24
  %add27 = fadd fast <2 x double> %add25, %mul14
  %mul28 = fmul fast <2 x double> %load_f, %load_g
  %add29 = fadd fast <2 x double> %add27, %mul28
  store <2 x double> %add29, <2 x double>* @dst64x2, align 16
  ret void
}

@a32x8 = common global <8 x float> zeroinitializer, align 32
@b32x8 = common global <8 x float> zeroinitializer, align 32
@c32x8 = common global <8 x float> zeroinitializer, align 32
@d32x8 = common global <8 x float> zeroinitializer, align 32
@e32x8 = common global <8 x float> zeroinitializer, align 32
@f32x8 = common global <8 x float> zeroinitializer, align 32
@g32x8 = common global <8 x float> zeroinitializer, align 32
@dst32x8 = common global <8 x float> zeroinitializer, align 32
@h32x8 = common global <8 x float> zeroinitializer, align 32
@i32x8 = common global <8 x float> zeroinitializer, align 32

define void @func32x8() #0 {
; ALL-LABEL: func32x8:
; ALL:       # BB#0: # %entry
; ALL-NEXT:    vmovaps {{.*}}(%rip), %ymm0
; ALL-NEXT:    vmovaps {{.*}}(%rip), %ymm1
; ALL-NEXT:    vmovaps {{.*}}(%rip), %ymm2
; ALL-NEXT:    vmulps {{.*}}(%rip), %ymm0, %ymm0
; ALL-NEXT:    vaddps {{.*}}(%rip), %ymm1, %ymm1
; ALL-NEXT:    vfmadd231ps {{.*}}(%rip), %ymm2, %ymm0
; ALL-NEXT:    vfmadd132ps {{.*}}(%rip), %ymm1, %ymm1
; ALL-NEXT:    vfmadd213ps %ymm0, %ymm0, %ymm1
; ALL-NEXT:    vmovaps %ymm1, {{.*}}(%rip)
; AVX2-NEXT:   vzeroupper
; ALL-NEXT:    retq
entry:
  %load_a = load <8 x float>, <8 x float>* @a32x8, align 32
  %load_b = load <8 x float>, <8 x float>* @b32x8, align 32
  %mul = fmul fast <8 x float> %load_a, %load_b
  %load_c = load <8 x float>, <8 x float>* @c32x8, align 32
  %mul1 = fmul fast <8 x float> %mul, %load_c
  %load_d = load <8 x float>, <8 x float>* @d32x8, align 32
  %mul2 = fmul fast <8 x float> %mul1, %load_d
  %load_e = load <8 x float>, <8 x float>* @e32x8, align 32
  %mul5 = fmul fast <8 x float> %mul1, %load_e
  %add = fadd fast <8 x float> %mul2, %mul5
  %mul6 = fmul fast <8 x float> %load_a, %load_d
  %load_f = load <8 x float>, <8 x float>* @f32x8, align 32
  %mul7 = fmul fast <8 x float> %mul6, %load_f
  %load_g = load <8 x float>, <8 x float>* @g32x8, align 32
  %mul8 = fmul fast <8 x float> %mul7, %load_g
  %add9 = fadd fast <8 x float> %add, %mul8
  %mul10 = fmul fast <8 x float> %load_a, %load_e
  %mul11 = fmul fast <8 x float> %mul10, %load_f
  %mul12 = fmul fast <8 x float> %mul11, %load_g
  %add13 = fadd fast <8 x float> %add9, %mul12
  %mul14 = fmul fast <8 x float> %load_b, %load_c
  %mul15 = fmul fast <8 x float> %mul14, %load_d
  %add16 = fadd fast <8 x float> %add13, %mul15
  %mul18 = fmul fast <8 x float> %mul14, %load_e
  %add19 = fadd fast <8 x float> %add16, %mul18
  %mul20 = fmul fast <8 x float> %load_d, %load_f
  %mul21 = fmul fast <8 x float> %mul20, %load_g
  %add22 = fadd fast <8 x float> %add19, %mul21
  %mul23 = fmul fast <8 x float> %load_e, %load_f
  %mul24 = fmul fast <8 x float> %mul23, %load_g
  %add25 = fadd fast <8 x float> %add22, %mul24
  %add27 = fadd fast <8 x float> %add25, %mul14
  %mul28 = fmul fast <8 x float> %load_f, %load_g
  %add29 = fadd fast <8 x float> %add27, %mul28
  store <8 x float> %add29, <8 x float>* @dst32x8, align 32
  ret void
}

@a64x4 = common global <4 x double> zeroinitializer, align 32
@b64x4 = common global <4 x double> zeroinitializer, align 32
@c64x4 = common global <4 x double> zeroinitializer, align 32
@d64x4 = common global <4 x double> zeroinitializer, align 32
@e64x4 = common global <4 x double> zeroinitializer, align 32
@f64x4 = common global <4 x double> zeroinitializer, align 32
@g64x4 = common global <4 x double> zeroinitializer, align 32
@dst64x4 = common global <4 x double> zeroinitializer, align 32
@h64x4 = common global <4 x double> zeroinitializer, align 32
@i64x4 = common global <4 x double> zeroinitializer, align 32

define void @func64x4() #0 {
; ALL-LABEL: func64x4:
; ALL:       # BB#0: # %entry
; ALL-NEXT:    vmovapd {{.*}}(%rip), %ymm0
; ALL-NEXT:    vmovapd {{.*}}(%rip), %ymm1
; ALL-NEXT:    vmovapd {{.*}}(%rip), %ymm2
; ALL-NEXT:    vmulpd {{.*}}(%rip), %ymm0, %ymm0
; ALL-NEXT:    vaddpd {{.*}}(%rip), %ymm1, %ymm1
; ALL-NEXT:    vfmadd231pd {{.*}}(%rip), %ymm2, %ymm0
; ALL-NEXT:    vfmadd132pd {{.*}}(%rip), %ymm1, %ymm1
; ALL-NEXT:    vfmadd213pd %ymm0, %ymm0, %ymm1
; ALL-NEXT:    vmovapd %ymm1, {{.*}}(%rip)
; AVX2-NEXT:   vzeroupper
; ALL-NEXT:    retq
entry:
  %load_a = load <4 x double>, <4 x double>* @a64x4, align 32
  %load_b = load <4 x double>, <4 x double>* @b64x4, align 32
  %mul = fmul fast <4 x double> %load_a, %load_b
  %load_c = load <4 x double>, <4 x double>* @c64x4, align 32
  %mul1 = fmul fast <4 x double> %mul, %load_c
  %load_d = load <4 x double>, <4 x double>* @d64x4, align 32
  %mul2 = fmul fast <4 x double> %mul1, %load_d
  %load_e = load <4 x double>, <4 x double>* @e64x4, align 32
  %mul5 = fmul fast <4 x double> %mul1, %load_e
  %add = fadd fast <4 x double> %mul2, %mul5
  %mul6 = fmul fast <4 x double> %load_a, %load_d
  %load_f = load <4 x double>, <4 x double>* @f64x4, align 32
  %mul7 = fmul fast <4 x double> %mul6, %load_f
  %load_g = load <4 x double>, <4 x double>* @g64x4, align 32
  %mul8 = fmul fast <4 x double> %mul7, %load_g
  %add9 = fadd fast <4 x double> %add, %mul8
  %mul10 = fmul fast <4 x double> %load_a, %load_e
  %mul11 = fmul fast <4 x double> %mul10, %load_f
  %mul12 = fmul fast <4 x double> %mul11, %load_g
  %add13 = fadd fast <4 x double> %add9, %mul12
  %mul14 = fmul fast <4 x double> %load_b, %load_c
  %mul15 = fmul fast <4 x double> %mul14, %load_d
  %add16 = fadd fast <4 x double> %add13, %mul15
  %mul18 = fmul fast <4 x double> %mul14, %load_e
  %add19 = fadd fast <4 x double> %add16, %mul18
  %mul20 = fmul fast <4 x double> %load_d, %load_f
  %mul21 = fmul fast <4 x double> %mul20, %load_g
  %add22 = fadd fast <4 x double> %add19, %mul21
  %mul23 = fmul fast <4 x double> %load_e, %load_f
  %mul24 = fmul fast <4 x double> %mul23, %load_g
  %add25 = fadd fast <4 x double> %add22, %mul24
  %add27 = fadd fast <4 x double> %add25, %mul14
  %mul28 = fmul fast <4 x double> %load_f, %load_g
  %add29 = fadd fast <4 x double> %add27, %mul28
  store <4 x double> %add29, <4 x double>* @dst64x4, align 32
  ret void
}

@a32x16 = common global <16 x float> zeroinitializer, align 64
@b32x16 = common global <16 x float> zeroinitializer, align 64
@c32x16 = common global <16 x float> zeroinitializer, align 64
@d32x16 = common global <16 x float> zeroinitializer, align 64
@e32x16 = common global <16 x float> zeroinitializer, align 64
@f32x16 = common global <16 x float> zeroinitializer, align 64
@g32x16 = common global <16 x float> zeroinitializer, align 64
@dst32x16 = common global <16 x float> zeroinitializer, align 64
@h32x16 = common global <16 x float> zeroinitializer, align 64
@i32x16 = common global <16 x float> zeroinitializer, align 64

define void @func32x16() #1 {
; ALL-LABEL: func32x16:
; ALL:       # BB#0: # %entry
; ALL-NEXT:    vmovaps {{.*}}(%rip), %zmm0
; ALL-NEXT:    vmovaps {{.*}}(%rip), %zmm1
; ALL-NEXT:    vmovaps {{.*}}(%rip), %zmm2
; ALL-NEXT:    vmulps {{.*}}(%rip), %zmm0, %zmm0
; ALL-NEXT:    vaddps {{.*}}(%rip), %zmm1, %zmm1
; ALL-NEXT:    vfmadd231ps {{.*}}(%rip), %zmm2, %zmm0
; ALL-NEXT:    vfmadd132ps {{.*}}(%rip), %zmm1, %zmm1
; ALL-NEXT:    vfmadd213ps %zmm0, %zmm0, %zmm1
; ALL-NEXT:    vmovaps %zmm1, {{.*}}(%rip)
; ALL-NEXT:    retq
entry:
  %load_a = load <16 x float>, <16 x float>* @a32x16, align 64
  %load_b = load <16 x float>, <16 x float>* @b32x16, align 64
  %mul = fmul fast <16 x float> %load_a, %load_b
  %load_c = load <16 x float>, <16 x float>* @c32x16, align 64
  %mul1 = fmul fast <16 x float> %mul, %load_c
  %load_d = load <16 x float>, <16 x float>* @d32x16, align 64
  %mul2 = fmul fast <16 x float> %mul1, %load_d
  %load_e = load <16 x float>, <16 x float>* @e32x16, align 64
  %mul5 = fmul fast <16 x float> %mul1, %load_e
  %add = fadd fast <16 x float> %mul2, %mul5
  %mul6 = fmul fast <16 x float> %load_a, %load_d
  %load_f = load <16 x float>, <16 x float>* @f32x16, align 64
  %mul7 = fmul fast <16 x float> %mul6, %load_f
  %load_g = load <16 x float>, <16 x float>* @g32x16, align 64
  %mul8 = fmul fast <16 x float> %mul7, %load_g
  %add9 = fadd fast <16 x float> %add, %mul8
  %mul10 = fmul fast <16 x float> %load_a, %load_e
  %mul11 = fmul fast <16 x float> %mul10, %load_f
  %mul12 = fmul fast <16 x float> %mul11, %load_g
  %add13 = fadd fast <16 x float> %add9, %mul12
  %mul14 = fmul fast <16 x float> %load_b, %load_c
  %mul15 = fmul fast <16 x float> %mul14, %load_d
  %add16 = fadd fast <16 x float> %add13, %mul15
  %mul18 = fmul fast <16 x float> %mul14, %load_e
  %add19 = fadd fast <16 x float> %add16, %mul18
  %mul20 = fmul fast <16 x float> %load_d, %load_f
  %mul21 = fmul fast <16 x float> %mul20, %load_g
  %add22 = fadd fast <16 x float> %add19, %mul21
  %mul23 = fmul fast <16 x float> %load_e, %load_f
  %mul24 = fmul fast <16 x float> %mul23, %load_g
  %add25 = fadd fast <16 x float> %add22, %mul24
  %add27 = fadd fast <16 x float> %add25, %mul14
  %mul28 = fmul fast <16 x float> %load_f, %load_g
  %add29 = fadd fast <16 x float> %add27, %mul28
  store <16 x float> %add29, <16 x float>* @dst32x16, align 64
  ret void
}

@a64x8 = common global <8 x double> zeroinitializer, align 64
@b64x8 = common global <8 x double> zeroinitializer, align 64
@c64x8 = common global <8 x double> zeroinitializer, align 64
@d64x8 = common global <8 x double> zeroinitializer, align 64
@e64x8 = common global <8 x double> zeroinitializer, align 64
@f64x8 = common global <8 x double> zeroinitializer, align 64
@g64x8 = common global <8 x double> zeroinitializer, align 64
@dst64x8 = common global <8 x double> zeroinitializer, align 64
@h64x8 = common global <8 x double> zeroinitializer, align 64
@i64x8 = common global <8 x double> zeroinitializer, align 64

define void @func64x8() #1 {
; ALL-LABEL: func64x8:
; ALL:       # BB#0: # %entry
; ALL-NEXT:    vmovapd {{.*}}(%rip), %zmm0
; ALL-NEXT:    vmovapd {{.*}}(%rip), %zmm1
; ALL-NEXT:    vmovapd {{.*}}(%rip), %zmm2
; ALL-NEXT:    vmulpd {{.*}}(%rip), %zmm0, %zmm0
; ALL-NEXT:    vaddpd {{.*}}(%rip), %zmm1, %zmm1
; ALL-NEXT:    vfmadd231pd {{.*}}(%rip), %zmm2, %zmm0
; ALL-NEXT:    vfmadd132pd {{.*}}(%rip), %zmm1, %zmm1
; ALL-NEXT:    vfmadd213pd %zmm0, %zmm0, %zmm1
; ALL-NEXT:    vmovapd %zmm1, {{.*}}(%rip)
; ALL-NEXT:    retq
entry:
  %load_a = load <8 x double>, <8 x double>* @a64x8, align 64
  %load_b = load <8 x double>, <8 x double>* @b64x8, align 64
  %mul = fmul fast <8 x double> %load_a, %load_b
  %load_c = load <8 x double>, <8 x double>* @c64x8, align 64
  %mul1 = fmul fast <8 x double> %mul, %load_c
  %load_d = load <8 x double>, <8 x double>* @d64x8, align 64
  %mul2 = fmul fast <8 x double> %mul1, %load_d
  %load_e = load <8 x double>, <8 x double>* @e64x8, align 64
  %mul5 = fmul fast <8 x double> %mul1, %load_e
  %add = fadd fast <8 x double> %mul2, %mul5
  %mul6 = fmul fast <8 x double> %load_a, %load_d
  %load_f = load <8 x double>, <8 x double>* @f64x8, align 64
  %mul7 = fmul fast <8 x double> %mul6, %load_f
  %load_g = load <8 x double>, <8 x double>* @g64x8, align 64
  %mul8 = fmul fast <8 x double> %mul7, %load_g
  %add9 = fadd fast <8 x double> %add, %mul8
  %mul10 = fmul fast <8 x double> %load_a, %load_e
  %mul11 = fmul fast <8 x double> %mul10, %load_f
  %mul12 = fmul fast <8 x double> %mul11, %load_g
  %add13 = fadd fast <8 x double> %add9, %mul12
  %mul14 = fmul fast <8 x double> %load_b, %load_c
  %mul15 = fmul fast <8 x double> %mul14, %load_d
  %add16 = fadd fast <8 x double> %add13, %mul15
  %mul18 = fmul fast <8 x double> %mul14, %load_e
  %add19 = fadd fast <8 x double> %add16, %mul18
  %mul20 = fmul fast <8 x double> %load_d, %load_f
  %mul21 = fmul fast <8 x double> %mul20, %load_g
  %add22 = fadd fast <8 x double> %add19, %mul21
  %mul23 = fmul fast <8 x double> %load_e, %load_f
  %mul24 = fmul fast <8 x double> %mul23, %load_g
  %add25 = fadd fast <8 x double> %add22, %mul24
  %add27 = fadd fast <8 x double> %add25, %mul14
  %mul28 = fmul fast <8 x double> %load_f, %load_g
  %add29 = fadd fast <8 x double> %add27, %mul28
  store <8 x double> %add29, <8 x double>* @dst64x8, align 64
  ret void
}

