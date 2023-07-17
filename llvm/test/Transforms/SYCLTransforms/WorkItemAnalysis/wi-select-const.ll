; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare i32 @_Z13get_global_idj(i32)

; CHECK: WorkItemAnalysis for function constant_select_true:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: SEQ   %select_val = select i1 true, i32 %gid, i32 %uni
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid

define void @constant_select_true(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out, i32 %uni) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %select_val = select i1 true, i32 %gid, i32 %uni
  %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid
  store i32 %select_val, ptr addrspace(1) %ptr, align 4
  ret void
}

; CHECK: WorkItemAnalysis for function constant_select_false:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: UNI   %select_val = select i1 false, i32 %gid, i32 %uni
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid

define void @constant_select_false(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out, i32 %uni) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %select_val = select i1 false, i32 %gid, i32 %uni
  %ptr = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %gid
  store i32 %select_val, ptr addrspace(1) %ptr, align 4
  ret void
}

!0 = !{!"int*", !"int*", !"int"}
!1 = !{ptr addrspace(1) null, ptr addrspace(1) null, i32 0}
