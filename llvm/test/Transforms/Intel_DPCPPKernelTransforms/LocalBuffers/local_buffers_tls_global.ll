; RUN: opt -dpcpp-kernel-enable-tls-globals -passes=dpcpp-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -passes=dpcpp-kernel-local-buffers -S %s | FileCheck %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-local-buffers -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

@main_kernel.local_arr = internal addrspace(3) global i32 undef, align 4
@pLocalMemBase = linkonce_odr thread_local global i8 addrspace(3)* undef, align 8

define void @main_kernel() {
entry:
; CHECK: %LocalMemBase = load i8 addrspace(3)*, i8 addrspace(3)** @pLocalMemBase, align 8
; CHECK: %0 = getelementptr i8, i8 addrspace(3)* %LocalMemBase, i32 0
; CHECK: %1 = addrspacecast i8 addrspace(3)* %0 to i32 addrspace(3)**
; CHECK: %2 = load i32 addrspace(3)*, i32 addrspace(3)** %1, align 8
; CHECK: store i32 1, i32 addrspace(3)* %2, align 4
  store i32 1, i32 addrspace(3)* @main_kernel.local_arr, align 4
  ret void
}

!opencl.kernels = !{!0}

!0 = !{void ()* @main_kernel}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
