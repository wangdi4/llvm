; RUN: %oclopt %s -enable-byval-byref-function-call-vectorization -sg-size-collector -S | FileCheck %s
; RUN: %oclopt %s -sg-size-collector -S | FileCheck %s

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

define void @kernel(%struct.A* nocapture readonly %arr) #0 !kernel_has_sub_groups !1 !ocl_recommended_vector_length !2 !sg_emu_size !3 !no_barrier_path !4 {
entry:
  %ptridx = getelementptr inbounds %struct.A, %struct.A* %arr, i64 0
  call void @f1(%struct.A* nonnull byval(%struct.A) align 8 %ptridx)
  ret void
}

attributes #0 = { "has-sub-groups" }
;; We don't needs vector variants if the ocl_recommended_vector_length is 1.
; CHECK-NOT: vector-variants

!opencl.kernels = !{!0}

!0 = !{void (%struct.A*)* @kernel}
!1 = !{i1 true}
!2 = !{i32 1}
!3 = !{i32 16}
!4 = !{i1 false}

