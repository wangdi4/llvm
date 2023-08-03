; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;test_div(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = gid + 100;
;  int y = gid * 7 + 12;
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
; CHECK-NEXT: STR   %3 = mul nsw i32 %1, 7
; CHECK-NEXT: STR   %4 = add nsw i32 %3, 12
; CHECK-NEXT: RND   %5 = icmp eq i32 %4, 0
; CHECK-NEXT: RND   %6 = icmp eq i32 %2, -2147483648
; CHECK-NEXT: RND   %7 = icmp eq i32 %4, -1
; CHECK-NEXT: RND   %8 = and i1 %6, %7
; CHECK-NEXT: RND   %9 = or i1 %5, %8
; CHECK-NEXT: RND   %10 = select i1 %9, i32 1, i32 %4
; CHECK-NEXT: RND   %11 = sdiv i32 %2, %10
; CHECK-NEXT: PTR   %12 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
; CHECK-NEXT: RND   store i32 %11, ptr addrspace(1) %12, align 4
; CHECK-NEXT: RND   %13 = icmp eq i32 %2, 0
; CHECK-NEXT: RND   %14 = icmp eq i32 %4, -2147483648
; CHECK-NEXT: RND   %15 = icmp eq i32 %2, -1
; CHECK-NEXT: RND   %16 = and i1 %14, %15
; CHECK-NEXT: RND   %17 = or i1 %13, %16
; CHECK-NEXT: RND   %18 = select i1 %17, i32 1, i32 %2
; CHECK-NEXT: RND   %19 = sdiv i32 %4, %18
; CHECK-NEXT: SEQ   %20 = add nsw i32 %1, 10
; CHECK-NEXT: PTR   %21 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %20
; CHECK-NEXT: RND   store i32 %19, ptr addrspace(1) %21, align 4
; CHECK-NEXT: UNI   ret void

define void @test_div(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = add nsw i32 %1, 100
  %3 = mul nsw i32 %1, 7
  %4 = add nsw i32 %3, 12
  %5 = icmp eq i32 %4, 0
  %6 = icmp eq i32 %2, -2147483648
  %7 = icmp eq i32 %4, -1
  %8 = and i1 %6, %7
  %9 = or i1 %5, %8
  %10 = select i1 %9, i32 1, i32 %4
  %11 = sdiv i32 %2, %10
  %12 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
  store i32 %11, ptr addrspace(1) %12, align 4
  %13 = icmp eq i32 %2, 0
  %14 = icmp eq i32 %4, -2147483648
  %15 = icmp eq i32 %2, -1
  %16 = and i1 %14, %15
  %17 = or i1 %13, %16
  %18 = select i1 %17, i32 1, i32 %2
  %19 = sdiv i32 %4, %18
  %20 = add nsw i32 %1, 10
  %21 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %20
  store i32 %19, ptr addrspace(1) %21, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_div, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_div_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}
