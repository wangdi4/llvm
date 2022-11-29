; RUN: opt %s -passes=dpcpp-kernel-update-call-attrs -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt %s -passes=dpcpp-kernel-update-call-attrs -S | FileCheck %s

define void @bar() #0 {
entry:
  ret void
}

define void @foo() #1 {
entry:
  call void @bar()
; CHECK: call void @bar() #0
  ret void
}

define void @kernel1() {
entry:
  call void @foo()
; CHECK: call void @foo() #1
  ret void
}

define void @kernel2() {
entry:
  call void @bar()
; CHECK: call void @bar() #0
  ret void
}

attributes #0 = { "vector-variants"="_ZGVxN8_bar,_ZGVxN16_bar,_ZGVxN32_bar" }
attributes #1 = { "vector-variants"="_ZGVxN8_foo,_ZGVxN16_foo,_ZGVxN32_foo" }

; DEBUGIFY-NOT: WARNING
