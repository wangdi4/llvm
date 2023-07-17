; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;test_div(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = gid * 11 + 100;
;  int y = gid * 7 - 12;
;  int z;
;  int i;
;
;  out[gid] = x / y;
;
;  out[gid + 10] = y / x;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_div:
; CHECK-NEXT: SEQ   %1 = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: STR   %2 = mul nsw i32 %1, 11
; CHECK-NEXT: STR   %3 = add nsw i32 %2, 100
; CHECK-NEXT: STR   %4 = mul nsw i32 %1, 7
; CHECK-NEXT: STR   %5 = add nsw i32 %4, -12
; CHECK-NEXT: RND   %6 = icmp eq i32 %5, 0
; CHECK-NEXT: RND   %7 = icmp eq i32 %3, -2147483648
; CHECK-NEXT: RND   %8 = icmp eq i32 %5, -1
; CHECK-NEXT: RND   %9 = and i1 %7, %8
; CHECK-NEXT: RND   %10 = or i1 %6, %9
; CHECK-NEXT: RND   %11 = select i1 %10, i32 1, i32 %5
; CHECK-NEXT: RND   %12 = sdiv i32 %3, %11
; CHECK-NEXT: PTR   %13 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
; CHECK-NEXT: RND   store i32 %12, ptr addrspace(1) %13, align 4
; CHECK-NEXT: RND   %14 = icmp eq i32 %3, 0
; CHECK-NEXT: RND   %15 = icmp eq i32 %5, -2147483648
; CHECK-NEXT: RND   %16 = icmp eq i32 %3, -1
; CHECK-NEXT: RND   %17 = and i1 %15, %16
; CHECK-NEXT: RND   %18 = or i1 %14, %17
; CHECK-NEXT: RND   %19 = select i1 %18, i32 1, i32 %3
; CHECK-NEXT: RND   %20 = sdiv i32 %5, %19
; CHECK-NEXT: SEQ   %21 = add nsw i32 %1, 10
; CHECK-NEXT: PTR   %22 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %21
; CHECK-NEXT: RND   store i32 %20, ptr addrspace(1) %22, align 4
; CHECK-NEXT: UNI   ret void

define void @test_div(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = mul nsw i32 %1, 11
  %3 = add nsw i32 %2, 100
  %4 = mul nsw i32 %1, 7
  %5 = add nsw i32 %4, -12
  %6 = icmp eq i32 %5, 0
  %7 = icmp eq i32 %3, -2147483648
  %8 = icmp eq i32 %5, -1
  %9 = and i1 %7, %8
  %10 = or i1 %6, %9
  %11 = select i1 %10, i32 1, i32 %5
  %12 = sdiv i32 %3, %11
  %13 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
  store i32 %12, ptr addrspace(1) %13, align 4
  %14 = icmp eq i32 %3, 0
  %15 = icmp eq i32 %5, -2147483648
  %16 = icmp eq i32 %3, -1
  %17 = and i1 %15, %16
  %18 = or i1 %14, %17
  %19 = select i1 %18, i32 1, i32 %3
  %20 = sdiv i32 %5, %19
  %21 = add nsw i32 %1, 10
  %22 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %21
  store i32 %20, ptr addrspace(1) %22, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_div, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_div_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}
