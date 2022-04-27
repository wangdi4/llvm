; RUN: opt -dpcpp-kernel-vf-analysis -analyze -enable-new-pm=0 %s -S 2>&1 | FileCheck %s
; RUN: opt -passes="print<dpcpp-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

;; Have a has-sub-group kernel with byref parameter.
;; We can proceed with vectorization as the kernel args are uniform.

; CHECK-LABEL: Kernel --> VF:

; CHECK: <kernel> : 16

; CHECK-LABEL: Kernel --> SGEmuSize:

%struct.A = type { float, i32, double, i64 }


define void @f1(%struct.A* byval(%struct.A) align 8 %arg) {
entry:
  ret void
}

define void @f2(%struct.A* nocapture readonly %arr) {
  %ptridx = getelementptr inbounds %struct.A, %struct.A* %arr, i64 0
  call void @f1(%struct.A* nonnull byval(%struct.A) align 8 %ptridx)
  ret void
}

define void @f3(%struct.A* nocapture readonly %arr) #0 {
  call void @f2(%struct.A* nocapture readonly %arr)
  ret void
}

define void @kernel(%struct.A* nocapture readonly %arr, %struct.A* byval(%struct.A) align 8 %arg) #0 !kernel_has_sub_groups !1 !intel_reqd_sub_group_size !2 {
entry:
  call void @f3(%struct.A* nocapture readonly %arr)
  ret void
}

attributes #0 = { "has-sub-groups" }

!sycl.kernels = !{!0}

!0 = !{void (%struct.A*,%struct.A*)* @kernel}
!1 = !{i1 true}
!2 = !{i32 16}
