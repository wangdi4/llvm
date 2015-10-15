; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -kernel-analysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; CHECK: KernelAnalysis
; CHECK: test_kernel_to_call no
; CHECK: test_kernel_call_kernel no
; CHECK: test_kernel_call_function yes


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @test_kernel_to_call() nounwind {
entry:
  ret void
}

declare i32 @_Z13get_global_idj(i32) nounwind readnone

define void @test_kernel_call_kernel() nounwind {
entry:
  tail call void @test_kernel_to_call()
  ret void
}

define void @test_kernel_call_function() nounwind {
entry:
  tail call void @test_function_to_call()
  ret void
}

define void @test_function_to_call() nounwind {
  ret void
}


!opencl.kernels = !{!0, !1, !2}


!0 = !{void ()* @test_kernel_to_call}
!1 = !{void ()* @test_kernel_call_kernel}
!2 = !{void ()* @test_kernel_call_function}
