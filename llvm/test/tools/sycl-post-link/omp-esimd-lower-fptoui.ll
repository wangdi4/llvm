; RUN: sycl-post-link -ompoffload-link-entries -ompoffload-explicit-simd --ir-output-only %s -S -o - | FileCheck %s

; TODO: SPIR_KERNEL CC requires 'void' return type,
;       but that'd disable SIMD lowering. Need to change
;       CC to spir_func and add required MD to trigger lowering

define dso_local spir_kernel i32 @foo(float %x) !omp_simd_kernel !0 {
  %y = fptoui float %x to i32
; check that the scalar float to unsigned int conversion is left intact
; CHECK: %y = fptoui float %x to i32
  %z = fptosi float %x to i16
; CHECK: %z = fptosi float %x to i16
  %z.ext = zext i16 %z to i32
  %res = add i32 %y, %z.ext
  ret i32 %res
}

!0 = !{}
