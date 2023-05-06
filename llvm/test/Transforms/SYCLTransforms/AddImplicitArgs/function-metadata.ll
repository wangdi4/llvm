; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-implicit-args %s -S | FileCheck %s

; check the metadata is preserved correctly during transformations

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: noduplicate nounwind
define void @test() !kernel_arg_addr_space !0 {
entry:
  ret void
}

; CHECK: @test{{.*}} !kernel_arg_addr_space ![[AS:[0-9]+]]

; Function Attrs: noduplicate nounwind
define void @__Vectorized_.test() !kernel_arg_addr_space !0 {
entry:
  ret void
}

; CHECK: @__Vectorized_.test{{.*}} !kernel_arg_addr_space ![[AS:[0-9]+]]


!0 = !{}

; CHECK: ![[AS]] = !{}

; DEBUGIFY-NOT: WARNING

!sycl.kernels = !{!1}
!1 = !{void ()* @test, void ()* @__Vectorized_.test}
