; RUN: opt -passes='dpcpp-kernel-add-implicit-args,debugify,dpcpp-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='dpcpp-kernel-add-implicit-args,dpcpp-kernel-resolve-wi-call' -S %s -o %t.1.ll
; RUN: opt -passes=dpcpp-kernel-add-implicit-args -S %s -o %t.2.ll
; RUN: diff %t.1.ll %t.2.ll

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent noinline nounwind optnone
define spir_func i32 @foo(i32 %arg) {
entry:
  %arg.addr = alloca i32, align 4
  store i32 %arg, i32* %arg.addr, align 4
  %0 = load i32, i32* %arg.addr, align 4
  %add = add nsw i32 %0, 10
  ret i32 %add
}

; Function Attrs: convergent noinline nounwind optnone
define spir_kernel void @test(i32 addrspace(1)* %data, i32 %input) {
entry:
  %data.addr = alloca i32 addrspace(1)*, align 8
  %input.addr = alloca i32, align 4
  %fp = alloca i32 (i32)*, align 8
  store i32 addrspace(1)* %data, i32 addrspace(1)** %data.addr, align 8
  store i32 %input, i32* %input.addr, align 4
  store i32 (i32)* @foo, i32 (i32)** %fp, align 8
  %0 = load i32 (i32)*, i32 (i32)** %fp, align 8
  %1 = load i32, i32* %input.addr, align 4
  %call = call spir_func i32 %0(i32 %1) #2
  %2 = load i32 addrspace(1)*, i32 addrspace(1)** %data.addr, align 8
  store i32 %call, i32 addrspace(1)* %2, align 4
  ret void
}

; DEBUGIFY-NOT: WARNING

!sycl.kernels = !{!0}
!0 = !{void (i32 addrspace(1)*, i32)* @test}
