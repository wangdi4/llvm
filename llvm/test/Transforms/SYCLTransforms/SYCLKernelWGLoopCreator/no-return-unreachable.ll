; Check dummpy return block is added if kernel has no return instruction but
; unreachable.

; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: scalar_kernel_entry:
; CHECK: unreachable
; CHECK: dummy_ret:

; Function Attrs: nofree norecurse noreturn nosync nounwind readnone
define dso_local void @test() local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_name !1 !kernel_arg_host_accessible !1 !kernel_arg_pipe_depth !1 !kernel_arg_pipe_io !1 !kernel_arg_buffer_location !1 !no_barrier_path !2 !vectorized_kernel !3 !vectorized_width !4 !recommended_vector_length !5 {
entry:
  unreachable
}

; Function Attrs: nofree norecurse noreturn nosync nounwind readnone
define dso_local void @_ZGVeN16_test() local_unnamed_addr #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_name !1 !kernel_arg_host_accessible !1 !kernel_arg_pipe_depth !1 !kernel_arg_pipe_io !1 !kernel_arg_buffer_location !1 !no_barrier_path !2 !vectorized_width !6 !recommended_vector_length !6 !scalar_kernel !0 !vectorization_dimension !5 !can_unite_workgroups !2 {
entry:
  unreachable
}

attributes #0 = { nofree norecurse noreturn nosync nounwind readnone "vector-variants"="_ZGVeN16_test" }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{}
!2 = !{i1 true}
!3 = !{ptr @_ZGVeN16_test}
!4 = !{i32 1}
!5 = !{i32 0}
!6 = !{i32 16}

; DEBUGIFY-COUNT-62: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY-NOT: WARNING:
