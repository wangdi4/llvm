;; Have a byval function with has-sub-groups attribute. Emulate.
; RUN: opt -dpcpp-enable-subgroup-emulation -dpcpp-kernel-vf-analysis -analyze -enable-new-pm=0 %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-NO-VECTORIZATION,CHECK-EMULATION
; RUN: opt -dpcpp-enable-subgroup-emulation -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-NO-VECTORIZATION,CHECK-EMULATION

; If subgroup emulation is not enabled, then subgroup is broken.
; RUN: not opt -dpcpp-enable-subgroup-emulation=false -dpcpp-kernel-vf-analysis -analyze -enable-new-pm=0 %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-SUBGROUP-BROKEN
; RUN: not opt -dpcpp-enable-subgroup-emulation=false -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-SUBGROUP-BROKEN

; If vectorization on byval/byref function is enabled, no need to do emulation then.
; RUN: opt -dpcpp-enable-byval-byref-function-call-vectorization -dpcpp-kernel-vf-analysis -analyze -enable-new-pm=0 %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-VECTORIZATION,CHECK-NO-EMULATION
; RUN: opt -dpcpp-enable-byval-byref-function-call-vectorization -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-COMMON,CHECK-VECTORIZATION,CHECK-NO-EMULATION

; CHECK-COMMON-LABEL: Kernel --> VF:
; CHECK-NO-VECTORIZATION: <kernel> : 1
; CHECK-VECTORIZATION: <kernel> : 16

; CHECK-COMMON-LABEL: Kernel --> SGEmuSize:
; CHECK-EMULATION: <kernel> : 16
; CHECK-NO-EMULATION: <kernel> : 0

; CHECK-SUBGROUP-BROKEN: error: kernel "kernel": Subgroup calls in scalar function can't be resolved

%struct.A = type { float, i32, double, i64 }


define void @f3() #0 {
  ret void
}

define void @f2() #0 {
  call void @f3()
  ret void
}

define void @f1(%struct.A* byval(%struct.A) align 8 %arg) #0 {
entry:
  call void @f2()
  ret void
}

define void @kernel(%struct.A* nocapture readonly %arr) #0 !kernel_has_sub_groups !1 !intel_reqd_sub_group_size !2 {
entry:
  %ptridx = getelementptr inbounds %struct.A, %struct.A* %arr, i64 0
  call void @f1(%struct.A* nonnull byval(%struct.A) align 8 %ptridx)
  ret void
}

attributes #0 = { "has-sub-groups" }

!sycl.kernels = !{!0}

!0 = !{void (%struct.A*)* @kernel}
!1 = !{i1 true}
!2 = !{i32 16}
