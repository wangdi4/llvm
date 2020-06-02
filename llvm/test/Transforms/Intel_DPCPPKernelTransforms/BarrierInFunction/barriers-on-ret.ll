; RUN: opt -dpcpp-kernel-barrier-in-function %s -S -o - | FileCheck %s

;;*****************************************************************************
;; This test checks the BarrierInFunction pass inserts barriers only
;; in reachable basic blocks.
;;*****************************************************************************

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK-LABEL: define void @foo
define void @foo(i32 %x) #0 {
entry:
  %y = xor i32 %x, %x
  call void @__builtin_dpcpp_kernel_barrier(i32 2)
  br label %reachable.exit

reachable.exit:                                   ; preds = entry
  ret void

unreachable.exit:                                 ; No predecessors!
  ret void
; CHECK:      reachable.exit:
; CHECK-NEXT: @__builtin_dpcpp_kernel_barrier(i32 1)
; CHECK:      unreachable.exit:
; CHECK-NOT:  @__builtin_dpcpp_kernel_barrier(i32 1)
; CHECK-NEXT: ret void
}

declare void @__builtin_dpcpp_kernel_barrier(i32)

attributes #1 = { "dpcpp-no-barrier-path"="false" }
