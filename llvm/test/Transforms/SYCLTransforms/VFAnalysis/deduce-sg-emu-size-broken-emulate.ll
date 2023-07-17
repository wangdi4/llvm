; Deduce subgroup emulation size when subgroup semantics is broken.; RUN: opt -sycl-vector-variant-isa-encoding-override=AVX512Core -sycl-enable-subgroup-emulation -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-EMULATION

; If emulation is not enabled, then emit diagnosis.
; RUN: not opt -sycl-enable-subgroup-emulation=false -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-NO-EMULATION

; CHECK-EMULATION-LABEL: Kernel --> VF:
; CHECK-EMULATION-COUNT-2: <{{.*}}> : 1

; CHECK-EMULATION-LABEL: Kernel --> SGEmuSize:
; CHECK-EMULATION-DAG: <reqd> : 8
; CHECK-EMULATION-DAG: <sg_emu_size> : 16

; CHECK-NO-EMULATION: error: kernel "{{reqd|sg_emu_size}}": Subgroup calls in scalar function can't be resolved

define void @reqd() noinline optnone "has-sub-groups" !kernel_has_sub_groups !{i1 true} !intel_reqd_sub_group_size !{i32 8} {
  ret void
}

define void @sg_emu_size() noinline optnone "has-sub-groups" !kernel_has_sub_groups !{i1 true} !sg_emu_size !{i32 16} {
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @reqd, ptr @sg_emu_size}
