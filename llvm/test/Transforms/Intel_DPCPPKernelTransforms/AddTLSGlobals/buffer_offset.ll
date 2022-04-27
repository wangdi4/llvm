; This test checks that the pLocalMemBase global variable is modified before and after
; each function call.

; RUN: opt -dpcpp-kernel-add-tls-globals %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-tls-globals %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-add-tls-globals %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-tls-globals %s -S | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@test.lint = internal addrspace(3) global i32 undef, align 4
; CHECK: @pLocalMemBase = linkonce_odr thread_local global i8 addrspace(3)* undef

; Function Attrs: convergent noinline nounwind
define spir_func void @foo() #0 {
entry:
  store i32 1234, i32 addrspace(3)* @test.lint
  ret void
}

; Function Attrs: convergent noinline nounwind
define spir_kernel void @test() #1 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !kernel_arg_name !2 {
entry:
; CHECK:  %0 = load i8 addrspace(3)*, i8 addrspace(3)** @pLocalMemBase
; CHECK-NEXT:  %pLocalMem_foo = getelementptr i8, i8 addrspace(3)* %0, i32 0
; CHECK-NEXT:  %1 = getelementptr i8, i8 addrspace(3)* %pLocalMem_foo, i32 0
; CHECK-NEXT:  %2 = addrspacecast i8 addrspace(3)* %1 to i32 addrspace(3)**
; CHECK-NEXT:  store i32 addrspace(3)* @test.lint, i32 addrspace(3)** %2
; CHECK-NEXT: store i8 addrspace(3)* %pLocalMem_foo, i8 addrspace(3)** @pLocalMemBase
; CHECK-NEXT: call spir_func void @foo()
; CHECK-NEXT: store i8 addrspace(3)* %0, i8 addrspace(3)** @pLocalMemBase
  call spir_func void @foo() #2
  ret void
}

attributes #0 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"-cl-opt-disable"}
!4 = !{!"icx (ICX) dev.8.x.0"}

; DEBUGIFY-NOT: WARNING
