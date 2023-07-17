; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;test_div(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = gid + 100;
;  int y = in[gid] * 7 + 12;
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
; CHECK-NEXT: SEQ   %2 = add nsw i32 %1, 100
; CHECK-NEXT: PTR   %3 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %1
; CHECK-NEXT: RND   %4 = load i32, ptr addrspace(1) %3, align 4
; CHECK-NEXT: RND   %5 = mul nsw i32 %4, 7
; CHECK-NEXT: RND   %6 = add nsw i32 %5, 12
; CHECK-NEXT: RND   %7 = icmp eq i32 %6, 0
; CHECK-NEXT: RND   %8 = icmp eq i32 %2, -2147483648
; CHECK-NEXT: RND   %9 = icmp eq i32 %6, -1
; CHECK-NEXT: RND   %10 = and i1 %8, %9
; CHECK-NEXT: RND   %11 = or i1 %7, %10
; CHECK-NEXT: RND   %12 = select i1 %11, i32 1, i32 %6
; CHECK-NEXT: RND   %13 = sdiv i32 %2, %12
; CHECK-NEXT: PTR   %14 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
; CHECK-NEXT: RND   store i32 %13, ptr addrspace(1) %14, align 4
; CHECK-NEXT: RND   %15 = icmp eq i32 %2, 0
; CHECK-NEXT: RND   %16 = icmp eq i32 %6, -2147483648
; CHECK-NEXT: RND   %17 = icmp eq i32 %2, -1
; CHECK-NEXT: RND   %18 = and i1 %16, %17
; CHECK-NEXT: RND   %19 = or i1 %15, %18
; CHECK-NEXT: RND   %20 = select i1 %19, i32 1, i32 %2
; CHECK-NEXT: RND   %21 = sdiv i32 %6, %20
; CHECK-NEXT: SEQ   %22 = add nsw i32 %1, 10
; CHECK-NEXT: PTR   %23 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %22
; CHECK-NEXT: RND   store i32 %21, ptr addrspace(1) %23, align 4
; CHECK-NEXT: UNI   ret void

define void @test_div(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = add nsw i32 %1, 100
  %3 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %1
  %4 = load i32, ptr addrspace(1) %3, align 4
  %5 = mul nsw i32 %4, 7
  %6 = add nsw i32 %5, 12
  %7 = icmp eq i32 %6, 0
  %8 = icmp eq i32 %2, -2147483648
  %9 = icmp eq i32 %6, -1
  %10 = and i1 %8, %9
  %11 = or i1 %7, %10
  %12 = select i1 %11, i32 1, i32 %6
  %13 = sdiv i32 %2, %12
  %14 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
  store i32 %13, ptr addrspace(1) %14, align 4
  %15 = icmp eq i32 %2, 0
  %16 = icmp eq i32 %6, -2147483648
  %17 = icmp eq i32 %2, -1
  %18 = and i1 %16, %17
  %19 = or i1 %15, %18
  %20 = select i1 %19, i32 1, i32 %2
  %21 = sdiv i32 %6, %20
  %22 = add nsw i32 %1, 10
  %23 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %22
  store i32 %21, ptr addrspace(1) %23, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_div, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_div_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}
