; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -analyze -kernel-analysis -cl-loop-bound -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: CLWGLoopBoundaries
; CHECK: found 2 early exit boundaries
;; boundary output pattern: dim, contains_val(T/F), is_global_id(T/F), isSigned(T/F), isUpperBound(T,F)
; CHECK: dim=0, contains=T, isGID=T, isSigned=T, isUpper=F
; CHECK-NEXT: i32 1
; CHECK: dim=1, contains=T, isGID=T, isSigned=T, isUpper=F
; CHECK-NEXT: i32 1
; CHECK: found 0 uniform early exit conditions

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

declare i32 @get_global_id(i32) nounwind readnone


define void @cmp_select_max_kernel(<4 x i8> addrspace(1)* nocapture %dst, i32 %width, i32 %height) nounwind {
  %id0 = tail call i32 @get_global_id(i32 0) nounwind readnone
  %id1 = tail call i32 @get_global_id(i32 1) nounwind readnone
  %cmp0 = icmp sle i32 %id0, 1
  %select0 = select i1 %cmp0, i32 1, i32 %id0
  %cmp1 = icmp sge i32 %id1, 1
  %select1 = select i1 %cmp1, i32 %id1, i32 1
  %mul = mul nsw i32 %select1, %width
  %ind = add nsw i32 %mul, %select0
  %ptr = getelementptr inbounds <4 x i8> addrspace(1)* %dst, i32 %ind
  store <4 x i8> <i8 100, i8 100, i8 100, i8 100>, <4 x i8> addrspace(1)* %ptr, align 4
  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x i8> addrspace(1)*, i32, i32)* @cmp_select_max_kernel}
