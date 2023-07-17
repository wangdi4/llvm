; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;test_div(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = 143;
;  int y = in[gid * 7] + 12;
;  int z;
;  int i;
;
;  for (i = 1; i < 1000; i++) {
;    if (i > 12) {
;      y += 12;
;      x -= 1;
;    }
;    y += i;
;  }
;
;  out[gid] = x / y;
;
;  out[gid + 10] = y / x;
;}


target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_div:
; CHECK-NEXT: SEQ   %0 = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: STR   %1 = mul nsw i32 %0, 7
; CHECK-NEXT: RND   %2 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %1
; CHECK-NEXT: RND   %3 = load i32, ptr addrspace(1) %2, align 4
; CHECK-NEXT: RND   %4 = add nsw i32 %3, 12
; CHECK-NEXT: UNI   br label %5
; CHECK-NEXT: UNI   %indvar = phi i32 [ 0, %bb.nph ], [ %i.03, %5 ]
; CHECK-NEXT: RND   %y.12 = phi i32 [ %4, %bb.nph ], [ %tmp, %5 ]
; CHECK-NEXT: UNI   %x.11 = phi i32 [ 143, %bb.nph ], [ %x.0, %5 ]
; CHECK-NEXT: UNI   %i.03 = add i32 %indvar, 1
; CHECK-NEXT: UNI   %6 = icmp sgt i32 %i.03, 12
; CHECK-NEXT: UNI   %7 = add nsw i32 %x.11, -1
; CHECK-NEXT: UNI   %x.0 = select i1 %6, i32 %7, i32 %x.11
; CHECK-NEXT: RND   %8 = add nsw i32 %y.12, 12
; CHECK-NEXT: RND   %y.0 = select i1 %6, i32 %8, i32 %y.12
; CHECK-NEXT: UNI   %exitcond = icmp eq i32 %i.03, 999
; CHECK-NEXT: RND   %tmp = add i32 %y.0, %i.03
; CHECK-NEXT: UNI   br i1 %exitcond, label %._crit_edge, label %5
; CHECK-NEXT: RND   %9 = icmp eq i32 %tmp, 0
; CHECK-NEXT: UNI   %10 = icmp eq i32 %x.0, -2147483648
; CHECK-NEXT: RND   %11 = icmp eq i32 %tmp, -1
; CHECK-NEXT: RND   %12 = and i1 %10, %11
; CHECK-NEXT: RND   %13 = or i1 %9, %12
; CHECK-NEXT: RND   %14 = select i1 %13, i32 1, i32 %tmp
; CHECK-NEXT: RND   %15 = sdiv i32 %x.0, %14
; CHECK-NEXT: PTR   %16 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %0
; CHECK-NEXT: RND   store i32 %15, ptr addrspace(1) %16, align 4
; CHECK-NEXT: UNI   %17 = icmp eq i32 %x.0, 0
; CHECK-NEXT: RND   %18 = icmp eq i32 %tmp, -2147483648
; CHECK-NEXT: UNI   %19 = icmp eq i32 %x.0, -1
; CHECK-NEXT: RND   %20 = and i1 %18, %19
; CHECK-NEXT: RND   %21 = or i1 %17, %20
; CHECK-NEXT: RND   %22 = select i1 %21, i32 1, i32 %x.0
; CHECK-NEXT: RND   %23 = sdiv i32 %tmp, %22
; CHECK-NEXT: SEQ   %24 = add nsw i32 %0, 10
; CHECK-NEXT: PTR   %25 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %24
; CHECK-NEXT: RND   store i32 %23, ptr addrspace(1) %25, align 4
; CHECK-NEXT: UNI   ret void

define void @test_div(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
bb.nph:
  %0 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %1 = mul nsw i32 %0, 7
  %2 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %1
  %3 = load i32, ptr addrspace(1) %2, align 4
  %4 = add nsw i32 %3, 12
  br label %5

; <label>:5                                       ; preds = %5, %bb.nph
  %indvar = phi i32 [ 0, %bb.nph ], [ %i.03, %5 ]
  %y.12 = phi i32 [ %4, %bb.nph ], [ %tmp, %5 ]
  %x.11 = phi i32 [ 143, %bb.nph ], [ %x.0, %5 ]
  %i.03 = add i32 %indvar, 1
  %6 = icmp sgt i32 %i.03, 12
  %7 = add nsw i32 %x.11, -1
  %x.0 = select i1 %6, i32 %7, i32 %x.11
  %8 = add nsw i32 %y.12, 12
  %y.0 = select i1 %6, i32 %8, i32 %y.12
  %exitcond = icmp eq i32 %i.03, 999
  %tmp = add i32 %y.0, %i.03
  br i1 %exitcond, label %._crit_edge, label %5

._crit_edge:                                      ; preds = %5
  %9 = icmp eq i32 %tmp, 0
  %10 = icmp eq i32 %x.0, -2147483648
  %11 = icmp eq i32 %tmp, -1
  %12 = and i1 %10, %11
  %13 = or i1 %9, %12
  %14 = select i1 %13, i32 1, i32 %tmp
  %15 = sdiv i32 %x.0, %14
  %16 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %0
  store i32 %15, ptr addrspace(1) %16, align 4
  %17 = icmp eq i32 %x.0, 0
  %18 = icmp eq i32 %tmp, -2147483648
  %19 = icmp eq i32 %x.0, -1
  %20 = and i1 %18, %19
  %21 = or i1 %17, %20
  %22 = select i1 %21, i32 1, i32 %x.0
  %23 = sdiv i32 %tmp, %22
  %24 = add nsw i32 %0, 10
  %25 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %24
  store i32 %23, ptr addrspace(1) %25, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_div, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_div_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}
