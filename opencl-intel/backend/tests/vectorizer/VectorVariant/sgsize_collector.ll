; RUN: %oclopt %s -ocl-vector-variant-isa-encoding-override=AVX512Core -enable-direct-function-call-vectorization=true -sg-size-collector -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt %s -ocl-vector-variant-isa-encoding-override=AVX512Core -enable-direct-function-call-vectorization=true -sg-size-collector -S | FileCheck %s
; RUN: %oclopt %s -ocl-vector-variant-isa-encoding-override=AVX512Core -enable-direct-function-call-vectorization=false -sg-size-collector -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt %s -ocl-vector-variant-isa-encoding-override=AVX512Core -enable-direct-function-call-vectorization=false -sg-size-collector -S | FileCheck %s -check-prefix CHECK-NO-FLAG

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

attributes #0 = { "vector-variants"="_ZGVbM16_bar" }

; CHECK: attributes #[[ATTR0]] = { "vector-variants"="_ZGVbM16_bar,_ZGVeM8_bar,_ZGVeN8_bar,_ZGVeM32_bar,_ZGVeN32_bar" }
; CHECK: attributes #[[ATTR1]] = { "vector-variants"="_ZGVeM8_foo,_ZGVeN8_foo,_ZGVeM16_foo,_ZGVeN16_foo,_ZGVeM32_foo,_ZGVeN32_foo" }
; CHECK-NO-FLAG: attributes #0 = { "vector-variants"="_ZGVbM16_bar" }
; CHECK-NO-FLAG-NOT: vector-variants

!0 = !{i32 8}
!1 = !{i32 16}
!2 = !{i32 32}

; DEBUGIFY-NOT: WARNING
