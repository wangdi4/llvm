; RUN: llc <%s -mtriple=csa -fp-contract=fast -enable-unsafe-fp-math -csa-global-fma | FileCheck %s

; This test checks that FWS module of Global FMA can optimize/fuse
; expressions having two or more users.
;
; IR for this test was generated from this simple test:
;    double dst1, dst2;
;    double gl_a, gl_b, gl_c;
;    void func() {
;      double a = gl_a;
;      double b = gl_b;
;      double c = gl_c;
;
;      double t = a*b;
;      dst1 = t+c;
;      dst2 = t-c;
;    }
;
; Global FMA is supposed to optimize replace 1 MUL, 1 ADD, 1SUB into
; 1 FMA and 1 FMS.
; The tricky part here is that the MUL has 2 independent users and the
; fusing/FWS'ing that MUL into ADD and SUB does not give improvement
; to those ADD and SUB. But it still efficient because we can eliminate
; the initial MUL by doing aggressive FWS:
;   dst1 = a*b+c;
;   dst2 = a*b-c;
;

define void @func() {
; CHECK:        .entry  func
; CHECK-DAG:    ld64    [[A:%.+]], gl_a
; CHECK-DAG:    ld64    [[B:%.+]], gl_b
; CHECK-DAG:    ld64    [[C:%.+]], gl_c
; CHECK-DAG:    fmaf64  [[T0:%.+]], [[A]], [[B]], [[C]]
; CHECK-DAG:    st64    dst1, [[T0]]
; CHECK-DAG:    fmsf64  [[T1:%.+]], [[A]], [[B]], [[C]]
; CHECK-DAG:    st64    dst2, [[T1]]
entry:
  %a = load double, double* @gl_a, align 8
  %b = load double, double* @gl_b, align 8
  %c = load double, double* @gl_c, align 8
  %mul = fmul fast double %a, %b
  %add = fadd fast double %mul, %c
  store double %add, double* @dst1, align 8
  %sub = fsub fast double %mul, %c
  store double %sub, double* @dst2, align 8
  ret void
}

@dst1 = common global double 0.000000e+00, align 8
@dst2 = common global double 0.000000e+00, align 8
@gl_a = common global double 0.000000e+00, align 8
@gl_b = common global double 0.000000e+00, align 8
@gl_c = common global double 0.000000e+00, align 8

