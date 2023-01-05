; RUN: opt %s -dpcpp-enable-byval-byref-function-call-vectorization -passes=dpcpp-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -passes=dpcpp-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -enable-debugify -disable-output -dpcpp-enable-byval-byref-function-call-vectorization -passes=dpcpp-kernel-sg-size-collector -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt %s -enable-debugify -disable-output -passes=dpcpp-kernel-sg-size-collector -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

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

define void @kernel(%struct.A* nocapture readonly %arr) #0 !kernel_has_sub_groups !1 !recommended_vector_length !2 !sg_emu_size !3 !no_barrier_path !4 {
entry:
  %ptridx = getelementptr inbounds %struct.A, %struct.A* %arr, i64 0
  call void @f1(%struct.A* nonnull byval(%struct.A) align 8 %ptridx)
  ret void
}

attributes #0 = { "has-sub-groups" }
;; We don't needs vector variants if the recommended_vector_length is 1.
; CHECK-NOT: vector-variants

!sycl.kernels = !{!0}

!0 = !{void (%struct.A*)* @kernel}
!1 = !{i1 true}
!2 = !{i32 1}
!3 = !{i32 16}
!4 = !{i1 false}

; DEBUGIFY-NOT: WARNING
