; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -analyze -kernel-analysis -cl-loop-bound -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; CHECK: CLWGLoopBoundaries
; CHECK: found 0 early exit boundaries
; CHECK: found 0 uniform early exit conditions

; ModuleID = '-'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"


define void @program(i32 addrspace(1)* %out, <4 x i32> %dim, i32 %uni) nounwind alwaysinline {
  %id = call i32 @_Z13get_global_idj(i32 0) nounwind
  %thr = extractelement <4 x i32> %dim, i32 0
  %bound_bool = icmp slt i32 %id, %thr
  %uni_bool = icmp sgt i32 %uni, 0 
  %and_cond = and i1 %bound_bool, %uni_bool
  br i1 %and_cond, label %ret, label %body
  
body:
  %outptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  br label %ret
  
ret:
  ret void
}

declare i32 @_Z13get_global_idj(i32)

declare void @_Z7barrierm(i32)

!opencl.kernels = !{!0}
!0 = !{void (i32 addrspace(1)* , <4 x i32>, i32)* @program}
