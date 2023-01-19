; RUN: opt %s -dpcpp-enable-direct-function-call-vectorization -dpcpp-enable-direct-subgroup-function-call-vectorization -dpcpp-enable-byval-byref-function-call-vectorization -passes=dpcpp-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -passes=dpcpp-kernel-sg-size-collector -S | FileCheck %s -check-prefix CHECK-NO-FLAG
; RUN: opt %s -enable-debugify -disable-output -dpcpp-enable-direct-function-call-vectorization -dpcpp-enable-direct-subgroup-function-call-vectorization -dpcpp-enable-byval-byref-function-call-vectorization -passes=dpcpp-kernel-sg-size-collector -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt %s -enable-debugify -disable-output -passes=dpcpp-kernel-sg-size-collector -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

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

define void @kernel(%struct.A* nocapture readonly %arr) !recommended_vector_length !0 {
entry:
  %ptridx = getelementptr inbounds %struct.A, %struct.A* %arr, i64 0
  call void @foo(%struct.A* nonnull byval(%struct.A) align 8 %ptridx)
  ret void
}

define void @kernel2(%struct.A* nocapture readonly %arr) !recommended_vector_length !1 {
entry:
  %ptridx = getelementptr inbounds %struct.A, %struct.A* %arr, i64 0
  call void @bar(%struct.A* nonnull byval(%struct.A) align 8 %ptridx)
  ret void
}

attributes #0 = { "vector-variants"="_ZGVeM16v_bar" }

; CHECK: attributes #[[ATTR0]] = { "vector-variants"="_ZGVeM16v_bar,_ZGVbM8v_bar,_ZGVbN8v_bar" }
; CHECK: attributes #[[ATTR1]] = { "vector-variants"="_ZGVbM8v_foo,_ZGVbN8v_foo,_ZGVbM16v_foo,_ZGVbN16v_foo" }
; CHECK-NOT: vector-variants
; CHECK-NO-FLAG: attributes #0 = { "vector-variants"="_ZGVeM16v_bar" }
; CHECK-NO-FLAG-NOT: vector-variants

!sycl.kernels = !{!2}

!0 = !{i32 8}
!1 = !{i32 16}
!2 = !{void (%struct.A*)* @kernel, void (%struct.A*)* @kernel2}

; DEBUGIFY-NOT: WARNING
