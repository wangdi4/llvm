; RUN: opt -passes='require<dpcpp-kernel-builtin-info-analysis>,print<dpcpp-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s
; RUN: opt -analyze -enable-new-pm=0 -dpcpp-kernel-work-item-analysis %s -S -o - | FileCheck %s

;kernel void
;test_div(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = 143;
;  int y = 12;
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
; CHECK-NEXT: UNI   br label %1
; CHECK-NEXT: UNI   %indvar = phi i32 [ 0, %bb.nph ], [ %i.03, %1 ]
; CHECK-NEXT: UNI   %y.12 = phi i32 [ 12, %bb.nph ], [ %tmp, %1 ]
; CHECK-NEXT: UNI   %x.11 = phi i32 [ 143, %bb.nph ], [ %x.0, %1 ]
; CHECK-NEXT: UNI   %i.03 = add i32 %indvar, 1
; CHECK-NEXT: UNI   %2 = icmp sgt i32 %i.03, 12
; CHECK-NEXT: UNI   %3 = add nsw i32 %x.11, -1
; CHECK-NEXT: UNI   %x.0 = select i1 %2, i32 %3, i32 %x.11
; CHECK-NEXT: UNI   %4 = add nsw i32 %y.12, 12
; CHECK-NEXT: UNI   %y.0 = select i1 %2, i32 %4, i32 %y.12
; CHECK-NEXT: UNI   %exitcond = icmp eq i32 %i.03, 999
; CHECK-NEXT: UNI   %tmp = add i32 %y.0, %i.03
; CHECK-NEXT: UNI   br i1 %exitcond, label %._crit_edge, label %1
; CHECK-NEXT: UNI   %5 = icmp eq i32 %tmp, 0
; CHECK-NEXT: UNI   %6 = icmp eq i32 %x.0, -2147483648
; CHECK-NEXT: UNI   %7 = icmp eq i32 %tmp, -1
; CHECK-NEXT: UNI   %8 = and i1 %6, %7
; CHECK-NEXT: UNI   %9 = or i1 %5, %8
; CHECK-NEXT: UNI   %10 = select i1 %9, i32 1, i32 %tmp
; CHECK-NEXT: UNI   %11 = sdiv i32 %x.0, %10
; CHECK-NEXT: PTR   %12 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %0
; CHECK-NEXT: RND   store i32 %11, i32 addrspace(1)* %12, align 4
; CHECK-NEXT: UNI   %13 = icmp eq i32 %x.0, 0
; CHECK-NEXT: UNI   %14 = icmp eq i32 %tmp, -2147483648
; CHECK-NEXT: UNI   %15 = icmp eq i32 %x.0, -1
; CHECK-NEXT: UNI   %16 = and i1 %14, %15
; CHECK-NEXT: UNI   %17 = or i1 %13, %16
; CHECK-NEXT: UNI   %18 = select i1 %17, i32 1, i32 %x.0
; CHECK-NEXT: UNI   %19 = sdiv i32 %tmp, %18
; CHECK-NEXT: SEQ   %20 = add nsw i32 %0, 10
; CHECK-NEXT: PTR   %21 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %20
; CHECK-NEXT: RND   store i32 %19, i32 addrspace(1)* %21, align 4
; CHECK-NEXT: UNI   ret void

define void @test_div(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) nounwind {
bb.nph:
  %0 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  br label %1

; <label>:1                                       ; preds = %1, %bb.nph
  %indvar = phi i32 [ 0, %bb.nph ], [ %i.03, %1 ]
  %y.12 = phi i32 [ 12, %bb.nph ], [ %tmp, %1 ]
  %x.11 = phi i32 [ 143, %bb.nph ], [ %x.0, %1 ]
  %i.03 = add i32 %indvar, 1
  %2 = icmp sgt i32 %i.03, 12
  %3 = add nsw i32 %x.11, -1
  %x.0 = select i1 %2, i32 %3, i32 %x.11
  %4 = add nsw i32 %y.12, 12
  %y.0 = select i1 %2, i32 %4, i32 %y.12
  %exitcond = icmp eq i32 %i.03, 999
  %tmp = add i32 %y.0, %i.03
  br i1 %exitcond, label %._crit_edge, label %1

._crit_edge:                                      ; preds = %1
  %5 = icmp eq i32 %tmp, 0
  %6 = icmp eq i32 %x.0, -2147483648
  %7 = icmp eq i32 %tmp, -1
  %8 = and i1 %6, %7
  %9 = or i1 %5, %8
  %10 = select i1 %9, i32 1, i32 %tmp
  %11 = sdiv i32 %x.0, %10
  %12 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %0
  store i32 %11, i32 addrspace(1)* %12, align 4
  %13 = icmp eq i32 %x.0, 0
  %14 = icmp eq i32 %tmp, -2147483648
  %15 = icmp eq i32 %x.0, -1
  %16 = and i1 %14, %15
  %17 = or i1 %13, %16
  %18 = select i1 %17, i32 1, i32 %x.0
  %19 = sdiv i32 %tmp, %18
  %20 = add nsw i32 %0, 10
  %21 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %20
  store i32 %19, i32 addrspace(1)* %21, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_div, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_div_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
