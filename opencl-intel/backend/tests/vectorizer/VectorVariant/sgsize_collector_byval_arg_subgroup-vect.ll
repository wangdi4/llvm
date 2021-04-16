; RUN: %oclopt %s -sg-size-collector -S | FileCheck %s

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

define void @kernel(%struct.A* nocapture readonly %arr) #0 !ocl_recommended_vector_length !0 {
entry:
  call void @f3(%struct.A* nocapture readonly %arr)
  ret void
}

attributes #0 = { "has-sub-groups" }

;; f1 does not have a vector variant.
; CHECK: attributes #0 = { "vector-variants"="_ZGVeM8v_f2,_ZGVeN8v_f2" }
; CHECK: attributes #1 = { "has-sub-groups" "vector-variants"="_ZGVeM8v_f3,_ZGVeN8v_f3" }
; CHECK: attributes #2 = { "has-sub-groups" }
; CHECK-NOT: attributes

!opencl.kernels = !{!1}

!0 = !{i32 8}
!1 = !{void (%struct.A*)* @kernel}
