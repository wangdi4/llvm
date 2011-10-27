; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -runtime=ocl -print-wia-check -WIAnalysis %t.bc -S -o %t1.ll  | FileCheck %s


; ModuleID = 'WItest'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare i32 @get_global_id(i32)

;CHECK: and_cons_uni
;CHECK: WI-RunOnFunction 4 %and_val
;CHECK: ret void
define void @and_cons_uni(i32 addrspace(1)* nocapture %out, i32 %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %and_val = and i32 %gid, %uni
  %ptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %gid
  store i32 %and_val, i32 addrspace(1)* %ptr, align 4
  ret void
}


;CHECK: and_uni_const_right_16bits
;CHECK: WI-RunOnFunction 1 %and_val
;CHECK: ret void
define void @and_uni_const_right_16bits(i32 addrspace(1)* nocapture %out, i32 %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %and_val = and i32 %gid, 65535
  %ptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %gid
  store i32 %and_val, i32 addrspace(1)* %ptr, align 4
  ret void
}


;CHECK: and_uni_const_left_16bits
;CHECK: WI-RunOnFunction 1 %and_val
;CHECK: ret void
define void @and_uni_const_left_16bits(i32 addrspace(1)* nocapture %out, i32 %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %and_val = and i32 65535, %gid
  %ptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %gid
  store i32 %and_val, i32 addrspace(1)* %ptr, align 4
  ret void
}


;CHECK: and_uni_const_right_15bits
;CHECK: WI-RunOnFunction 4 %and_val
;CHECK: ret void
define void @and_uni_const_right_15bits(i32 addrspace(1)* nocapture %out, i32 %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %and_val = and i32 %gid, 32767
  %ptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %gid
  store i32 %and_val, i32 addrspace(1)* %ptr, align 4
  ret void
}


;CHECK: and_uni_const_left_15bits
;CHECK: WI-RunOnFunction 4 %and_val
;CHECK: ret void
define void @and_uni_const_left_15bits(i32 addrspace(1)* nocapture %out, i32 %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %and_val = and i32 32767, %gid
  %ptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %gid
  store i32 %and_val, i32 addrspace(1)* %ptr, align 4
  ret void
}