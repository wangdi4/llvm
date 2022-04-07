; RUN: %oclopt -strip-intel-ip -verify -S %s | FileCheck %s

@global = constant i32 4, align 4
; CHECK-NOT: @global

declare !kernel_wrapper !1 !not_ocl_func_metadata_1 !2 void @kernel_separated_args() #0
; CHECK-NOT: !not_ocl_func_metadata_1
; CHECK: !kernel_wrapper

define void @kernel() !not_ocl_func_metadata_2 !2  {
entry:
  %x = add i64 1, 1
  ret void
}
; CHECK-NOT: !not_ocl_func_metadata_2

attributes #0 = { alwaysinline }
; CHECK-NOT: attributes

!not.ocl.module.metadata = !{!2}
!llvm.module.flags = !{}
!sycl.kernels = !{!0}
; CHECK-NOT: !not.ocl.module.metadata
; CHECK-NOT: !llvm.module.flags
; CHECK: !sycl.kernels

!0 = !{void ()* @kernel_separated_args}
!1 = !{void ()* @kernel}
!2 = !{i32 4}
