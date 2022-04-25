; RUN: opt -passes='require<dpcpp-kernel-builtin-info-analysis>,print<dpcpp-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s
; RUN: opt -analyze -enable-new-pm=0 -dpcpp-kernel-work-item-analysis %s -S -o - | FileCheck %s

;kernel void
;test_add(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = gid * 2 + 123;
;  int y = gid * 11;
;
;  y = y - gid;
;
;  out[gid] = x + y;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_add:
; CHECK-NEXT: SEQ   %1 = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: STR   %tmp = mul i32 %1, 12
; CHECK-NEXT: STR   %2 = add nsw i32 %tmp, 123
; CHECK-NEXT: PTR   %3 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %1
; CHECK-NEXT: RND   store i32 %2, i32 addrspace(1)* %3, align 4
; CHECK-NEXT: UNI   ret void

define void @test_add(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %tmp = mul i32 %1, 12
  %2 = add nsw i32 %tmp, 123
  %3 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %1
  store i32 %2, i32 addrspace(1)* %3, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_add, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_add_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
