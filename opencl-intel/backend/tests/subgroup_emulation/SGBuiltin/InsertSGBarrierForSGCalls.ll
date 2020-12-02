; RUN: %oclopt -sg-builtin -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(i32 %a) !kernel_has_sub_groups !1 !sg_emu_size !2 {
entry:
; CHECK: call void @_Z17sub_group_barrierj(i32 1)
  %call = tail call i32 @_Z13sub_group_alli(i32 %a)
; CHECK-NEXT: tail call i32 @_Z13sub_group_alli(i32 %a) #[[#CALL_ATTR:]]
; CHECK-NEXT: call void @dummy_sg_barrier()
  ret void
}

declare i32 @_Z13sub_group_alli(i32)
; CHECK: declare i32 @_Z13sub_group_alli(i32) #[[#FUNC_ATTR:]]

; CHECK-DAG: attributes #[[#FUNC_ATTR]] = { "vector-variants"="_ZGVbM16v__Z13sub_group_alli(_Z13sub_group_allDv16_iDv16_j)" }
; CHECK-DAG: attributes #[[#CALL_ATTR]] = { "has-vplan-mask" }

!opencl.kernels = !{!0}

!0 = !{void (i32)* @test}
!1 = !{i1 true}
!2 = !{i32 16}