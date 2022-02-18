; RUN: opt -passes=dpcpp-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s
; RUN: opt -dpcpp-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: DPCPPKernelAnalysisPass
; CHECK: Kernel <kernel_call_kernel>: NoBarrierPath=0
; CHECK: Kernel <kernel_to_call>: NoBarrierPath=0
; CHECK: Kernel <kernel_call_function>: NoBarrierPath=1

define void @kernel_to_call() {
  ret void
}

define void @kernel_call_kernel() {
  tail call void @kernel_to_call()
  ret void
}

define void @kernel_call_function() {
entry:
  tail call void @foo()
  ret void
}

define void @foo() {
  ret void
}

!sycl.kernels = !{!0}
!0 = !{void ()* @kernel_call_kernel, void ()* @kernel_to_call, void ()* @kernel_call_function}

; DEBUGIFY-NOT: WARNING
