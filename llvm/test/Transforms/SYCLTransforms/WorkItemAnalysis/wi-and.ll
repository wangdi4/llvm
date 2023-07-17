; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare i32 @_Z13get_global_idj(i32)

; CHECK: WorkItemAnalysis for function and_cons_uni:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: RND   %and_val = and i32 %gid, %uni
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid

define void @and_cons_uni(ptr addrspace(1) nocapture %out, i32 %uni) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %and_val = and i32 %gid, %uni
  %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid
  store i32 %and_val, ptr addrspace(1) %ptr, align 4
  ret void
}

; CHECK: WorkItemAnalysis for function and_uni_const_right_16bits:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: SEQ   %and_val = and i32 %gid, 65535
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid

define void @and_uni_const_right_16bits(ptr addrspace(1) nocapture %out, i32 %uni) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %and_val = and i32 %gid, 65535
  %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid
  store i32 %and_val, ptr addrspace(1) %ptr, align 4
  ret void
}

; CHECK: WorkItemAnalysis for function and_uni_const_left_16bits:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: SEQ   %and_val = and i32 65535, %gid
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid

define void @and_uni_const_left_16bits(ptr addrspace(1) nocapture %out, i32 %uni) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %and_val = and i32 65535, %gid
  %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid
  store i32 %and_val, ptr addrspace(1) %ptr, align 4
  ret void
}

; CHECK: WorkItemAnalysis for function and_uni_const_right_15bits:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: RND   %and_val = and i32 %gid, 32767
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid

define void @and_uni_const_right_15bits(ptr addrspace(1) nocapture %out, i32 %uni) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %and_val = and i32 %gid, 32767
  %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid
  store i32 %and_val, ptr addrspace(1) %ptr, align 4
  ret void
}

; CHECK: WorkItemAnalysis for function and_uni_const_left_15bits:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: RND   %and_val = and i32 32767, %gid
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid

define void @and_uni_const_left_15bits(ptr addrspace(1) nocapture %out, i32 %uni) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %and_val = and i32 32767, %gid
  %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid
  store i32 %and_val, ptr addrspace(1) %ptr, align 4
  ret void
}

!0 = !{!"int*", !"int"}
!1 = !{ptr addrspace(1) null, i32 0}
