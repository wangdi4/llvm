; RUN: opt -passes=dpcpp-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; This test checks that get_*_id are replaced in O0 mode in the case there is no
; kernels.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: define dso_local void @test(i32 addrspace(1)* noalias noundef %dst, [3 x i64]* noalias %local.ids)
; CHECK-NEXT: entry:
; CHECK-NEXT: %gid0.addr = alloca i64, align 8
; CHECK-NEXT: %dst.addr = alloca i32 addrspace(1)*, align 8
; CHECK-NEXT: %lid0.addr = alloca i64, align 8
; CHECK-NEXT: %ptr.lid0 = getelementptr inbounds [3 x i64], [3 x i64]* %local.ids, i64 0, i32 0
; CHECK-NEXT: %lid0 = load i64, i64* %ptr.lid0, align 8
; CHECK-NEXT: store i64 %lid0, i64* %lid0.addr, align 8
; CHECK-NEXT: %base.gid0 = call i64 @get_base_global_id.(i32 0)
; CHECK-NEXT: %gid0 = add i64 %lid0, %base.gid0
; CHECK-NEXT: store i64 %gid0, i64* %gid0.addr, align 8
; CHECK-NEXT: store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 8
; CHECK-NEXT: %lid0.ld = load i64, i64* %lid0.addr, align 8
; CHECK-NEXT: %conv = trunc i64 %lid0.ld to i32
; CHECK-NEXT: [[PTR:%[0-9]+]] = load i32 addrspace(1)*, i32 addrspace(1)** %dst.addr, align 8
; CHECK-NEXT: %gid0.ld = load i64, i64* %gid0.addr, align 8
; CHECK-NEXT: %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* [[PTR]], i64 %gid0.ld

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local void @test(i32 addrspace(1)* noalias noundef %dst) #0 {
entry:
  %dst.addr = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 8
  %call = call i64 @_Z12get_local_idj(i32 noundef 0) #2
  %conv = trunc i64 %call to i32
  %0 = load i32 addrspace(1)*, i32 addrspace(1)** %dst.addr, align 8
  %call1 = call i64 @_Z13get_global_idj(i32 noundef 0) #2
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %0, i64 %call1
  store i32 %conv, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z12get_local_idj(i32 noundef) #1

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32 noundef) #1

attributes #0 = { convergent noinline norecurse nounwind optnone "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent nounwind readnone willreturn }

!opencl.compiler.options = !{!0}
!sycl.kernels = !{!1}

!0 = !{!"-cl-std=CL2.0", !"-cl-opt-disable"}
!1 = !{}

; DEBUGIFY-NOT: WARNING
