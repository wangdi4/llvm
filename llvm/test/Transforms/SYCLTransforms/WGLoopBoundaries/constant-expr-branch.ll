; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-wg-loop-bound %s -S | FileCheck %s

@.glb = external unnamed_addr addrspace(1) constant [32 x i8]

define void @func(ptr addrspace(1) noalias %_arg_) !no_barrier_path !1 {
entry:
  br i1 icmp ne (ptr addrspace(4) addrspacecast (ptr addrspace(1) @.glb to ptr addrspace(4)), ptr addrspace(4) null), label %t, label %f
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
