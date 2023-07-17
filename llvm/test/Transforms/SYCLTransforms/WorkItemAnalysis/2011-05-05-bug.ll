; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

declare float @_Z5acoshf(float)

declare i64 @_Z13get_global_idj(i32)

; make sure that the WorkItemAnalysis detects the ptr as consecutive.
; CHECK: WorkItemAnalysis for function __Vectorized_wlacosh:
; CHECK-NEXT: SEQ   %1 = tail call i64 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: SEQ   %2 = and i64 %1, 4294967295
; CHECK-NEXT: PTR   %3 = getelementptr inbounds float, ptr addrspace(1) %input, i64 %2
; CHECK-NEXT: RND   %4 = load float, ptr addrspace(1) %3, align 4
; CHECK-NEXT: RND   %5 = tail call float @_Z5acoshf(float %4) #0
; CHECK-NEXT: PTR   %6 = getelementptr inbounds float, ptr addrspace(1) %output, i64 %2
; CHECK-NEXT: RND   store float %5, ptr addrspace(1) %6, align 4
; CHECK-NEXT: UNI   ret void

define void @__Vectorized_wlacosh(ptr addrspace(1) nocapture %input, ptr addrspace(1) nocapture %output, i32 %buffer_size) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind
  %2 = and i64 %1, 4294967295
  %3 = getelementptr inbounds float, ptr addrspace(1) %input, i64 %2
  %4 = load float, ptr addrspace(1) %3, align 4
  %5 = tail call float @_Z5acoshf(float %4) nounwind
  %6 = getelementptr inbounds float, ptr addrspace(1) %output, i64 %2
  store float %5, ptr addrspace(1) %6, align 4
  ret void
}

; make sure that the WorkItemAnalysis does NOT detect the ptr as consecutive.
; CHECK: WorkItemAnalysis for function __Vectorized_wlacosh2:
; CHECK-NEXT: SEQ   %1 = tail call i64 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: RND   %2 = and i64 %1, 4294967000
; CHECK-NEXT: RND   %3 = getelementptr inbounds float, ptr addrspace(1) %input, i64 %2
; CHECK-NEXT: RND   %4 = load float, ptr addrspace(1) %3, align 4
; CHECK-NEXT: RND   %5 = tail call float @_Z5acoshf(float %4) #0
; CHECK-NEXT: RND   %6 = getelementptr inbounds float, ptr addrspace(1) %output, i64 %2
; CHECK-NEXT: RND   store float %5, ptr addrspace(1) %6, align 4
; CHECK-NEXT: UNI   ret void

define void @__Vectorized_wlacosh2(ptr addrspace(1) nocapture %input, ptr addrspace(1) nocapture %output, i32 %buffer_size) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind
  %2 = and i64 %1, 4294967000  ; <--------notice the constant change
  %3 = getelementptr inbounds float, ptr addrspace(1) %input, i64 %2
  %4 = load float, ptr addrspace(1) %3, align 4
  %5 = tail call float @_Z5acoshf(float %4) nounwind
  %6 = getelementptr inbounds float, ptr addrspace(1) %output, i64 %2
  store float %5, ptr addrspace(1) %6, align 4
  ret void
}

; make sure that the WorkItemAnalysis does NOT detect the ptr as consecutive.
; CHECK: WorkItemAnalysis for function __Vectorized_wlacosh3:
; CHECK-NEXT: SEQ   %1 = tail call i64 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: RND   %2 = and i64 %1, 123415
; CHECK-NEXT: RND   %3 = getelementptr inbounds float, ptr addrspace(1) %input, i64 %2
; CHECK-NEXT: RND   %4 = load float, ptr addrspace(1) %3, align 4
; CHECK-NEXT: RND   %5 = tail call float @_Z5acoshf(float %4) #0
; CHECK-NEXT: RND   %6 = getelementptr inbounds float, ptr addrspace(1) %output, i64 %2
; CHECK-NEXT: RND   store float %5, ptr addrspace(1) %6, align 4
; CHECK-NEXT: UNI   ret void

define void @__Vectorized_wlacosh3(ptr addrspace(1) nocapture %input, ptr addrspace(1) nocapture %output, i32 %buffer_size) nounwind {
  %1 = tail call i64 @_Z13get_global_idj(i32 0) nounwind
  %2 = and i64 %1, 123415  ; <--------notice the constant change
  %3 = getelementptr inbounds float, ptr addrspace(1) %input, i64 %2
  %4 = load float, ptr addrspace(1) %3, align 4
  %5 = tail call float @_Z5acoshf(float %4) nounwind
  %6 = getelementptr inbounds float, ptr addrspace(1) %output, i64 %2
  store float %5, ptr addrspace(1) %6, align 4
  ret void
}
