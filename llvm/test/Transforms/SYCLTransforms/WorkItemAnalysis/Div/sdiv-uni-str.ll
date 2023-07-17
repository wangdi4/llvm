; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;test_div(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = 143;
;  int y = gid * 7 + 12;
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
; CHECK-NEXT: STR   %2 = add nsw i32 %1, 12
; CHECK-NEXT: UNI   br label %3
; CHECK-NEXT: UNI   %indvar = phi i32 [ 0, %bb.nph ], [ %i.03, %3 ]
; CHECK-NEXT: STR   %y.12 = phi i32 [ %2, %bb.nph ], [ %tmp, %3 ]
; CHECK-NEXT: UNI   %x.11 = phi i32 [ 143, %bb.nph ], [ %x.0, %3 ]
; CHECK-NEXT: UNI   %i.03 = add i32 %indvar, 1
; CHECK-NEXT: UNI   %4 = icmp sgt i32 %i.03, 12
; CHECK-NEXT: UNI   %5 = add nsw i32 %x.11, -1
; CHECK-NEXT: UNI   %x.0 = select i1 %4, i32 %5, i32 %x.11
; CHECK-NEXT: STR   %6 = add nsw i32 %y.12, 12
; CHECK-NEXT: STR   %y.0 = select i1 %4, i32 %6, i32 %y.12
; CHECK-NEXT: UNI   %exitcond = icmp eq i32 %i.03, 999
; CHECK-NEXT: STR   %tmp = add i32 %y.0, %i.03
; CHECK-NEXT: UNI   br i1 %exitcond, label %._crit_edge, label %3
; CHECK-NEXT: RND   %7 = icmp eq i32 %tmp, 0
; CHECK-NEXT: UNI   %8 = icmp eq i32 %x.0, -2147483648
; CHECK-NEXT: RND   %9 = icmp eq i32 %tmp, -1
; CHECK-NEXT: RND   %10 = and i1 %8, %9
; CHECK-NEXT: RND   %11 = or i1 %7, %10
; CHECK-NEXT: RND   %12 = select i1 %11, i32 1, i32 %tmp
; CHECK-NEXT: RND   %13 = sdiv i32 %x.0, %12
; CHECK-NEXT: PTR   %14 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %0
; CHECK-NEXT: RND   store i32 %13, ptr addrspace(1) %14, align 4
; CHECK-NEXT: UNI   %15 = icmp eq i32 %x.0, 0
; CHECK-NEXT: RND   %16 = icmp eq i32 %tmp, -2147483648
; CHECK-NEXT: UNI   %17 = icmp eq i32 %x.0, -1
; CHECK-NEXT: RND   %18 = and i1 %16, %17
; CHECK-NEXT: RND   %19 = or i1 %15, %18
; CHECK-NEXT: RND   %20 = select i1 %19, i32 1, i32 %x.0
; CHECK-NEXT: RND   %21 = sdiv i32 %tmp, %20
; CHECK-NEXT: SEQ   %22 = add nsw i32 %0, 10
; CHECK-NEXT: PTR   %23 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %22
; CHECK-NEXT: RND   store i32 %21, ptr addrspace(1) %23, align 4
; CHECK-NEXT: UNI   ret void

define void @test_div(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
bb.nph:
  %0 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %1 = mul nsw i32 %0, 7
  %2 = add nsw i32 %1, 12
  br label %3

; <label>:3                                       ; preds = %3, %bb.nph
  %indvar = phi i32 [ 0, %bb.nph ], [ %i.03, %3 ]
  %y.12 = phi i32 [ %2, %bb.nph ], [ %tmp, %3 ]
  %x.11 = phi i32 [ 143, %bb.nph ], [ %x.0, %3 ]
  %i.03 = add i32 %indvar, 1
  %4 = icmp sgt i32 %i.03, 12
  %5 = add nsw i32 %x.11, -1
  %x.0 = select i1 %4, i32 %5, i32 %x.11
  %6 = add nsw i32 %y.12, 12
  %y.0 = select i1 %4, i32 %6, i32 %y.12
  %exitcond = icmp eq i32 %i.03, 999
  %tmp = add i32 %y.0, %i.03
  br i1 %exitcond, label %._crit_edge, label %3

._crit_edge:                                      ; preds = %3
  %7 = icmp eq i32 %tmp, 0
  %8 = icmp eq i32 %x.0, -2147483648
  %9 = icmp eq i32 %tmp, -1
  %10 = and i1 %8, %9
  %11 = or i1 %7, %10
  %12 = select i1 %11, i32 1, i32 %tmp
  %13 = sdiv i32 %x.0, %12
  %14 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %0
  store i32 %13, ptr addrspace(1) %14, align 4
  %15 = icmp eq i32 %x.0, 0
  %16 = icmp eq i32 %tmp, -2147483648
  %17 = icmp eq i32 %x.0, -1
  %18 = and i1 %16, %17
  %19 = or i1 %15, %18
  %20 = select i1 %19, i32 1, i32 %x.0
  %21 = sdiv i32 %tmp, %20
  %22 = add nsw i32 %0, 10
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
