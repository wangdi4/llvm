; RUN: opt --dpcpp-kernel-vec-clone -S %s | FileCheck %s

target datalayout = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:32-n8:16:32-a:0:32-S32"
target triple = "i686-pc-win32-msvc-elf"

define void @foo(i32 addrspace(1)* %a) local_unnamed_addr #0 {
entry:
  %lid = call i32 @__builtin_get_local_id(i32 0)
  %addr = getelementptr i32, i32 addrspace(1)* %a, i32 %lid
  store i32 %lid, i32 addrspace(1)* %addr
  ret void
}

; CHECK: define void @_ZGVdN8u_foo
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(i32 addrspace(1)** %alloca.a) ]

declare i32 @__builtin_get_local_id(i32) local_unnamed_addr

!opencl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*)* @foo}

attributes #0 = { "sycl_kernel" "target-cpu"="skylake-avx512" }
