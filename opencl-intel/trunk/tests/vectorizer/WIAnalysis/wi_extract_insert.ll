; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -runtime=ocl -print-wia-check -WIAnalysis %t.bc -S -o %t1.ll  | FileCheck %s


; ModuleID = 'WItest'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare i32 @get_global_id(i32)


;CHECK: extract_elt_uni
;CHECK: WI-RunOnFunction 0 %extract_elt_val
;CHECK: ret void
define void @extract_elt_uni(i32 addrspace(1)* nocapture %out, <4 x i32> %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %extract_elt_val = extractelement <4 x i32> %uni, i32 0
  %ptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %gid
  store i32 %extract_elt_val, i32 addrspace(1)* %ptr, align 4
  ret void
}

;CHECK: extract_elt_rnd
;CHECK: WI-RunOnFunction 4 %extract_elt_val
;CHECK: ret void
define void @extract_elt_rnd(i32 addrspace(1)* nocapture %out, <4 x i32> %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %extract_elt_val = extractelement <4 x i32> %uni, i32 %gid
  %ptr = getelementptr inbounds i32 addrspace(1)* %out, i32 %gid
  store i32 %extract_elt_val, i32 addrspace(1)* %ptr, align 4
  ret void
}


;CHECK: insert_elt_uni
;CHECK: WI-RunOnFunction 0 %insert_elt_val
;CHECK: ret void
define void @insert_elt_uni(<4 x i32> addrspace(1)* nocapture %out, <4 x i32> %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %insert_elt_val = insertelement <4 x i32> %uni, i32 4, i32 0
  %ptr = getelementptr inbounds <4 x i32> addrspace(1)* %out, i32 %gid
  store <4 x i32> %insert_elt_val, <4 x i32> addrspace(1)* %ptr, align 4
  ret void
}

;CHECK: insert_elt_rnd
;CHECK: WI-RunOnFunction 4 %insert_elt_val
;CHECK: ret void
define void @insert_elt_rnd(<4 x i32> addrspace(1)* nocapture %out, <4 x i32> %uni) nounwind {
  %gid = tail call i32 @get_global_id(i32 0) nounwind
  %insert_elt_val = insertelement <4 x i32> %uni, i32 4, i32 %gid
  %ptr = getelementptr inbounds <4 x i32> addrspace(1)* %out, i32 %gid
  store <4 x i32> %insert_elt_val, <4 x i32> addrspace(1)* %ptr, align 4
  ret void
}
