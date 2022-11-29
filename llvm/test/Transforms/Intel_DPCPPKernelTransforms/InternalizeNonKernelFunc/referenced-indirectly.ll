; RUN: opt -passes=dpcpp-kernel-internalize-func -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-internalize-func -S %s | FileCheck %s
;
; Generated from:
; __attribute__((referenced_indirectly));
; int foo(int arg) {
;   return arg + 10;
; }
;
; void __kernel test(__global int *data, int input) {
;   int (__constant *fp)(int) = &foo;
;
;   *data = fp(input);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: define spir_func i32 @foo

define spir_func i32 @foo(i32 %arg) #0 {
entry:
  %arg.addr = alloca i32, align 4
  store i32 %arg, i32* %arg.addr, align 4
  %0 = load i32, i32* %arg.addr, align 4
  %add = add nsw i32 %0, 10
  ret i32 %add
}

define spir_kernel void @test(i32 addrspace(1)* %data, i32 %input) #1 !kernel_arg_addr_space !1 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 {
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

attributes #0 = { convergent noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" "referenced-indirectly" }
attributes #1 = { convergent noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent }

!llvm.module.flags = !{!0}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"clang version 7.0.0 "}
!5 = !{!"none", !"none"}
!6 = !{!"int*", !"int"}
!7 = !{!"", !""}

; DEBUGIFY-NOT: WARNING
