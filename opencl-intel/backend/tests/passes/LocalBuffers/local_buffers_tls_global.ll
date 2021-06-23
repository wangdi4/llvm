; RUN: %oclopt -debugify -use-tls-globals -local-buffers -check-debugify -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -use-tls-globals -local-buffers -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

@pLocalMemBase = linkonce_odr thread_local global i8 addrspace(3)* undef, align 8
define void @main_kernel() {
entry:
; CHECK: %LocalMemBase = load i8 addrspace(3)*, i8 addrspace(3)** @pLocalMemBase, align 8
  ret void
}

!opencl.kernels = !{!0}

!0 = !{void ()* @main_kernel}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function main_kernel --  %LocalMemBase = load i8 addrspace(3)*, i8 addrspace(3)** @pLocalMemBase, align 8
; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
