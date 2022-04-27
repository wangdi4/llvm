; RUN: opt -passes=dpcpp-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s
; RUN: opt -dpcpp-kernel-analysis %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-analysis %s -S -debug -disable-output 2>&1| FileCheck %s

; CHECK: DPCPPKernelAnalysisPass
; CHECK: Kernel <variable_gid>: NoBarrierPath=0
; CHECK: Kernel <variable_lid>: NoBarrierPath=0
; CHECK: Kernel <out_of_range_gid>: NoBarrierPath=0
; CHECK: Kernel <out_of_range_lid>: NoBarrierPath=0
; CHECK: Kernel <in_range_gid>: NoBarrierPath=1
; CHECK: Kernel <in_range_lid>: NoBarrierPath=1

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @variable_gid(i32 addrspace(1)* %out, i32 %dim) nounwind alwaysinline {
  %id = call i32 @_Z13get_global_idj(i32 %dim) nounwind
  %outptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @variable_lid(i32 addrspace(1)* %out, i32 %dim) nounwind alwaysinline {
  %id = call i32 @_Z12get_local_idj(i32 %dim) nounwind
  %outptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @out_of_range_gid(i32 addrspace(1)* %out) nounwind alwaysinline {
  %id = call i32 @_Z13get_global_idj(i32 10) nounwind
  %outptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @out_of_range_lid(i32 addrspace(1)* %out) nounwind alwaysinline {
  %id = call i32 @_Z12get_local_idj(i32 10) nounwind
  %outptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @in_range_gid(i32 addrspace(1)* %out) nounwind alwaysinline {
  %id = call i32 @_Z13get_global_idj(i32 1) nounwind
  %outptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

define void @in_range_lid(i32 addrspace(1)* %out) nounwind alwaysinline {
  %id = call i32 @_Z12get_local_idj(i32 1) nounwind
  %outptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %id
  store i32 0, i32 addrspace(1)* %outptr
  ret void
}

declare i32 @_Z13get_global_idj(i32) nounwind readnone

declare i32 @_Z12get_local_idj(i32) nounwind readnone

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)* , i32 )* @variable_gid, void (i32 addrspace(1)* , i32 )* @variable_lid, void (i32 addrspace(1)* )* @out_of_range_gid, void (i32 addrspace(1)* )* @out_of_range_lid, void (i32 addrspace(1)* )* @in_range_gid, void (i32 addrspace(1)* )* @in_range_lid}

; DEBUGIFY-NOT: WARNING
