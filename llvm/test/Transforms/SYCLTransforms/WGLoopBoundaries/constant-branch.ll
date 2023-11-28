; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S | FileCheck %s

define void @func(ptr addrspace(1) noalias %_arg_) !no_barrier_path !1 {
entry:
  br i1 true, label %t, label %f
t:
  ret void
f:
  ret void
}

; We just make sure this pass doesn't crash.
; CHECK: define void @func

!sycl.kernels =  !{!0}
!0 = !{ ptr @func }
!1 = !{i1 true}

; DEBUGIFY-NOT: WARNING
