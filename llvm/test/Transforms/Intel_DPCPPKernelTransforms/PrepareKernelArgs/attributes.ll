; RUN: opt -dpcpp-kernel-add-implicit-args -dpcpp-kernel-prepare-args -S %s | FileCheck %s
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-prepare-args' -S %s | FileCheck %s

; Checks that the attributes of the old kernel is copied to the wrapper. And
; for the old kernel, these attributes are also kept, and 'alwaysinline' is
; added.

; CHECK: define void @__test_separated_args
; CHECK-SAME: #[[ATTR0:[0-9]+]]
; CHECK: define void @test
; CHECK-SAME: #[[ATTR1:[0-9]+]]

; CHECK-DAG: attributes #[[ATTR0]] = { alwaysinline
; CHECK-DAG: attributes #[[ATTR1]] = { noinline

define void @test(i32 %a) #0 {
  ret void
}

attributes #0 = { noinline "sycl_kernel" }
