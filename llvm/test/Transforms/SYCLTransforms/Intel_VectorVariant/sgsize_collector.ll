; RUN: opt %s -sycl-enable-direct-function-call-vectorization=true -passes=sycl-kernel-sg-size-collector -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -sycl-enable-direct-function-call-vectorization=true -passes=sycl-kernel-sg-size-collector -S | FileCheck %s
; RUN: opt %s -sycl-enable-direct-function-call-vectorization=false -passes=sycl-kernel-sg-size-collector -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -sycl-enable-direct-function-call-vectorization=false -passes=sycl-kernel-sg-size-collector -S | FileCheck %s -check-prefix CHECK-NO-FLAG

define void @bar() #0 {
; CHECK: define void @bar() #[[ATTR0:[0-9]+]] {
entry:
  call void @foo()
  ret void
}

define void @foo() {
; CHECK: define void @foo() #[[ATTR1:[0-9]+]] {
entry:
  call void @bar()
  ret void
}

define void @kernel() !recommended_vector_length !0 {
entry:
  call void @foo()
  ret void
}

define void @kernel2() !recommended_vector_length !1 {
entry:
  call void @bar()
  ret void
}

define void @kernel3() !recommended_vector_length !2 {
entry:
  call void @foo()
  call void @bar()
  ret void
}

attributes #0 = { "vector-variants"="_ZGVeM16_bar" }

; CHECK: attributes #[[ATTR0]] = { "vector-variants"="_ZGVeM16_bar,_ZGVbM8_bar,_ZGVbN8_bar,_ZGVbM32_bar,_ZGVbN32_bar" }
; CHECK: attributes #[[ATTR1]] = { "vector-variants"="_ZGVbM8_foo,_ZGVbN8_foo,_ZGVbM16_foo,_ZGVbN16_foo,_ZGVbM32_foo,_ZGVbN32_foo" }
; CHECK-NO-FLAG: attributes #0 = { "vector-variants"="_ZGVeM16_bar" }
; CHECK-NO-FLAG-NOT: vector-variants

!sycl.kernels = !{!3}

!0 = !{i32 8}
!1 = !{i32 16}
!2 = !{i32 32}
!3 = !{ptr @kernel, ptr @kernel2, ptr @kernel3}

; DEBUGIFY-NOT: WARNING
