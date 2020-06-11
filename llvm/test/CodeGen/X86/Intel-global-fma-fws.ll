; INTEL_CUSTOMIZATION:
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

; RUN: llc < %s -mtriple=x86_64-unknown-linux-gnu -fp-contract=fast -enable-unsafe-fp-math | FileCheck %s

; Function Attrs: nounwind uwtable
define dso_local void @_Z4funcv() #0 {
; CHECK-LABEL: _Z4funcv:
; CHECK:       %bb.0: # %entry
; CHECK:       vfmadd213sd {{.*#+}}
; CHECK-NEXT:  vmovsd  %xmm{{.*}}, dst1(%rip)
; CHECK-NEXT:  vfmsub213sd {{.*#+}}
; CHECK-NEXT:  vmovsd  %xmm{{.*}}, dst2(%rip)
; CHECK-NEXT:  retq
entry:
  %0 = load double, double* @gl_a, align 8
  %1 = load double, double* @gl_b, align 8
  %2 = load double, double* @gl_c, align 8
  %mul = fmul fast double %0, %1
  %add = fadd fast double %mul, %2
  store double %add, double* @dst1, align 8
  %sub = fsub fast double %mul, %2
  store double %sub, double* @dst2, align 8
  ret void
}

attributes #0 = { "target-cpu"="skylake"}

@dst1 = dso_local global double 0.000000e+00, align 8
@dst2 = dso_local global double 0.000000e+00, align 8
@gl_a = dso_local global double 0.000000e+00, align 8
@gl_b = dso_local global double 0.000000e+00, align 8
@gl_c = dso_local global double 0.000000e+00, align 8
