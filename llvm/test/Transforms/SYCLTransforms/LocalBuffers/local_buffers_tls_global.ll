; RUN: opt -passes=sycl-kernel-local-buffers -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-local-buffers -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

@main_kernel.local_arr = internal addrspace(3) global i32 undef, align 4
@__pLocalMemBase = linkonce_odr thread_local global ptr addrspace(3) undef, align 8

define void @main_kernel() {
entry:
; CHECK:    %LocalMemBase = load ptr addrspace(3), ptr @__pLocalMemBase, align 8
; CHECK:    [[GEP:%[0-9]+]] = getelementptr i8, ptr addrspace(3) %LocalMemBase, i32 0
; CHECK:    store i32 1, ptr addrspace(3) [[GEP]], align 4
  store i32 1, ptr addrspace(3) @main_kernel.local_arr, align 4
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @main_kernel}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
