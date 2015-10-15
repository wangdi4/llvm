; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -analyze -kernel-analysis -cl-loop-bound -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: CLWGLoopBoundaries
; CHECK: found 4 early exit boundaries
;; boundary output pattern: dim, contains_val(T/F), is_global_id(T/F), isSigned(T/F), isUpperBound(T,F)
; CHECK: dim=0, contains=T, isGID=T, isSigned=T, isUpper=T
; CHECK-NEXT: %t1
; CHECK: dim=0, contains=T, isGID=T, isSigned=T, isUpper=F
; CHECK-NEXT: %t2
; CHECK: dim=0, contains=F, isGID=T, isSigned=T, isUpper=F
; CHECK-NEXT: %t3
; CHECK: dim=0, contains=F, isGID=T, isSigned=T, isUpper=T
; CHECK-NEXT: %t4

; CHECK: found 0 uniform early exit conditions



; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @program(i32 addrspace(1)* %out, i32 %t1, i32 %t2, i32 %t3, i32 %t4) nounwind {
entry:
  %id = call i32 @_Z13get_global_idj(i32 0) nounwind
  %b1 = icmp sle i32 %id, %t1
  br i1 %b1, label %c2, label %ret
 
c2:
  %b2 = icmp sle i32 %t2, %id
  br i1 %b2, label %c3, label %ret

c3:
  %b3 = icmp sle i32 %id, %t3
  br i1 %b3, label %ret, label %c4
  
c4:
  %b4 = icmp sle i32 %t4, %id
  br i1 %b4, label %ret, label %body
  
body:
  %outptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  br label %ret
  
ret:
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!opencl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32, i32, i32, i32)* @program}

