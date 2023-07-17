; RUN: opt -passes="print<sycl-kernel-vf-analysis>" %s -S 2>&1 | FileCheck %s

;; Have a has-sub-group kernel with byref parameter.
;; We can proceed with vectorization as the kernel args are uniform.

; CHECK-LABEL: Kernel --> VF:

; CHECK: <kernel> : 16

; CHECK-LABEL: Kernel --> SGEmuSize:

%struct.A = type { float, i32, double, i64 }


define void @f1(ptr byval(%struct.A) align 8 %arg) {
entry:
  ret void
}

define void @f2(ptr nocapture readonly %arr) {
  call void @f1(ptr nonnull byval(%struct.A) align 8 %arr)
  ret void
}

define void @f3(ptr nocapture readonly %arr) #0 {
  call void @f2(ptr nocapture readonly %arr)
  ret void
}

define void @kernel(ptr nocapture readonly %arr, ptr byval(%struct.A) align 8 %arg) #0 !kernel_has_sub_groups !1 !intel_reqd_sub_group_size !2 {
entry:
  call void @f3(ptr nocapture readonly %arr)
  ret void
}

attributes #0 = { "has-sub-groups" }

!sycl.kernels = !{!0}

!0 = !{ptr @kernel}
!1 = !{i1 true}
!2 = !{i32 16}
!3 = !{!"%struct.A"}
!4 = !{ptr null}
!5 = !{!"%struct.A*"}
!6 = !{ptr null}
!7 = !{!"%struct.A*", !"%struct.A"}
!8 = !{ptr null, ptr null}
