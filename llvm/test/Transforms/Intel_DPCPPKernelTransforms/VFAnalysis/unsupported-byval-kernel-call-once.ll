;; Have a byval function with kernel-call-once attribute, checks that vectorization is disabled.
; RUN: opt -dpcpp-kernel-add-function-attrs -dpcpp-kernel-vf-analysis -analyze -enable-new-pm=0 %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-NO-VECTORIZATION
; RUN: opt -passes="dpcpp-kernel-add-function-attrs,print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-NO-VECTORIZATION

; If vectorization on byval/byref function is enabled, no need to do emulation then.
; RUN: opt -dpcpp-enable-byval-byref-function-call-vectorization -dpcpp-kernel-add-function-attrs -dpcpp-kernel-vf-analysis -analyze -enable-new-pm=0 %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-VECTORIZATION
; RUN: opt -dpcpp-enable-byval-byref-function-call-vectorization -passes="dpcpp-kernel-add-function-attrs,print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s -check-prefixes=CHECK-VECTORIZATION

; CHECK-LABEL: Kernel --> VF:
; CHECK-NO-VECTORIZATION: <kernel> : 1
; CHECK-VECTORIZATION: <kernel> : 16

; CHECK-LABEL: Kernel --> SGEmuSize:

%struct.A = type { float, i32, double, i64 }

; dpcpp-kernel-add-function-attrs will add "kernel-call-once" to all barrier-calling functions.
define void @f1(%struct.A* byval(%struct.A) align 8 %arg) {
entry:
  call void @_Z18work_group_barrierj12memory_scope(i32 1, i32 1)
  ret void
}

define void @kernel(%struct.A* nocapture readonly %arr) !intel_reqd_sub_group_size !1 {
entry:
  %ptridx = getelementptr inbounds %struct.A, %struct.A* %arr, i64 0
  call void @f1(%struct.A* nonnull byval(%struct.A) align 8 %ptridx)
  ret void
}

declare void @_Z18work_group_barrierj12memory_scope(i32, i32)

!sycl.kernels = !{!0}

!0 = !{void (%struct.A*)* @kernel}
!1 = !{i32 16}
