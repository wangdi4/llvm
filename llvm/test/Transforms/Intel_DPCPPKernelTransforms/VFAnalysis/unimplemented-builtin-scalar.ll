; Checks scalar subgroup builtin is supported w/ subgroup emulation.
; RUN: opt -dpcpp-force-vf=1 -dpcpp-enable-subgroup-emulation -dpcpp-kernel-vf-analysis -analyze %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-EMULATION
; RUN: opt -dpcpp-force-vf=1 -dpcpp-enable-subgroup-emulation -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-EMULATION

; If subgroup emulation is disabled, then emits error on unimplemented scalar subgroup builtin.
; RUN: not opt -dpcpp-force-vf=1 -dpcpp-enable-subgroup-emulation=false -dpcpp-kernel-vf-analysis -analyze %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-NO-EMULATION
; RUN: not opt -dpcpp-force-vf=1 -dpcpp-enable-subgroup-emulation=false -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefix=CHECK-NO-EMULATION

; CHECK-EMULATION-LABEL: Kernel --> VF:
; CHECK-EMULATION: <test> : 1
; CHECK-EMULATION-LABEL: Kernel --> SGEmuSize:
; CHECK-EMULATION: <test> : 4

; CHECK-NO-EMULATION: error: function <test>: Unimplemented workgroup/subgroup builtin!

define void @test(i32 %a) "has-sub-groups" !kernel_has_sub_groups !{i1 true} {
  %call = call i32 @_Z13sub_group_alli(i32 %a)
  ret void
}

declare spir_func i32 @_Z13sub_group_alli(i32)

!sycl.kernels = !{!0}

!0 = !{void (i32)* @test}
