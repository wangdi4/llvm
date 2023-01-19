; RUN: opt %s -passes=dpcpp-kernel-sg-size-collector -dpcpp-enable-direct-subgroup-function-call-vectorization -dpcpp-enable-direct-function-call-vectorization -S | FileCheck %s
; RUN: opt %s -enable-debugify -disable-output -passes=dpcpp-kernel-sg-size-collector -dpcpp-enable-direct-subgroup-function-call-vectorization -dpcpp-enable-direct-function-call-vectorization -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

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

define void @kernel(%struct.A* nocapture readonly %arr) #0 !recommended_vector_length !0 {
entry:
  call void @f3(%struct.A* nocapture readonly %arr)
  ret void
}

attributes #0 = { "has-sub-groups" }

;; f1 does not have a vector variant.
; CHECK: attributes #0 = { "vector-variants"="_ZGVbM8v_f2,_ZGVbN8v_f2" }
; CHECK: attributes #1 = { "has-sub-groups" "vector-variants"="_ZGVbM8v_f3,_ZGVbN8v_f3" }
; CHECK: attributes #2 = { "has-sub-groups" }
; CHECK-NOT: attributes

!sycl.kernels = !{!1}

!0 = !{i32 8}
!1 = !{void (%struct.A*)* @kernel}

; DEBUGIFY-NOT: WARNING
