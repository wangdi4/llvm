; RUN: opt -passes=sycl-kernel-group-builtin -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-group-builtin -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the GroupBuiltins pass
;; The case: kernel "main" with no async_copy instruction
;; The expected result:
;;      1. nothing changed, kernel ends up the same as it started
;;*****************************************************************************

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: @main
define void @main(i32 %x) nounwind {
  %y = xor i32 %x, %x
  ret void
; CHECK: %y = xor i32 %x, %x
; CHECK: ret void
}

!sycl.kernels = !{!0}
!opencl.build.options = !{}
!opencl.kernel_info = !{!6}
!opencl.disabled.FP_CONTRACT = !{}

!0 = !{void (i32)* @main, !1, !1, !"", !"int", !"opencl_main_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 0}
!3 = !{i32 3}
!4 = !{!"int"}
!5 = !{!"x"}
!6 = !{void (i32)* @main, !7}
!7 = !{!8, !9, !10, !11, !12, !13, !14, !15, !16}
!8 = !{!"local_buffer_size", null}
!9 = !{!"barrier_buffer_size", null}
!10 = !{!"kernel_execution_length", null}
!11 = !{!"kernel_has_barrier", null}
!12 = !{!"no_barrier_path", i1 false}
!13 = !{!"vectorized_kernel", null}
!14 = !{!"vectorized_width", null}
!15 = !{!"kernel_wrapper", null}
!16 = !{!"scalar_kernel", null}

; DEBUGIFY-NOT: WARNING
