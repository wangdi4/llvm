; RUN: %oclopt -strip-intel-ip -verify -S %s | FileCheck %s

; global initializer must go
@global = internal local_unnamed_addr global i32 1, align 8
; CHECK-NOT: @global
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__global_pipes_ctor, i8* null }]
; global ctors/dtors must stay
; CHECK: @llvm.global_ctors{{.*}}@__global_pipes_ctor

; kernel wrapper must be present
declare !kernel_wrapper !1 void @kernel_separated_args()
; CHECK-LABEL: @kernel_separated_args

; kernel function is expected to be emptied
define void @kernel() {
entry:
 %x = load i32, i32* @global, align 8
 %y = add i32 %x, 1
 ret void
}

; CHECK-LABEL: @kernel
; CHECK-NEXT: exit:
; CHECK-NEXT: ret void

; internal function should dissappear completely
define internal i64 @function(i64 %a) {
entry:
  %x = add i64 %a, 1
  ret i64 %x
}

; CHECK-NOT: @function

; llvm intrinsic is expected to dissappear
declare void @llvm.stackprotector(i8*, i8**)
; CHECK-NOT: llvm.stackprotector

declare void @__global_pipes_ctor()

; global ctor/dtor must be declared
; CHECK: declare{{.*}}__global_pipes_ctor

!sycl.kernels = !{!0}

!0 = !{void ()* @kernel_separated_args}
!1 = !{void ()* @kernel}
