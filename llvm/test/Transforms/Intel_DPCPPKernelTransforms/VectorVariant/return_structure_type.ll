; Test for making sure that we don't put vector variants to functions with
; structure return type. Structure return type is not supported yet.

; REQUIRES: asserts
; RUN: opt %s --debug-only=dpcpp-kernel-sg-size-collector -enable-debugify -dpcpp-kernel-update-call-attrs -dpcpp-kernel-vec-clone -dpcpp-kernel-vector-variant-lowering -dpcpp-kernel-sg-size-collector -dpcpp-kernel-sg-size-collector-indirect -dpcpp-kernel-vector-variant-fillin -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt %s --debug-only=dpcpp-kernel-sg-size-collector -enable-debugify -passes="dpcpp-kernel-update-call-attrs,dpcpp-kernel-vec-clone,dpcpp-kernel-vector-variant-lowering,dpcpp-kernel-sg-size-collector,dpcpp-kernel-sg-size-collector-indirect,dpcpp-kernel-vector-variant-fillin" -S 2>&1 | FileCheck %s -check-prefix=DEBUGIFY

; RUN: opt %s --debug-only=dpcpp-kernel-sg-size-collector -dpcpp-kernel-update-call-attrs -dpcpp-kernel-vec-clone -dpcpp-kernel-vector-variant-lowering -dpcpp-kernel-sg-size-collector -dpcpp-kernel-sg-size-collector-indirect -dpcpp-kernel-vector-variant-fillin -S 2>&1 | FileCheck %s
; RUN: opt %s --debug-only=dpcpp-kernel-sg-size-collector -passes="dpcpp-kernel-update-call-attrs,dpcpp-kernel-vec-clone,dpcpp-kernel-vector-variant-lowering,dpcpp-kernel-sg-size-collector,dpcpp-kernel-sg-size-collector-indirect,dpcpp-kernel-vector-variant-fillin" -S 2>&1 | FileCheck %s

define void @test(float %x, i32 addrspace(1)* noalias %a) !recommended_vector_length !13 {
entry:
  %conv = fpext float %x to double
  %call = call {double, double} @direct(double %conv)
  ret void
}

define {double, double} @direct(double %val) {
entry:
  %call.ii = tail call { double, double } @clog(double %val, double 0.000000e+00)
  ret { double, double } %call.ii
}

declare dso_local { double, double } @clog(double, double) local_unnamed_addr

; CHECK-NOT: attributes #[[ATTR0:.*]] = { "vector-variants"
; CHECK: Vectorization for function with struct return type is not supported

; DEBUGIFY-COUNT-7: WARNING: Instruction with empty DebugLoc
; DEBUGIFY-NOT: WARNING

!sycl.kernels = !{!4}

!4 = !{void (float, i32 addrspace(1)*)* @test}
!12 = !{i1 false}
!13 = !{i32 4}
