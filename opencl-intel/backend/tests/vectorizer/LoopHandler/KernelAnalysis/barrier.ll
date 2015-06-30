; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -kernel-analysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; CHECK: KernelAnalysis
; CHECK: kernel_contains_barrier no
; CHECK: kernel_not_contains_barrier yes
; CHECK: kernel_call_func_call_barrier no
; CHECK: kernel_call_func_call_func_call_barrier no
; CHECK: kernel_call_func_no_call_barrier yes


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"


define void @func_no_call_barrier() nounwind {
  ret void
}

define void @func_call_barrier() nounwind {
  tail call void @_Z7barrierj(i32 0)
  ret void
}

define void @func_call_func_call_barrier() nounwind {
  tail call void @func_call_barrier()
  ret void
}

define void @kernel_contains_barrier() nounwind {
entry:
  tail call void @_Z7barrierj(i32 0)
  ret void
}


define void @kernel_not_contains_barrier() nounwind {
entry:
  ret void
}

define void @kernel_call_func_call_barrier() nounwind {
entry:
  tail call void @func_call_barrier()
  ret void
}

define void @kernel_call_func_call_func_call_barrier() nounwind {
entry:
  tail call void @func_call_func_call_barrier()
  ret void
}

define void @kernel_call_func_no_call_barrier() nounwind {
entry:
  tail call void @func_no_call_barrier()
  ret void
}





declare void @_Z7barrierj(i32)



!opencl.kernels = !{!0, !1, !2, !3, !4}


!0 = !{void ()* @kernel_contains_barrier}
!1 = !{void ()* @kernel_not_contains_barrier}
!2 = !{void ()* @kernel_call_func_call_barrier}
!3 = !{void ()* @kernel_call_func_call_func_call_barrier}
!4 = !{void ()* @kernel_call_func_no_call_barrier}

