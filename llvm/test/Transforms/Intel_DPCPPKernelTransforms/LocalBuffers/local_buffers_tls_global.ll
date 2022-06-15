; RUN: opt -dpcpp-kernel-enable-tls-globals -passes=dpcpp-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -dpcpp-kernel-enable-tls-globals -passes=dpcpp-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -opaque-pointers -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-enable-tls-globals -passes=dpcpp-kernel-local-buffers -S %s | FileCheck %s -check-prefix=NONOPAQUE
; RUN: opt -opaque-pointers -dpcpp-kernel-enable-tls-globals -passes=dpcpp-kernel-local-buffers -S %s | FileCheck %s -check-prefix=OPAQUE
; RUN: opt -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-local-buffers -S %s | FileCheck %s -check-prefix=NONOPAQUE
; RUN: opt -opaque-pointers -dpcpp-kernel-enable-tls-globals -dpcpp-kernel-local-buffers -S %s | FileCheck %s -check-prefix=OPAQUE

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

@main_kernel.local_arr = internal addrspace(3) global i32 undef, align 4
@pLocalMemBase = linkonce_odr thread_local global i8 addrspace(3)* undef, align 8

define void @main_kernel() {
entry:
; NONOPAQUE: %LocalMemBase = load i8 addrspace(3)*, i8 addrspace(3)** @pLocalMemBase, align 8
; NONOPAQUE: %0 = getelementptr i8, i8 addrspace(3)* %LocalMemBase, i32 0
; NONOPAQUE: %1 = addrspacecast i8 addrspace(3)* %0 to i32 addrspace(3)**
; NONOPAQUE: %2 = load i32 addrspace(3)*, i32 addrspace(3)** %1, align 8
; NONOPAQUE: store i32 1, i32 addrspace(3)* %2, align 4
; OPAQUE:    %LocalMemBase = load ptr addrspace(3), ptr @pLocalMemBase, align 8
; OPAQUE:    %0 = getelementptr i8, ptr addrspace(3) %LocalMemBase, i32 0
; OPAQUE:    %1 = addrspacecast ptr addrspace(3) %0 to ptr
; OPAQUE:    %2 = load ptr addrspace(3), ptr %1, align 8
; OPAQUE:    store i32 1, ptr addrspace(3) %2, align 4
  store i32 1, i32 addrspace(3)* @main_kernel.local_arr, align 4
  ret void
}

!opencl.kernels = !{!0}

!0 = !{void ()* @main_kernel}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
