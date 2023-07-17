; Checks that "sg_emu_size" metadata value is set properly according to ISA.

; Debugify check
; RUN: opt -sycl-enable-subgroup-emulation -passes=sycl-kernel-set-vf -sycl-vector-variant-isa-encoding-override=SSE42 %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; SSE42
; RUN: opt -sycl-enable-subgroup-emulation -passes=sycl-kernel-set-vf -sycl-vector-variant-isa-encoding-override=SSE42 %s -S | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-SSE42

; AVX1
; RUN: opt -sycl-enable-subgroup-emulation -passes=sycl-kernel-set-vf -sycl-vector-variant-isa-encoding-override=AVX1 %s -S | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-AVX1

; AVX2
; RUN: opt -sycl-enable-subgroup-emulation -passes=sycl-kernel-set-vf -sycl-vector-variant-isa-encoding-override=AVX2 %s -S | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-AVX2

; AVX512Core
; RUN: opt -sycl-enable-subgroup-emulation -passes=sycl-kernel-set-vf -sycl-vector-variant-isa-encoding-override=AVX512Core %s -S | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-AVX512Core

define void @kernel() "has-sub-groups" !kernel_has_sub_groups !{i1 true} !intel_vec_len_hint !{i32 1} {
  ret void
}

define void @non_kernel() {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @kernel}

; CHECK-COMMON: define void @kernel
; CHECK-COMMON-SAME: !recommended_vector_length ![[#VL_METADATA_ID:]]
; CHECK-COMMON-SAME: !sg_emu_size ![[#EMU_METADATA_ID:]]
; CHECK-COMMON-SAME: !no_barrier_path ![[#NOBARRIER_METADATA_ID:]]

; Don't attach metadata on non-kernel functions.
; CHECK-COMMON: define void @non_kernel
; CHECK-NOT: !recommended_vector_length

; CHECK-COMMON: ![[#VL_METADATA_ID]] = !{i32 1}
; CHECK-COMMON: ![[#EMU_METADATA_ID]] = !{i32
; CHECK-SSE42-SAME: 4
; CHECK-AVX1-SAME: 4
; CHECK-AVX2-SAME: 8
; CHECK-AVX512Core-SAME: 16
; CHECK-COMMON: ![[#NOBARRIER_METADATA_ID]] = !{i1 false}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
