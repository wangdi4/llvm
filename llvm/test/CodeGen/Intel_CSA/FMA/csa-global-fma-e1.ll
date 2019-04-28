; RUN: llc <%s -mtriple=csa -fp-contract=fast -enable-unsafe-fp-math -csa-global-fma | FileCheck %s

; This test checks that Global FMA optimizes the following test case.
; For the input expression:
;   +a*b*c*d+a*b*c*e+a*d*f*g+a*e*f*g+b*c*d+b*c*e+d*f*g+e*f*g+b*c+f*g;
; the output code must have only 5 arithmetic instructions:
;   F0=f*g; F1=d+e; F2=b*c+F0; F3=F1*a+F1; F4=F3*F2+F2;

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

define void @func32() {
; CHECK:        .entry  func32
; CHECK-DAG:    ld32    [[A:%.+]], a32
; CHECK-DAG:    ld32    [[B:%.+]], b32
; CHECK-DAG:    ld32    [[C:%.+]], c32
; CHECK-DAG:    ld32    [[D:%.+]], d32
; CHECK-DAG:    ld32    [[E:%.+]], e32
; CHECK-DAG:    ld32    [[F:%.+]], f32
; CHECK-DAG:    ld32    [[G:%.+]], g32
; CHECK-DAG:    mulf32  [[T0:%.+]], [[B]], [[C]]
; CHECK-DAG:    addf32  [[T1:%.+]], [[E]], [[D]]
; CHECK-DAG:    fmaf32  [[T2:%.+]], [[F]], [[G]], [[T0]]
; CHECK-DAG:    fmaf32  [[T3:%.+]], [[T1]], [[A]], [[T1]]
; CHECK-DAG:    fmaf32  [[T4:%.+]], [[T3]], [[T2]], [[T2]]
; CHECK-DAG:    st32    dst32, [[T4]]
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

define void @func64() {
; CHECK:        .entry  func64
; CHECK-DAG:    ld64    [[A:%.+]], a64
; CHECK-DAG:    ld64    [[B:%.+]], b64
; CHECK-DAG:    ld64    [[C:%.+]], c64
; CHECK-DAG:    ld64    [[D:%.+]], d64
; CHECK-DAG:    ld64    [[E:%.+]], e64
; CHECK-DAG:    ld64    [[F:%.+]], f64
; CHECK-DAG:    ld64    [[G:%.+]], g64
; CHECK-DAG:    mulf64  [[T0:%.+]], [[B]], [[C]]
; CHECK-DAG:    addf64  [[T1:%.+]], [[E]], [[D]]
; CHECK-DAG:    fmaf64  [[T2:%.+]], [[F]], [[G]], [[T0]]
; CHECK-DAG:    fmaf64  [[T3:%.+]], [[T1]], [[A]], [[T1]]
; CHECK-DAG:    fmaf64  [[T4:%.+]], [[T3]], [[T2]], [[T2]]
; CHECK-DAG:    st64    dst64, [[T4]]
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
