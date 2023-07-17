; Checks that "recommended_vector_length" metadata value is set properly according to ISA.

; Debugify check
; RUN: opt -passes=sycl-kernel-set-vf -sycl-vector-variant-isa-encoding-override=SSE42 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; SSE42
; RUN: opt -passes=sycl-kernel-set-vf -sycl-vector-variant-isa-encoding-override=SSE42 %s -S | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-SSE42

; AVX1
; RUN: opt -passes=sycl-kernel-set-vf -sycl-vector-variant-isa-encoding-override=AVX1 %s -S | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-AVX1

; AVX2
; RUN: opt -passes=sycl-kernel-set-vf -sycl-vector-variant-isa-encoding-override=AVX2 %s -S | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-AVX2

; AVX512Core
; RUN: opt -passes=sycl-kernel-set-vf -sycl-vector-variant-isa-encoding-override=AVX512Core %s -S | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-AVX512Core

define void @kernel() {
  ret void
}

define void @non_kernel() {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @kernel}

; CHECK-COMMON: define void @kernel
; CHECK-COMMON-SAME: !recommended_vector_length ![[#VL_METADATA_ID:]]

; Don't attach metadata on non-kernel functions.
; CHECK-COMMON: define void @non_kernel
; CHECK-NOT: !recommended_vector_length

; CHECK-COMMON: ![[#VL_METADATA_ID]] = !{i32
; CHECK-SSE42-SAME: 4
; CHECK-AVX1-SAME: 4
; CHECK-AVX2-SAME: 8
; CHECK-AVX512Core-SAME: 16

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
