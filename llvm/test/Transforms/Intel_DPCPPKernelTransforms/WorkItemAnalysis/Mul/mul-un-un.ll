; RUN: opt -passes='require<dpcpp-kernel-builtin-info-analysis>,print<dpcpp-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s
; RUN: opt -analyze -dpcpp-kernel-work-item-analysis %s -S -o - | FileCheck %s

;__kernel void
;test_mul(__global int *in, __global int *out) {
;  int gid = get_global_id(0);
;  int x = 1;
;  int y = 0;
;  int i = 0;
;  int z;
;
;  for (i = 0; i < 2000; i++) {
;    if (i > 300) {
;      x += x;
;    } else {
;      y += 1;
;    }
;  }
;
;  z = x * y;
;
;  out[gid] = z;
;
;  z = z + 100;
;  out[gid + 10] = z;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_mul:
; CHECK-NEXT: SEQ   %i = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: UNI   br label %bb
; CHECK-NEXT: UNI   %i1 = phi i32 [ 0, %bb.nph ], [ %i5, %bb ]
; CHECK-NEXT: UNI   %y.12 = phi i32 [ 0, %bb.nph ], [ %y.0, %bb ]
; CHECK-NEXT: UNI   %x.11 = phi i32 [ 1, %bb.nph ], [ %x.0, %bb ]
; CHECK-NEXT: UNI   %i2 = icmp sgt i32 %i1, 300
; CHECK-NEXT: UNI   %i3 = zext i1 %i2 to i32
; CHECK-NEXT: UNI   %x.0 = shl i32 %x.11, %i3
; CHECK-NEXT: UNI   %i4 = xor i32 %i3, 1
; CHECK-NEXT: UNI   %y.0 = add i32 %i4, %y.12
; CHECK-NEXT: UNI   %i5 = add nsw i32 %i1, 1
; CHECK-NEXT: UNI   %exitcond = icmp eq i32 %i5, 2000
; CHECK-NEXT: UNI   br i1 %exitcond, label %._crit_edge, label %bb
; CHECK-NEXT: UNI   %i6 = mul nsw i32 %y.0, %x.0
; CHECK-NEXT: PTR   %i7 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %i
; CHECK-NEXT: RND   store i32 %i6, i32 addrspace(1)* %i7, align 4
; CHECK-NEXT: UNI   %i8 = add nsw i32 %i6, 100
; CHECK-NEXT: SEQ   %i9 = add nsw i32 %i, 10
; CHECK-NEXT: PTR   %i10 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %i9
; CHECK-NEXT: RND   store i32 %i8, i32 addrspace(1)* %i10, align 4
; CHECK-NEXT: UNI   ret void

; Function Attrs: nounwind
define void @test_mul(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) #0 {
bb.nph:
  %i = tail call i32 @_Z13get_global_idj(i32 0) #0
  br label %bb

bb:                                               ; preds = %bb, %bb.nph
  %i1 = phi i32 [ 0, %bb.nph ], [ %i5, %bb ]
  %y.12 = phi i32 [ 0, %bb.nph ], [ %y.0, %bb ]
  %x.11 = phi i32 [ 1, %bb.nph ], [ %x.0, %bb ]
  %i2 = icmp sgt i32 %i1, 300
  %i3 = zext i1 %i2 to i32
  %x.0 = shl i32 %x.11, %i3
  %i4 = xor i32 %i3, 1
  %y.0 = add i32 %i4, %y.12
  %i5 = add nsw i32 %i1, 1
  %exitcond = icmp eq i32 %i5, 2000
  br i1 %exitcond, label %._crit_edge, label %bb

._crit_edge:                                      ; preds = %bb
  %i6 = mul nsw i32 %y.0, %x.0
  %i7 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %i
  store i32 %i6, i32 addrspace(1)* %i7, align 4
  %i8 = add nsw i32 %i6, 100
  %i9 = add nsw i32 %i, 10
  %i10 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %i9
  store i32 %i8, i32 addrspace(1)* %i10, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

attributes #0 = { nounwind }

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_mul, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_mul_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
