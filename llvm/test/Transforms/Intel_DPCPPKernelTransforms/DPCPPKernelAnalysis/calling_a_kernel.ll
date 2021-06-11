; RUN: opt -analyze -dpcpp-kernel-analysis < %s -S -o - | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: DPCPPKernelAnalysisPass
; CHECK: kernel no
; CHECK: func no

define void @kernel() #0 {
  ret void
}

define void @func() #0 {
  tail call void @kernel()
  ret void
}

attributes #0 = { "sycl-kernel" }

!sycl.kernels = !{!0}
!0 = !{void ()* @kernel, void ()* @func}
