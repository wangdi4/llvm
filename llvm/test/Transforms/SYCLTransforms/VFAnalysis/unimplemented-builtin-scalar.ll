; Checks scalar subgroup builtin is supported w/ subgroup emulation.
; RUN: opt -sycl-force-vf=1 -sycl-enable-subgroup-emulation -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-EMULATION

; If subgroup emulation is disabled, then emits error on unimplemented scalar subgroup builtin.
; RUN: not opt -sycl-force-vf=1 -sycl-enable-subgroup-emulation=false -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-NO-EMULATION

; CHECK-EMULATION-LABEL: Kernel --> VF:
; CHECK-EMULATION: <test> : 1
; CHECK-EMULATION-LABEL: Kernel --> SGEmuSize:
; CHECK-EMULATION: <test> : 4

; CHECK-NO-EMULATION: error: kernel "test": Unimplemented function(s): _Z13sub_group_alli with vectorization width 1

define void @test(i32 %a) "has-sub-groups" !kernel_has_sub_groups !{i1 true} {
  %call = call i32 @_Z13sub_group_alli(i32 %a)
  ret void
}

declare spir_func i32 @_Z13sub_group_alli(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test}
