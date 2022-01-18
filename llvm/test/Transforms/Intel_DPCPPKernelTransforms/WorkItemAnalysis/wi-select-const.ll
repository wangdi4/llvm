; RUN: opt -passes='require<dpcpp-kernel-builtin-info-analysis>,print<dpcpp-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s
; RUN: opt -analyze -dpcpp-kernel-work-item-analysis %s -S -o - | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare i32 @_Z13get_global_idj(i32)

; CHECK: WorkItemAnalysis for function constant_select_true:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: SEQ   %select_val = select i1 true, i32 %gid, i32 %uni
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %gid

define void @constant_select_true(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out, i32 %uni) nounwind {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %select_val = select i1 true, i32 %gid, i32 %uni
  %ptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %gid
  store i32 %select_val, i32 addrspace(1)* %ptr, align 4
  ret void
}

; CHECK: WorkItemAnalysis for function constant_select_false:
; CHECK-NEXT: SEQ   %gid = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: UNI   %select_val = select i1 false, i32 %gid, i32 %uni
; CHECK-NEXT: PTR   %ptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %gid

define void @constant_select_false(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out, i32 %uni) nounwind {
  %gid = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %select_val = select i1 false, i32 %gid, i32 %uni
  %ptr = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %gid
  store i32 %select_val, i32 addrspace(1)* %ptr, align 4
  ret void
}
