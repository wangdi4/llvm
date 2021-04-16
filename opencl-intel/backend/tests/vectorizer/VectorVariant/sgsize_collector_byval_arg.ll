; RUN: %oclopt %s -enable-byval-byref-function-call-vectorization -sg-size-collector -S | FileCheck %s
; RUN: %oclopt %s -sg-size-collector -S | FileCheck %s -check-prefix CHECK-NO-FLAG

%struct.A = type { float, i32, double, i64 }

define void @bar(%struct.A* byval(%struct.A) align 8 %arg) #0 {
; CHECK: define void @bar(%struct.A* byval(%struct.A) align 8 %arg) #[[ATTR0:[0-9]+]] {
entry:
  call void @foo(%struct.A* nonnull byval(%struct.A) align 8 %arg)
  ret void
}

define void @foo(%struct.A* byref(%struct.A) align 8 %arg) {
; CHECK: define void @foo(%struct.A* byref(%struct.A) align 8 %arg) #[[ATTR1:[0-9]+]] {
entry:
  call void @bar(%struct.A* nonnull byval(%struct.A) align 8 %arg)
  ret void
}

define void @kernel(%struct.A* nocapture readonly %arr) !ocl_recommended_vector_length !0 {
entry:
  %ptridx = getelementptr inbounds %struct.A, %struct.A* %arr, i64 0
  call void @foo(%struct.A* nonnull byval(%struct.A) align 8 %ptridx)
  ret void
}

define void @kernel2(%struct.A* nocapture readonly %arr) !ocl_recommended_vector_length !1 {
entry:
  %ptridx = getelementptr inbounds %struct.A, %struct.A* %arr, i64 0
  call void @bar(%struct.A* nonnull byval(%struct.A) align 8 %ptridx)
  ret void
}

attributes #0 = { "vector-variants"="_ZGVbM16v_bar" }

; CHECK: attributes #[[ATTR0]] = { "vector-variants"="_ZGVbM16v_bar,_ZGVeM8v_bar,_ZGVeN8v_bar" }
; CHECK: attributes #[[ATTR1]] = { "vector-variants"="_ZGVeM8v_foo,_ZGVeN8v_foo,_ZGVeM16v_foo,_ZGVeN16v_foo" }
; CHECK-NOT: vector-variants
; CHECK-NO-FLAG: attributes #0 = { "vector-variants"="_ZGVbM16v_bar" }
; CHECK-NO-FLAG-NOT: vector-variants

!opencl.kernels = !{!2}

!0 = !{i32 8}
!1 = !{i32 16}
!2 = !{void (%struct.A*)* @kernel, void (%struct.A*)* @kernel2}
