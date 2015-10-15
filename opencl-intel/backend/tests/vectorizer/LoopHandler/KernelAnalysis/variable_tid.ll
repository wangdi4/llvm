; RUN: llvm-as %s -o %t.bc
; RUN: opt -analyze -kernel-analysis -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; CHECK: KernelAnalysis
; CHECK: variable_gid no
; CHECK: variable_lid no
; CHECK: out_of_range_gid no
; CHECK: out_of_range_lid no
; CHECK: in_range_gid yes
; CHECK: in_range_lid yes


; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @variable_gid(i32 addrspace(1)* %out, i32 %dim) nounwind alwaysinline {
  %id = call i32 @_Z13get_global_idj(i32 %dim) nounwind
  %outptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @variable_lid(i32 addrspace(1)* %out, i32 %dim) nounwind alwaysinline {
  %id = call i32 @_Z12get_local_idj(i32 %dim) nounwind
  %outptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @out_of_range_gid(i32 addrspace(1)* %out) nounwind alwaysinline {
  %id = call i32 @_Z13get_global_idj(i32 10) nounwind
  %outptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @out_of_range_lid(i32 addrspace(1)* %out) nounwind alwaysinline {
  %id = call i32 @_Z12get_local_idj(i32 10) nounwind
  %outptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @in_range_gid(i32 addrspace(1)* %out) nounwind alwaysinline {
  %id = call i32 @_Z13get_global_idj(i32 1) nounwind
  %outptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @in_range_lid(i32 addrspace(1)* %out) nounwind alwaysinline {
  %id = call i32 @_Z12get_local_idj(i32 1) nounwind
  %outptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

declare i32 @_Z13get_global_idj(i32) nounwind readnone

declare i32 @_Z12get_local_idj(i32) nounwind readnone



!opencl.kernels = !{!0, !1, !2, !3, !4, !5}

!0 = !{void (i32 addrspace(1)* , i32 )* @variable_gid}
!1 = !{void (i32 addrspace(1)* , i32 )* @variable_lid}
!2 = !{void (i32 addrspace(1)* )* @out_of_range_gid}
!3 = !{void (i32 addrspace(1)* )* @out_of_range_lid}
!4 = !{void (i32 addrspace(1)* )* @in_range_gid}
!5 = !{void (i32 addrspace(1)* )* @in_range_lid}
