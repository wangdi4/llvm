; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;test_div(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = gid + 100;
;  int y = gid;
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
; CHECK-NEXT: RND   %3 = icmp eq i32 %1, 0
; CHECK-NEXT: RND   %4 = icmp eq i32 %2, -2147483648
; CHECK-NEXT: RND   %5 = icmp eq i32 %1, -1
; CHECK-NEXT: RND   %6 = and i1 %4, %5
; CHECK-NEXT: RND   %7 = or i1 %3, %6
; CHECK-NEXT: RND   %8 = select i1 %7, i32 1, i32 %1
; CHECK-NEXT: RND   %9 = sdiv i32 %2, %8
; CHECK-NEXT: PTR   %10 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
; CHECK-NEXT: RND   store i32 %9, ptr addrspace(1) %10, align 4
; CHECK-NEXT: RND   %11 = icmp eq i32 %2, 0
; CHECK-NEXT: RND   %12 = icmp eq i32 %1, -2147483648
; CHECK-NEXT: RND   %13 = icmp eq i32 %2, -1
; CHECK-NEXT: RND   %14 = and i1 %12, %13
; CHECK-NEXT: RND   %15 = or i1 %11, %14
; CHECK-NEXT: RND   %16 = select i1 %15, i32 1, i32 %2
; CHECK-NEXT: RND   %17 = sdiv i32 %1, %16
; CHECK-NEXT: SEQ   %18 = add nsw i32 %1, 10
; CHECK-NEXT: PTR   %19 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %18
; CHECK-NEXT: RND   store i32 %17, ptr addrspace(1) %19, align 4
; CHECK-NEXT: UNI   ret void

define void @test_div(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = add nsw i32 %1, 100
  %3 = icmp eq i32 %1, 0
  %4 = icmp eq i32 %2, -2147483648
  %5 = icmp eq i32 %1, -1
  %6 = and i1 %4, %5
  %7 = or i1 %3, %6
  %8 = select i1 %7, i32 1, i32 %1
  %9 = sdiv i32 %2, %8
  %10 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
  store i32 %9, ptr addrspace(1) %10, align 4
  %11 = icmp eq i32 %2, 0
  %12 = icmp eq i32 %1, -2147483648
  %13 = icmp eq i32 %2, -1
  %14 = and i1 %12, %13
  %15 = or i1 %11, %14
  %16 = select i1 %15, i32 1, i32 %2
  %17 = sdiv i32 %1, %16
  %18 = add nsw i32 %1, 10
  %19 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %18
  store i32 %17, ptr addrspace(1) %19, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_div, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_div_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}
