; RUN: %oclopt %s -enable-direct-function-call-vectorization -sg-size-collector -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt %s -enable-direct-function-call-vectorization -sg-size-collector -S | FileCheck %s
; RUN: %oclopt %s -sg-size-collector -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt %s -sg-size-collector -S | FileCheck %s -check-prefix CHECK-NO-FLAG

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

define void @kernel() !ocl_recommended_vector_length !0 {
entry:
  call void @foo()
  ret void
}

define void @kernel2() !ocl_recommended_vector_length !1 {
entry:
  call void @bar()
  ret void
}

define void @kernel3() !ocl_recommended_vector_length !2 {
entry:
  call void @foo()
  call void @bar()
  ret void
}

attributes #0 = { "vector-variants"="_ZGVbM16_bar" }

; CHECK: attributes #[[ATTR0]] = { "vector-variants"="_ZGVbM16_bar,_ZGV{{[bcde]}}M8_bar,_ZGV{{[bcde]}}N8_bar,_ZGV{{[bcde]}}M32_bar,_ZGV{{[bcde]}}N32_bar" }
; CHECK: attributes #[[ATTR1]] = { "vector-variants"="_ZGV{{[bcde]}}M8_foo,_ZGV{{[bcde]}}N8_foo,_ZGV{{[bcde]}}M16_foo,_ZGV{{[bcde]}}N16_foo,_ZGV{{[bcde]}}M32_foo,_ZGV{{[bcde]}}N32_foo" }
; CHECK-NO-FLAG: attributes #0 = { "vector-variants"="_ZGVbM16_bar" }
; CHECK-NO-FLAG-NOT: vector-variants

!0 = !{i32 8}
!1 = !{i32 16}
!2 = !{i32 32}

; DEBUGIFY-NOT: WARNING
