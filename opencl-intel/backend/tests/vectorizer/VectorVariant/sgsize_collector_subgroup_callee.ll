; RUN: %oclopt %s -sg-size-collector -S | FileCheck %s

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

define void @kernel() #1 !ocl_recommended_vector_length !0 {
; CHECK: define void @kernel() #[[ATTR2:[0-9]+]]
entry:
  call void @foo()
  ret void
}

define void @kernel2() #1 !ocl_recommended_vector_length !1 {
; CHECK: define void @kernel2() #[[ATTR3:[0-9]+]]
entry:
  call void @bar()
  ret void
}

define void @kernel3() #1 !ocl_recommended_vector_length !2 {
; CHECK: define void @kernel3() #[[ATTR4:[0-9]+]]
entry:
  call void @foo()
  call void @bar()
  ret void
}

attributes #0 = { "vector-variants"="_ZGVbM16_bar" "has-sub-groups" }
attributes #1 = { "has-sub-groups" }

; CHECK: attributes #[[ATTR0]] = { "has-sub-groups" "vector-variants"="_ZGVbM16_bar,_ZGV{{[bcde]}}M8_bar,_ZGV{{[bcde]}}N8_bar,_ZGV{{[bcde]}}M32_bar,_ZGV{{[bcde]}}N32_bar" }
; CHECK: attributes #[[ATTR1]] = { "has-sub-groups" "vector-variants"="_ZGV{{[bcde]}}M8_foo,_ZGV{{[bcde]}}N8_foo,_ZGV{{[bcde]}}M16_foo,_ZGV{{[bcde]}}N16_foo,_ZGV{{[bcde]}}M32_foo,_ZGV{{[bcde]}}N32_foo" }
; CHECKL attributes #[[ATTR2]] = { "has-sub-groups" "vector-variants"="_ZGV{{[bcde]}}M8_kernel,_ZGV{{[bcde]}}N8_kernel" }
; CHECKL attributes #[[ATTR3]] = { "has-sub-groups" "vector-variants"="_ZGV{{[bcde]}}M16_kernel2,_ZGV{{[bcde]}}N16_kernel2" }
; CHECKL attributes #[[ATTR4]] = { "has-sub-groups" "vector-variants"="_ZGV{{[bcde]}}M32_kernel3,_ZGV{{[bcde]}}N32_kernel3" }

!0 = !{i32 8}
!1 = !{i32 16}
!2 = !{i32 32}
