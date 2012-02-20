; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf

define double @f(double %d) {
  %ret = tail call x86_svmlcc double @__ocl_svml_b1_floorf2(double %d)
  ret double %ret
}

declare double @__ocl_svml_b1_floorf2(double) readnone
