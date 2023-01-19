; RUN: opt %s -dpcpp-enable-byval-byref-function-call-vectorization -passes=dpcpp-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -passes=dpcpp-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -enable-debugify -disable-output -dpcpp-enable-byval-byref-function-call-vectorization -passes=dpcpp-kernel-sg-size-collector -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt %s -enable-debugify -disable-output -passes=dpcpp-kernel-sg-size-collector -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

%struct.A = type { float, i32, double, i64 }

define void @foo(%struct.A* align 8 %arg) {
; CHECK: define void @foo(%struct.A* align 8 %arg) #[[#ATTR:]] {
entry:
  ret void
}

define void @kernel(%struct.A* byval(%struct.A) %arr) !recommended_vector_length !0 {
entry:
  %ptridx = getelementptr inbounds %struct.A, %struct.A* %arr, i64 0
  call void @foo(%struct.A* align 8 %ptridx)
  ret void
}

; CHECK: attributes #[[#ATTR]] = { "vector-variants"="_ZGVbM8v_foo,_ZGVbN8v_foo" }

!sycl.kernels = !{!1}

!0 = !{i32 8}
!1 = !{void (%struct.A*)* @kernel}

; DEBUGIFY-NOT: WARNING
