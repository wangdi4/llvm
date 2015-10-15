; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -analyze -kernel-analysis -cl-loop-bound -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: CLWGLoopBoundaries
; CHECK: found 2 early exit boundaries
;; boundary output pattern: dim, contains_val(T/F), is_global_id(T/F), isSigned(T/F), isUpperBound(T,F)
; CHECK: dim=0, contains=T, isGID=T, isSigned=F, isUpper=T
; CHECK-NEXT: %add0
; CHECK: dim=1, contains=T, isGID=T, isSigned=F, isUpper=T
; CHECK-NEXT: %add1
; CHECK: found 0 uniform early exit conditions

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

declare i32 @_Z13get_global_idj(i32) nounwind readnone

define void @cmp_select_min_kernel(<4 x i8> addrspace(1)* nocapture %dst, i32 %width, i32 %height) nounwind {
  %id0 = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %id1 = tail call i32 @_Z13get_global_idj(i32 1) nounwind readnone
  %add0 = add nsw i32 %width, -1
  %cmp0 = icmp ugt i32 %id0, %add0
  %select0 = select i1 %cmp0, i32 %add0, i32 %id0
  %add1 = add nsw i32 %height, -1
  %cmp1 = icmp ult i32 %id1, %add1
  %select1 = select i1 %cmp1, i32 %id1, i32 %add1
  %mul = mul nsw i32 %select1, %width
  %ind = add nsw i32 %mul, %select0
  %ptr = getelementptr inbounds <4 x i8> addrspace(1)* %dst, i32 %ind
  store <4 x i8> <i8 100, i8 100, i8 100, i8 100>, <4 x i8> addrspace(1)* %ptr, align 4
  ret void
}


!opencl.kernels = !{!0}

!0 = !{void (<4 x i8> addrspace(1)*, i32, i32)* @cmp_select_min_kernel}
