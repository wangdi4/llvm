; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;test_add(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = in[gid];
;  int y = x + 2;
;
;  out[gid] = y - 10;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_add:
; CHECK-NEXT: SEQ   %1 = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: PTR   %2 = getelementptr inbounds i32, i32 addrspace(1)* %in, i32 %1
; CHECK-NEXT: RND   %3 = load i32, i32 addrspace(1)* %2, align 4
; CHECK-NEXT: RND   %4 = add nsw i32 %3, -8
; CHECK-NEXT: PTR   %5 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %1
; CHECK-NEXT: RND   store i32 %4, i32 addrspace(1)* %5, align 4
; CHECK-NEXT: UNI   ret void

define void @test_add(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = getelementptr inbounds i32, i32 addrspace(1)* %in, i32 %1
  %3 = load i32, i32 addrspace(1)* %2, align 4
  %4 = add nsw i32 %3, -8
  %5 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %1
  store i32 %4, i32 addrspace(1)* %5, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_add, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_add_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
