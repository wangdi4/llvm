; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;__kernel void
;test_mul(__global int *in, __global int *out) {
;  int gid = get_global_id(0);
;  int x = gid + 12; // seq
;  int y = gid;
;  int i = 0;
;  int z;
;
;  for (i = 0; i < 2000; i++) {
;    if (i > 300) {
;      x += i;
;    }
;  }
;
;  z = x * y;
;
;  out[gid] = z;
;
;  z = z * gid;
;  out[gid + 10] = z;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_mul:
; CHECK-NEXT: SEQ   %i = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: SEQ   %i1 = add nsw i32 %i, 12
; CHECK-NEXT: UNI   br label %bb
; CHECK-NEXT: UNI   %i2 = phi i32 [ 0, %bb.nph ], [ %i5, %bb ]
; CHECK-NEXT: SEQ   %x.11 = phi i32 [ %i1, %bb.nph ], [ %.x.1, %bb ]
; CHECK-NEXT: UNI   %i3 = icmp sgt i32 %i2, 300
; CHECK-NEXT: UNI   %i4 = select i1 %i3, i32 %i2, i32 0
; CHECK-NEXT: SEQ   %.x.1 = add i32 %i4, %x.11
; CHECK-NEXT: UNI   %i5 = add nsw i32 %i2, 1
; CHECK-NEXT: UNI   %exitcond = icmp eq i32 %i5, 2000
; CHECK-NEXT: UNI   br i1 %exitcond, label %._crit_edge, label %bb
; CHECK-NEXT: RND   %i6 = mul nsw i32 %.x.1, %i
; CHECK-NEXT: PTR   %i7 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %i
; CHECK-NEXT: RND   store i32 %i6, ptr addrspace(1) %i7, align 4
; CHECK-NEXT: RND   %i8 = mul nsw i32 %i6, %i
; CHECK-NEXT: SEQ   %i9 = add nsw i32 %i, 10
; CHECK-NEXT: PTR   %i10 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %i9
; CHECK-NEXT: RND   store i32 %i8, ptr addrspace(1) %i10, align 4
; CHECK-NEXT: UNI   ret void

; Function Attrs: nounwind
define void @test_mul(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) #0 !kernel_arg_base_type !2 !arg_type_null_val !3 {
bb.nph:
  %i = tail call i32 @_Z13get_global_idj(i32 0) #0
  %i1 = add nsw i32 %i, 12
  br label %bb

bb:                                               ; preds = %bb, %bb.nph
  %i2 = phi i32 [ 0, %bb.nph ], [ %i5, %bb ]
  %x.11 = phi i32 [ %i1, %bb.nph ], [ %.x.1, %bb ]
  %i3 = icmp sgt i32 %i2, 300
  %i4 = select i1 %i3, i32 %i2, i32 0
  %.x.1 = add i32 %i4, %x.11
  %i5 = add nsw i32 %i2, 1
  %exitcond = icmp eq i32 %i5, 2000
  br i1 %exitcond, label %._crit_edge, label %bb

._crit_edge:                                      ; preds = %bb
  %i6 = mul nsw i32 %.x.1, %i
  %i7 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %i
  store i32 %i6, ptr addrspace(1) %i7, align 4
  %i8 = mul nsw i32 %i6, %i
  %i9 = add nsw i32 %i, 10
  %i10 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %i9
  store i32 %i8, ptr addrspace(1) %i10, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

attributes #0 = { nounwind }

!sycl.kernels = !{!0}

!0 = !{ptr @test_mul, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_mul_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}
