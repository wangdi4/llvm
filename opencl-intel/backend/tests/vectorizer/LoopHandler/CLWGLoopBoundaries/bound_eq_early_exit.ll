; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -analyze -kernel-analysis -cl-loop-bound -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: CLWGLoopBoundaries
; CHECK: found 2 early exit boundaries
;; boundary output pattern: dim, contains_val(T/F), is_global_id(T/F), isSigned(T/F), isUpperBound(T,F)
; CHECK: dim=0, contains=T, isGID=T, isSigned=F, isUpper=T
; CHECK-NEXT: %t1
; CHECK: dim=0, contains=T, isGID=T, isSigned=F, isUpper=F
; CHECK-NEXT: %t1

; CHECK: found 0 uniform early exit conditions


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @program(i32 addrspace(1)* %out, i32 %t1) nounwind {
entry:
  %id = call i32 @_Z13get_global_idj(i32 0) nounwind
  %b1 = icmp eq i32 %id, %t1
  br i1 %b1, label %body, label %ret
 
body:
  %outptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  br label %ret
  
ret:
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!opencl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32)* @program}


