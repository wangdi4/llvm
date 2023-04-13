; RUN: opt -opaque-pointers=0 -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;__kernel void
;test_mul(__global int *in, __global int *out) {
;  int gid = get_global_id(0);
;  int x = gid * 12;
;  int y = ((gid + 1) * 3 + 2) * 2 + 11;
;  int i = 0;
;  int z = 3 + y;
;
;  for (i = 0; i < 2000; i++) {
;    if (i > 300) {
;      x += x;
;    }
;  }
;
;  z = x * y;
;
;  out[gid] = z;
;
;  z = z * (gid * 4 + 12);
;  out[gid + 10] = z;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_mul:
; CHECK-NEXT: SEQ   %i = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: STR   %i1 = mul nsw i32 %i, 12
; CHECK-NEXT: STR   %i2 = mul i32 %i, 6
; CHECK-NEXT: STR   %i3 = add nsw i32 %i2, 21
; CHECK-NEXT: UNI   br label %bb
; CHECK-NEXT: UNI   %i4 = phi i32 [ 0, %bb.nph ], [ %i7, %bb ]
; CHECK-NEXT: STR   %x.11 = phi i32 [ %i1, %bb.nph ], [ %.x.1, %bb ]
; CHECK-NEXT: UNI   %i5 = icmp sgt i32 %i4, 300
; CHECK-NEXT: UNI   %i6 = zext i1 %i5 to i32
; CHECK-NEXT: STR   %.x.1 = shl i32 %x.11, %i6
; CHECK-NEXT: UNI   %i7 = add nsw i32 %i4, 1
; CHECK-NEXT: UNI   %exitcond = icmp eq i32 %i7, 2000
; CHECK-NEXT: UNI   br i1 %exitcond, label %._crit_edge, label %bb
; CHECK-NEXT: RND   %i8 = mul nsw i32 %.x.1, %i3
; CHECK-NEXT: PTR   %i9 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %i
; CHECK-NEXT: RND   store i32 %i8, i32 addrspace(1)* %i9, align 4
; CHECK-NEXT: STR   %i10 = shl i32 %i, 2
; CHECK-NEXT: STR   %i11 = add nsw i32 %i10, 12
; CHECK-NEXT: RND   %i12 = mul nsw i32 %i8, %i11
; CHECK-NEXT: SEQ   %i13 = add nsw i32 %i, 10
; CHECK-NEXT: PTR   %i14 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %i13
; CHECK-NEXT: RND   store i32 %i12, i32 addrspace(1)* %i14, align 4
; CHECK-NEXT: UNI   ret void

; Function Attrs: nounwind
define void @test_mul(i32 addrspace(1)* nocapture %in, i32 addrspace(1)* nocapture %out) #0 {
bb.nph:
  %i = tail call i32 @_Z13get_global_idj(i32 0) #0
  %i1 = mul nsw i32 %i, 12
  %i2 = mul i32 %i, 6
  %i3 = add nsw i32 %i2, 21
  br label %bb

bb:                                               ; preds = %bb, %bb.nph
  %i4 = phi i32 [ 0, %bb.nph ], [ %i7, %bb ]
  %x.11 = phi i32 [ %i1, %bb.nph ], [ %.x.1, %bb ]
  %i5 = icmp sgt i32 %i4, 300
  %i6 = zext i1 %i5 to i32
  %.x.1 = shl i32 %x.11, %i6
  %i7 = add nsw i32 %i4, 1
  %exitcond = icmp eq i32 %i7, 2000
  br i1 %exitcond, label %._crit_edge, label %bb

._crit_edge:                                      ; preds = %bb
  %i8 = mul nsw i32 %.x.1, %i3
  %i9 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %i
  store i32 %i8, i32 addrspace(1)* %i9, align 4
  %i10 = shl i32 %i, 2
  %i11 = add nsw i32 %i10, 12
  %i12 = mul nsw i32 %i8, %i11
  %i13 = add nsw i32 %i, 10
  %i14 = getelementptr inbounds i32, i32 addrspace(1)* %out, i32 %i13
  store i32 %i12, i32 addrspace(1)* %i14, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

attributes #0 = { nounwind }

!sycl.kernels = !{!0}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @test_mul, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_mul_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
