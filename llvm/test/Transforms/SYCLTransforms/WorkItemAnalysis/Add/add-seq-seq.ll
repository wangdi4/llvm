; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;test_add(global int *in, global int *out) {
;  int gid = get_global_id(0);
;  int x = gid;
;  int y = gid + 10;
;  int z;
;
;  if (y > 11) {
;    z = x + y;
;  } else {
;    z = 4 * y;
;  }
;
;  out[gid] = z - 1;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_add:
; CHECK-NEXT: SEQ   %1 = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: SEQ   %2 = add nsw i32 %1, 10
; CHECK-NEXT: RND   %3 = icmp sgt i32 %2, 11
; CHECK-NEXT: STR   %4 = add nsw i32 %2, %1
; CHECK-NEXT: STR   %5 = shl i32 %2, 2
; CHECK-NEXT: RND   %z.0 = select i1 %3, i32 %4, i32 %5
; CHECK-NEXT: RND   %6 = add nsw i32 %z.0, -1
; CHECK-NEXT: PTR   %7 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
; CHECK-NEXT: RND   store i32 %6, ptr addrspace(1) %7, align 4
; CHECK-NEXT: UNI   ret void

define void @test_add(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %2 = add nsw i32 %1, 10
  %3 = icmp sgt i32 %2, 11
  %4 = add nsw i32 %2, %1
  %5 = shl i32 %2, 2
  %z.0 = select i1 %3, i32 %4, i32 %5
  %6 = add nsw i32 %z.0, -1
  %7 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
  store i32 %6, ptr addrspace(1) %7, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_add, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_add_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}
