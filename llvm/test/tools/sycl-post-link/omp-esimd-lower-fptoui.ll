; RUN: sycl-post-link -ompoffload-link-entries -ompoffload-explicit-simd --ir-output-only %s -S -o - | FileCheck %s

define dso_local spir_func i32 @foo(float %x) {
  %y = fptoui float %x to i32
; check that the scalar float to unsigned int conversion is left intact
; CHECK: %y = fptoui float %x to i32
  ret i32 %y
}
