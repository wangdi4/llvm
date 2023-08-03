; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare i32 @_Z13get_global_idj(i32)

; CHECK: WorkItemAnalysis for function extract_elt_uni:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: UNI   %extract_elt_val = extractelement <4 x i32> %uni, i32 0
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid

define void @extract_elt_uni(ptr addrspace(1) nocapture %out, <4 x i32> %uni) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %extract_elt_val = extractelement <4 x i32> %uni, i32 0
  %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid
  store i32 %extract_elt_val, ptr addrspace(1) %ptr, align 4
  ret void
}

; CHECK: WorkItemAnalysis for function extract_elt_rnd:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: RND   %extract_elt_val = extractelement <4 x i32> %uni, i32 %gid
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid

define void @extract_elt_rnd(ptr addrspace(1) nocapture %out, <4 x i32> %uni) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %extract_elt_val = extractelement <4 x i32> %uni, i32 %gid
  %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid
  store i32 %extract_elt_val, ptr addrspace(1) %ptr, align 4
  ret void
}

; CHECK: WorkItemAnalysis for function insert_elt_uni:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: UNI   %insert_elt_val = insertelement <4 x i32> %uni, i32 4, i32 0
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds <4 x i32>, ptr addrspace(1) %out, i32 %gid

define void @insert_elt_uni(ptr addrspace(1) nocapture %out, <4 x i32> %uni) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %insert_elt_val = insertelement <4 x i32> %uni, i32 4, i32 0
  %ptr = getelementptr inbounds <4 x i32>, ptr addrspace(1) %out, i32 %gid
  store <4 x i32> %insert_elt_val, ptr addrspace(1) %ptr, align 4
  ret void
}

; CHECK: WorkItemAnalysis for function insert_elt_rnd:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: RND   %insert_elt_val = insertelement <4 x i32> %uni, i32 4, i32 %gid
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds <4 x i32>, ptr addrspace(1) %out, i32 %gid

define void @insert_elt_rnd(ptr addrspace(1) nocapture %out, <4 x i32> %uni) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %insert_elt_val = insertelement <4 x i32> %uni, i32 4, i32 %gid
  %ptr = getelementptr inbounds <4 x i32>, ptr addrspace(1) %out, i32 %gid
  store <4 x i32> %insert_elt_val, ptr addrspace(1) %ptr, align 4
  ret void
}

!0 = !{!"int*", !"int4"}
!1 = !{ptr addrspace(1) null, <4 x i32> zeroinitializer}
!2 = !{!"int4*", !"int4"}
!3 = !{ptr addrspace(1) null, <4 x i32> zeroinitializer}
