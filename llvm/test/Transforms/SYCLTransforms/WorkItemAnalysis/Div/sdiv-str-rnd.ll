; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;test_div(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = gid * 11 + 100;
;  int y = in[gid] * 7 - 12;
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
; CHECK-NEXT: PTR   %4 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %1
; CHECK-NEXT: RND   %5 = load i32, ptr addrspace(1) %4, align 4
; CHECK-NEXT: RND   %6 = mul nsw i32 %5, 7
; CHECK-NEXT: RND   %7 = add nsw i32 %6, -12
; CHECK-NEXT: RND   %8 = icmp eq i32 %7, 0
; CHECK-NEXT: RND   %9 = icmp eq i32 %3, -2147483648
; CHECK-NEXT: RND   %10 = icmp eq i32 %7, -1
; CHECK-NEXT: RND   %11 = and i1 %9, %10
; CHECK-NEXT: RND   %12 = or i1 %8, %11
; CHECK-NEXT: RND   %13 = select i1 %12, i32 1, i32 %7
; CHECK-NEXT: RND   %14 = sdiv i32 %3, %13
; CHECK-NEXT: PTR   %15 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
; CHECK-NEXT: RND   store i32 %14, ptr addrspace(1) %15, align 4
; CHECK-NEXT: RND   %16 = icmp eq i32 %3, 0
; CHECK-NEXT: RND   %17 = icmp eq i32 %7, -2147483648
; CHECK-NEXT: RND   %18 = icmp eq i32 %3, -1
; CHECK-NEXT: RND   %19 = and i1 %17, %18
; CHECK-NEXT: RND   %20 = or i1 %16, %19
; CHECK-NEXT: RND   %21 = select i1 %20, i32 1, i32 %3
; CHECK-NEXT: RND   %22 = sdiv i32 %7, %21
; CHECK-NEXT: SEQ   %23 = add nsw i32 %1, 10
; CHECK-NEXT: PTR   %24 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %23
; CHECK-NEXT: RND   store i32 %22, ptr addrspace(1) %24, align 4
; CHECK-NEXT: UNI   ret void

define void @test_div(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = mul nsw i32 %1, 11
  %3 = add nsw i32 %2, 100
  %4 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %1
  %5 = load i32, ptr addrspace(1) %4, align 4
  %6 = mul nsw i32 %5, 7
  %7 = add nsw i32 %6, -12
  %8 = icmp eq i32 %7, 0
  %9 = icmp eq i32 %3, -2147483648
  %10 = icmp eq i32 %7, -1
  %11 = and i1 %9, %10
  %12 = or i1 %8, %11
  %13 = select i1 %12, i32 1, i32 %7
  %14 = sdiv i32 %3, %13
  %15 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
  store i32 %14, ptr addrspace(1) %15, align 4
  %16 = icmp eq i32 %3, 0
  %17 = icmp eq i32 %7, -2147483648
  %18 = icmp eq i32 %3, -1
  %19 = and i1 %17, %18
  %20 = or i1 %16, %19
  %21 = select i1 %20, i32 1, i32 %3
  %22 = sdiv i32 %7, %21
  %23 = add nsw i32 %1, 10
  %24 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %23
  store i32 %22, ptr addrspace(1) %24, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_div, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_div_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}
