; RUN: opt -dpcpp-kernel-set-vf -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=SSE42 -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes="dpcpp-kernel-set-vf,dpcpp-kernel-vec-clone" -dpcpp-vector-variant-isa-encoding-override=SSE42 -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-set-vf -dpcpp-kernel-vec-clone -dpcpp-vector-variant-isa-encoding-override=SSE42 -S %s | FileCheck %s
; RUN: opt -passes="dpcpp-kernel-set-vf,dpcpp-kernel-vec-clone" -dpcpp-vector-variant-isa-encoding-override=SSE42 -S %s | FileCheck %s

target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:32-n8:16:32-a:0:32-S32"
target triple = "i686-pc-win32-msvc-elf"

define void @foo(i32 addrspace(1)* %a) local_unnamed_addr !recommended_vector_length !1 {
entry:
  %lid = call i32 @_Z12get_local_idj(i32 0)
  %addr = getelementptr i32, i32 addrspace(1)* %a, i32 %lid
  store i32 %lid, i32 addrspace(1)* %addr
  ret void
}

; CHECK: define void @_ZGVbN4u_foo
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.UNIFORM:TYPED"(i32 addrspace(1)** %alloca.a, i32 addrspace(1)* null, i32 1) ]

declare i32 @_Z12get_local_idj(i32) local_unnamed_addr

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @foo}
!1 = !{i32 4}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_foo {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_foo {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_foo {{.*}} add
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_foo {{.*}} icmp
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_foo {{.*}} br
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_foo {{.*}} call
; DEBUGIFY-NEXT: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_foo {{.*}} br
; DEBUGIFY-NOT: WARNING
