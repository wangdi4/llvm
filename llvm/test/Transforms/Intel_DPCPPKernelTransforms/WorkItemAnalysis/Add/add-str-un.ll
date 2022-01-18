; RUN: opt -passes='require<dpcpp-kernel-builtin-info-analysis>,print<dpcpp-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s
; RUN: opt -analyze -dpcpp-kernel-work-item-analysis %s -S -o - | FileCheck %s

;kernel void
;test_add(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = gid * 5;
;  int y = x + 2;
;
;  out[gid] = y - 10;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_add:
; CHECK-NEXT: SEQ   %1 = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: STR   %2 = mul nsw i32 %1, 5
; CHECK-NEXT: STR   %3 = add nsw i32 %2, -8
; CHECK-NEXT: PTR   %4 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %1
; CHECK-NEXT: RND   store i32 %3, i32 addrspace(1)* %4, align 4
; CHECK-NEXT: UNI   ret void

define void @test_add(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = mul nsw i32 %1, 5
  %3 = add nsw i32 %2, -8
  %4 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %1
  store i32 %3, i32 addrspace(1)* %4, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_add, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_add_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
