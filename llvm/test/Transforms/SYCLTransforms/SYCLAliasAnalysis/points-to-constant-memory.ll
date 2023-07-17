; RUN: opt -passes=instcombine -aa-pipeline=sycl-kernel-aa -S %s | FileCheck %s

; This test checks that store to constant memory is removed by instcombine.

; CHECK-NOT: store

define void @test(ptr addrspace(2) %c) !kernel_arg_base_type !0 !arg_type_null_val !1 {
  store i8 0, ptr addrspace(2) %c
  ret void
}

!0 = !{!"char*"}
!1 = !{ptr addrspace(2) null}
