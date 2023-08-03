; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-resolve-wi-call,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-resolve-wi-call' -S %s -o %t.1.ll
; RUN: opt -passes=sycl-kernel-add-implicit-args -S %s -o %t.2.ll
; RUN: diff %t.1.ll %t.2.ll

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent noinline nounwind optnone
define spir_func i32 @foo(i32 %arg) {
entry:
  %arg.addr = alloca i32, align 4
  store i32 %arg, ptr %arg.addr, align 4
  %0 = load i32, ptr %arg.addr, align 4
  %add = add nsw i32 %0, 10
  ret i32 %add
}

; Function Attrs: convergent noinline nounwind optnone
define spir_kernel void @test(ptr addrspace(1) %data, i32 %input) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %data.addr = alloca ptr addrspace(1), align 8
  %input.addr = alloca i32, align 4
  %fp = alloca ptr, align 8
  store ptr addrspace(1) %data, ptr %data.addr, align 8
  store i32 %input, ptr %input.addr, align 4
  store ptr @foo, ptr %fp, align 8
  %0 = load ptr, ptr %fp, align 8
  %1 = load i32, ptr %input.addr, align 4
  %call = call spir_func i32 %0(i32 %1) #2
  %2 = load ptr addrspace(1), ptr %data.addr, align 8
  store i32 %call, ptr addrspace(1) %2, align 4
  ret void
}

; DEBUGIFY-NOT: WARNING

!sycl.kernels = !{!0}
!0 = !{ptr @test}
!1 = !{!"int*", !"int"}
!2 = !{ptr addrspace(1) null, i32 0}
