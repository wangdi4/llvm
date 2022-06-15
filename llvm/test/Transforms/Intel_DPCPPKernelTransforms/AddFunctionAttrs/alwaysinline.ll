; RUN: opt -dpcpp-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-function-attrs -S %s | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-add-function-attrs -S < %s | FileCheck %s

; Check that alwaysinline is added to @positive which is called twice.

; CHECK: define internal void @positive() [[ATTR_ALWAYSINLINE:#[0-9]+]]
define internal void @positive() {
  ret void
}

; CHECK: define internal void @negative1()
; CHECK-NOT: #
define internal void @negative1() {
  ret void
}

; CHECK: define internal void @negative2() [[ATTR_NOINLINE:#[0-9]+]]
define internal void @negative2() #0 {
  ret void
}

define void @test() {
entry:
  call void @positive();
  call void @positive();

  call void @negative1();
  call void @negative1();
  call void @negative1();

  call void @negative2();
  call void @negative2();

  ret void
}

; CHECK: attributes [[ATTR_ALWAYSINLINE]] {{.*}} alwaysinline
; CHECK: attributes [[ATTR_NOINLINE]] {{.*}} noinline

attributes #0 = { noinline }

; DEBUGIFY-NOT: WARNING
