; RUN: opt -passes=dpcpp-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-wg-loop-bound %s -S | FileCheck %s
; RUN: opt -dpcpp-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-wg-loop-bound %s -S | FileCheck %s

define void @func(i8 addrspace(1)* noalias %_arg_) !no_barrier_path !1 {
entry:
  br i1 true, label %t, label %f
t:
  ret void
f:
  ret void
}

; We just make sure this pass doesn't crash and generates the definition
; following function.
; CHECK: define [7 x i64] @WG.boundaries.func(i8 addrspace(1)* %0)

!sycl.kernels =  !{!0}
!0 = !{ void(i8 addrspace(1)*)* @func }
!1 = !{i1 true}

; DEBUGIFY-COUNT-14: Instruction with empty DebugLoc in function WG.boundaries.
; DEBUGIFY-NOT: WARNING
