; RUN: opt -dpcpp-kernel-add-implicit-args %s -S | FileCheck %s
; check the metadata is preserved correctly during transformations

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: noduplicate nounwind
define void @test() #0 !kernel_arg_addr_space !0 {
entry:
  ret void
}

; CHECK: @test{{.*}} #0 !kernel_arg_addr_space ![[AS:[0-9]+]]

; Function Attrs: noduplicate nounwind
define void @__Vectorized_.test() #0 !kernel_arg_addr_space !0 {
entry:
  ret void
}

; CHECK: @__Vectorized_.test{{.*}} #0 !kernel_arg_addr_space ![[AS:[0-9]+]]

attributes #0 = { "sycl_kernel" }

!0 = !{}

; CHECK: ![[AS]] = !{}
