; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -runtime=ocl -print-wia-check -WIAnalysis %t.bc -S -o %t1.ll  | FileCheck %s


; ModuleID = 'WItest'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare i32 @get_global_id(i32)
declare i32 @get_local_size(i32)


;CHECK: get_local_size_const
;CHECK: WI-RunOnFunction 0 %size_val
;CHECK: ret void
define void @get_local_size_const(i32 addrspace(1)* nocapture %out, i32 %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %size_val = tail call i32 @get_local_size(i32 0) nounwind
  %ptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %gid
  store i32 %size_val, i32 addrspace(1)* %ptr, align 4
  ret void
}


;CHECK: get_local_size_uni
;CHECK: WI-RunOnFunction 0 %size_val
;CHECK: ret void
define void @get_local_size_uni(i32 addrspace(1)* nocapture %out, i32 %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %size_val = tail call i32 @get_local_size(i32 %uni) nounwind
  %ptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %gid
  store i32 %size_val, i32 addrspace(1)* %ptr, align 4
  ret void
}


;CHECK: get_local_size_rnd
;CHECK: WI-RunOnFunction 4 %size_val
;CHECK: ret void
define void @get_local_size_rnd(i32 addrspace(1)* nocapture %out, i32 %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %size_val = tail call i32 @get_local_size(i32 %gid) nounwind
  %ptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %gid
  store i32 %size_val, i32 addrspace(1)* %ptr, align 4
  ret void
}