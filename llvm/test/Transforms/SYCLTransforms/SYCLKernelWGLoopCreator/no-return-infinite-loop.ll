; Check dummpy return block is added if kernel has no return instruction but
; infinite loop.

; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S | FileCheck %s
; RUN: opt -passes=sycl-kernel-wgloop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: dummy_ret:

define dso_local void @test() #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_name !1 !kernel_arg_host_accessible !1 !kernel_arg_pipe_depth !1 !kernel_arg_pipe_io !1 !kernel_arg_buffer_location !1 !no_barrier_path !2 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  br label %for.cond, !llvm.loop !3
}

attributes #0 = { convergent norecurse nounwind }

!sycl.kernels = !{!0}

!0 = !{ptr @test}
!1 = !{}
!2 = !{i1 true}
!3 = distinct !{!3, !4, !5}
!4 = !{!"llvm.loop.mustprogress"}
!5 = !{!"llvm.loop.unroll.disable"}

; DEBUGIFY-COUNT-32: WARNING: Instruction with empty DebugLoc in function test
; DEBUGIFY-NOT: WARNING:
