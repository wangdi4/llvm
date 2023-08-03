; RUN: opt -passes=sycl-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: SYCLKernelAnalysisPass
; CHECK: Kernel <kernel_call_kernel>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_to_call>:
; CHECK-NEXT: NoBarrierPath=0
; CHECK: Kernel <kernel_call_function>:
; CHECK-NEXT: NoBarrierPath=1

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
!0 = !{ptr @kernel_call_kernel, ptr @kernel_to_call, ptr @kernel_call_function}

; DEBUGIFY-NOT: WARNING
