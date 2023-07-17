; RUN: opt -passes=sycl-kernel-add-function-attrs -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-add-function-attrs -S < %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: call i32 (ptr addrspace(2), ...) @printf({{.*}} #[[ATTR:[0-9]+]]
; CHECK: attributes #[[ATTR]] = { convergent nobuiltin }

@.str = private unnamed_addr addrspace(2) constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: convergent norecurse nounwind
define dso_local void @test() #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_name !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
  %call = call i32 (ptr addrspace(2), ...) @printf(ptr addrspace(2) noundef @.str, i32 noundef 15) #2
  ret void
}

; Function Attrs: convergent
declare i32 @printf(ptr addrspace(2) noundef, ...) #1

attributes #0 = { convergent norecurse nounwind "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" }
attributes #1 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #2 = { convergent }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.compiler.options = !{!1}
!sycl.kernels = !{!2}

!0 = !{i32 2, i32 0}
!1 = !{!"-cl-std=CL2.0"}
!2 = !{ptr @test}
!3 = !{}

; DEBUGIFY-NOT: WARNING
