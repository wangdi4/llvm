; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;__kernel void
;test_mul(__global int *in, __global int *out) {
;  int gid = get_global_id(0);
;  int x = in[gid];
;  int y = in[gid + 7];
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
;  z = z * (gid + 12);
;  out[gid + 10] = z;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_mul:
; CHECK-NEXT: SEQ   %i = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: PTR   %i1 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %i
; CHECK-NEXT: RND   %i2 = load i32, ptr addrspace(1) %i1, align 4
; CHECK-NEXT: SEQ   %i3 = add nsw i32 %i, 7
; CHECK-NEXT: PTR   %i4 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %i3
; CHECK-NEXT: RND   %i5 = load i32, ptr addrspace(1) %i4, align 4
; CHECK-NEXT: UNI   br label %bb
; CHECK-NEXT: UNI   %i6 = phi i32 [ 0, %bb.nph ], [ %i9, %bb ]
; CHECK-NEXT: RND   %x.11 = phi i32 [ %i2, %bb.nph ], [ %.x.1, %bb ]
; CHECK-NEXT: UNI   %i7 = icmp sgt i32 %i6, 300
; CHECK-NEXT: UNI   %i8 = zext i1 %i7 to i32
; CHECK-NEXT: RND   %.x.1 = shl i32 %x.11, %i8
; CHECK-NEXT: UNI   %i9 = add nsw i32 %i6, 1
; CHECK-NEXT: UNI   %exitcond = icmp eq i32 %i9, 2000
; CHECK-NEXT: UNI   br i1 %exitcond, label %._crit_edge, label %bb
; CHECK-NEXT: RND   %i10 = mul nsw i32 %.x.1, %i5
; CHECK-NEXT: PTR   %i11 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %i
; CHECK-NEXT: RND   store i32 %i10, ptr addrspace(1) %i11, align 4
; CHECK-NEXT: SEQ   %i12 = add nsw i32 %i, 12
; CHECK-NEXT: RND   %i13 = mul nsw i32 %i10, %i12
; CHECK-NEXT: SEQ   %i14 = add nsw i32 %i, 10
; CHECK-NEXT: PTR   %i15 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %i14
; CHECK-NEXT: RND   store i32 %i13, ptr addrspace(1) %i15, align 4
; CHECK-NEXT: UNI   ret void

; Function Attrs: nounwind
define void @test_mul(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) #0 !kernel_arg_base_type !2 !arg_type_null_val !3 {
bb.nph:
  %i = tail call i32 @_Z13get_global_idj(i32 0) #0
  %i1 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %i
  %i2 = load i32, ptr addrspace(1) %i1, align 4
  %i3 = add nsw i32 %i, 7
  %i4 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %i3
  %i5 = load i32, ptr addrspace(1) %i4, align 4
  br label %bb

bb:                                               ; preds = %bb, %bb.nph
  %i6 = phi i32 [ 0, %bb.nph ], [ %i9, %bb ]
  %x.11 = phi i32 [ %i2, %bb.nph ], [ %.x.1, %bb ]
  %i7 = icmp sgt i32 %i6, 300
  %i8 = zext i1 %i7 to i32
  %.x.1 = shl i32 %x.11, %i8
  %i9 = add nsw i32 %i6, 1
  %exitcond = icmp eq i32 %i9, 2000
  br i1 %exitcond, label %._crit_edge, label %bb

._crit_edge:                                      ; preds = %bb
  %i10 = mul nsw i32 %.x.1, %i5
  %i11 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %i
  store i32 %i10, ptr addrspace(1) %i11, align 4
  %i12 = add nsw i32 %i, 12
  %i13 = mul nsw i32 %i10, %i12
  %i14 = add nsw i32 %i, 10
  %i15 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %i14
  store i32 %i13, ptr addrspace(1) %i15, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

attributes #0 = { nounwind }

!sycl.kernels = !{!0}

!0 = !{ptr @test_mul, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_mul_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}
