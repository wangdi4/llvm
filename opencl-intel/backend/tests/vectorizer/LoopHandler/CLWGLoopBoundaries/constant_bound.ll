; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -kernel-analysis -cl-loop-bound -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @test
; CHECK: ret
; CHECK: @WG.boundaries.test
; CHECK: ret

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"




define void @test(float addrspace(1)* nocapture %paths) nounwind {
  %1 = call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = icmp sgt i32 %1, 131071
 br i1 %2, label %5, label %3

; <label>:3                                       ; preds = %0
  %4 = getelementptr inbounds float addrspace(1)* %paths, i32 %1
  store float 1.000000e+00, float addrspace(1)* %4, align 4
  br label %5

; <label>:5                                      ; preds = %0, %3
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!opencl.kernels = !{!0}
!opencl.compiler.options = !{!2}

!0 = !{void (float addrspace(1)*)* @test, !1}
!1 = !{!"image_access_qualifier", i32 3, i32 3}
!2 = !{}
