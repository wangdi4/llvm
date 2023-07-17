; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Check that wg loop isn't created if max_wg_dimensions is 0.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent norecurse nounwind
define dso_local void @test() local_unnamed_addr #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_name !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 !no_barrier_path !3 !kernel_has_sub_groups !4 !max_wg_dimensions !5 !vectorized_width !6 !kernel_execution_length !6 !kernel_has_global_sync !4 {
entry:
; CHECK-LABEL: @test(
; CHECK-NEXT: entry:
; CHECK-NEXT:   br label %scalar_kernel_entry
; CHECK:      scalar_kernel_entry:
; CHECK-NEXT:   br label %exit
; CHECK:      exit:
; CHECK-NEXT:   ret void

  ret void
}

attributes #0 = { convergent norecurse nounwind }

!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!sycl.kernels = !{!1}

!0 = !{i32 2, i32 0}
!1 = !{ptr @test}
!2 = !{}
!3 = !{i1 true}
!4 = !{i1 false}
!5 = !{i32 0}
!6 = !{i32 1}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  br label %scalar_kernel_entry
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test --  br label %exit
; DEBUGIFY-NOT: WARNING
