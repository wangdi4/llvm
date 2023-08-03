; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;test_add(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = gid * 10;
;  int y = out[gid] + 2;
;
;  out[gid] = x - y;
;  out[gid + gid] = x + y + 1;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_add:
; CHECK-NEXT: SEQ   %1 = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: STR   %2 = mul nsw i32 %1, 10
; CHECK-NEXT: PTR   %3 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
; CHECK-NEXT: RND   %4 = load i32, ptr addrspace(1) %3, align 4
; CHECK-NEXT: RND   %5 = add nsw i32 %4, 2
; CHECK-NEXT: RND   %6 = sub nsw i32 %2, %5
; CHECK-NEXT: RND   store i32 %6, ptr addrspace(1) %3, align 4
; CHECK-NEXT: RND   %7 = or i32 %2, 1
; CHECK-NEXT: RND   %8 = add nsw i32 %7, %5
; CHECK-NEXT: STR   %9 = shl i32 %1, 1
; CHECK-NEXT: RND   %10 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %9
; CHECK-NEXT: RND   store i32 %8, ptr addrspace(1) %10, align 4
; CHECK-NEXT: UNI   ret void

define void @test_add(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = mul nsw i32 %1, 10
  %3 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
  %4 = load i32, ptr addrspace(1) %3, align 4
  %5 = add nsw i32 %4, 2
  %6 = sub nsw i32 %2, %5
  store i32 %6, ptr addrspace(1) %3, align 4
  %7 = or i32 %2, 1
  %8 = add nsw i32 %7, %5
  %9 = shl i32 %1, 1
  %10 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %9
  store i32 %8, ptr addrspace(1) %10, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_add, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_add_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}
