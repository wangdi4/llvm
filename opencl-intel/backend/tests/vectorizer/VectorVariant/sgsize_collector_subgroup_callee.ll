; RUN: %oclopt %s -ocl-vector-variant-isa-encoding-override=AVX512Core -sg-size-collector -S -enable-direct-subgroup-function-call-vectorization -enable-direct-function-call-vectorization -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt %s -ocl-vector-variant-isa-encoding-override=AVX512Core -sg-size-collector -S -enable-direct-subgroup-function-call-vectorization -enable-direct-function-call-vectorization | FileCheck %s

define void @bar() #0 {
; CHECK: define void @bar() #[[ATTR0:[0-9]+]] {
entry:
  call void @foo()
  ret void
}

define void @foo() #1 {
; CHECK: define void @foo() #[[ATTR1:[0-9]+]] {
entry:
  call void @bar()
  ret void
}

define void @kernel() #1 !recommended_vector_length !0 {
; CHECK: define void @kernel() #[[ATTR2:[0-9]+]]
entry:
  call void @foo()
  ret void
}

define void @kernel2() #1 !recommended_vector_length !1 {
; CHECK: define void @kernel2() #[[ATTR3:[0-9]+]]
entry:
  call void @bar()
  ret void
}

define void @kernel3() #1 !recommended_vector_length !2 {
; CHECK: define void @kernel3() #[[ATTR4:[0-9]+]]
entry:
  call void @foo()
  call void @bar()
  ret void
}

attributes #0 = { "vector-variants"="_ZGVbM16_bar" "has-sub-groups" }
attributes #1 = { "has-sub-groups" }

; CHECK: attributes #[[ATTR0]] = { "has-sub-groups" "vector-variants"="_ZGVbM16_bar,_ZGVeM8_bar,_ZGVeN8_bar,_ZGVeM32_bar,_ZGVeN32_bar" }
; CHECK: attributes #[[ATTR1]] = { "has-sub-groups" "vector-variants"="_ZGVeM8_foo,_ZGVeN8_foo,_ZGVeM16_foo,_ZGVeN16_foo,_ZGVeM32_foo,_ZGVeN32_foo" }
; CHECK: attributes #[[ATTR2]] = { "has-sub-groups" "vector-variants"="_ZGVeM8_kernel,_ZGVeN8_kernel" }
; CHECK: attributes #[[ATTR3]] = { "has-sub-groups" "vector-variants"="_ZGVeM16_kernel2,_ZGVeN16_kernel2" }
; CHECK: attributes #[[ATTR4]] = { "has-sub-groups" "vector-variants"="_ZGVeM32_kernel3,_ZGVeN32_kernel3" }

!0 = !{i32 8}
!1 = !{i32 16}
!2 = !{i32 32}

; DEBUGIFY-NOT: WARNING
