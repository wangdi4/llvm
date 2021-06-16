; RUN: %oclopt %s -check-vf -S | FileCheck %s

;; Have a has-sub-group function calling byref function.
;; We can vectorize with serializartion of byval.

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

define void @kernel(%struct.A* nocapture readonly %arr) #0 !kernel_has_sub_groups !1 !ocl_recommended_vector_length !2 {
;; We have ocl_recommended_vector_length set to the original value.
; CHECK: !ocl_recommended_vector_length !2
entry:
  call void @f3(%struct.A* nocapture readonly %arr)
  ret void
}

attributes #0 = { "has-sub-groups" }

!opencl.kernels = !{!0}

!0 = !{void (%struct.A*)* @kernel}
!1 = !{i1 true}
!2 = !{i32 16}
; CHECK: !2 = !{i32 16}
