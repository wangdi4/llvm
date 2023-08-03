; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;//
;kernel void
;test_add(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = 23;
;  int y = 1;
;  int i;
;  for (i = 0; i < 1000; i++) {
;    if (y > 12) {
;      out[gid] = x + y;
;      y = y + 1 + i;
;    } else {
;      out[gid] = x - 2 * y;
;      x = x - 4 - i;
;      y += 3;
;    }
;  }
;
;  out[gid] += x + y;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_add:
; CHECK-NEXT: SEQ   %0 = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: PTR   %1 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %0
; CHECK-NEXT: UNI   br label %2
; CHECK-NEXT: UNI   %i.03 = phi i32 [ 0, %bb.nph ], [ %.pre-phi, %9 ]
; CHECK-NEXT: UNI   %y.12 = phi i32 [ 1, %bb.nph ], [ %y.0, %9 ]
; CHECK-NEXT: UNI   %x.11 = phi i32 [ 23, %bb.nph ], [ %x.0, %9 ]
; CHECK-NEXT: UNI   %3 = icmp sgt i32 %y.12, 12
; CHECK-NEXT: UNI   br i1 %3, label %4, label %6
; CHECK-NEXT: UNI   %tmp7 = add i32 %i.03, 1
; CHECK-NEXT: UNI   %5 = add nsw i32 %y.12, %x.11
; CHECK-NEXT: RND   store i32 %5, ptr addrspace(1) %1, align 4
; CHECK-NEXT: UNI   br label %9
; CHECK-NEXT: UNI   %tmp5 = sub i32 -4, %i.03
; CHECK-NEXT: UNI   %7 = shl i32 %y.12, 1
; CHECK-NEXT: UNI   %8 = sub nsw i32 %x.11, %7
; CHECK-NEXT: RND   store i32 %8, ptr addrspace(1) %1, align 4
; CHECK-NEXT: UNI   %tmp6 = add i32 %x.11, %tmp5
; CHECK-NEXT: UNI   %.pre10 = add nsw i32 %i.03, 1
; CHECK-NEXT: UNI   br label %9
; CHECK-NEXT: UNI   %.pre-phi = phi i32 [ %tmp7, %4 ], [ %.pre10, %6 ]
; CHECK-NEXT: UNI   %10 = phi i32 [ %5, %4 ], [ %8, %6 ]
; CHECK-NEXT: UNI   %x.0 = phi i32 [ %x.11, %4 ], [ %tmp6, %6 ]
; CHECK-NEXT: UNI   %tmp7.pn = phi i32 [ %tmp7, %4 ], [ 3, %6 ]
; CHECK-NEXT: UNI   %y.0 = add i32 %y.12, %tmp7.pn
; CHECK-NEXT: UNI   %exitcond = icmp eq i32 %.pre-phi, 1000
; CHECK-NEXT: UNI   br i1 %exitcond, label %11, label %2
; CHECK-NEXT: PTR   %.pre11 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %0
; CHECK-NEXT: UNI   %12 = add nsw i32 %y.0, %x.0
; CHECK-NEXT: UNI   %13 = add nsw i32 %12, %10
; CHECK-NEXT: RND   store i32 %13, ptr addrspace(1) %.pre11, align 4
; CHECK-NEXT: UNI   ret void

define void @test_add(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
bb.nph:
  %0 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %1 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %0
  br label %2

; <label>:2                                       ; preds = %9, %bb.nph
  %i.03 = phi i32 [ 0, %bb.nph ], [ %.pre-phi, %9 ]
  %y.12 = phi i32 [ 1, %bb.nph ], [ %y.0, %9 ]
  %x.11 = phi i32 [ 23, %bb.nph ], [ %x.0, %9 ]
  %3 = icmp sgt i32 %y.12, 12
  br i1 %3, label %4, label %6

; <label>:4                                       ; preds = %2
  %tmp7 = add i32 %i.03, 1
  %5 = add nsw i32 %y.12, %x.11
  store i32 %5, ptr addrspace(1) %1, align 4
  br label %9

; <label>:6                                       ; preds = %2
  %tmp5 = sub i32 -4, %i.03
  %7 = shl i32 %y.12, 1
  %8 = sub nsw i32 %x.11, %7
  store i32 %8, ptr addrspace(1) %1, align 4
  %tmp6 = add i32 %x.11, %tmp5
  %.pre10 = add nsw i32 %i.03, 1
  br label %9

; <label>:9                                       ; preds = %4, %6
  %.pre-phi = phi i32 [ %tmp7, %4 ], [ %.pre10, %6 ]
  %10 = phi i32 [ %5, %4 ], [ %8, %6 ]
  %x.0 = phi i32 [ %x.11, %4 ], [ %tmp6, %6 ]
  %tmp7.pn = phi i32 [ %tmp7, %4 ], [ 3, %6 ]
  %y.0 = add i32 %y.12, %tmp7.pn
  %exitcond = icmp eq i32 %.pre-phi, 1000
  br i1 %exitcond, label %11, label %2

; <label>:11                                      ; preds = %9
  %.pre11 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %0
  %12 = add nsw i32 %y.0, %x.0
  %13 = add nsw i32 %12, %10
  store i32 %13, ptr addrspace(1) %.pre11, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_add, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_add_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}
