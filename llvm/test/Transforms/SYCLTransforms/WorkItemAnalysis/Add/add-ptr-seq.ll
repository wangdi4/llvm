; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

;kernel void
;test_add(global int *in, global int *out, global int *p) {
;  int gid = get_global_id(0);
;  p = &in[gid] + gid;
;
;  if (p > 10) {
;    p = p + 633;
;  } else {
;    p = p + 111;
;  }
;
;  out[gid] = *p;
;}

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function test_add:
; CHECK-NEXT: SEQ   %1 = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: SEQ   %.sum = add i32 %1, 1
; CHECK-NEXT: PTR   %2 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %.sum
; CHECK-NEXT: RND   %3 = icmp ugt ptr addrspace(1) %2, inttoptr (i32 10 to ptr addrspace(1))
; CHECK-NEXT: RND   %.sum2.pn.v = select i1 %3, i32 633, i32 111
; CHECK-NEXT: RND   %.sum2.pn = add i32 %.sum2.pn.v, %.sum
; CHECK-NEXT: RND   %.0 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %.sum2.pn
; CHECK-NEXT: RND   %4 = load i32, ptr addrspace(1) %.0, align 4
; CHECK-NEXT: PTR   %5 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1

define void @test_add(ptr addrspace(1) %in, ptr addrspace(1) nocapture %out, ptr addrspace(1) nocapture %p) nounwind {
  %1 = tail call i32 @_Z13get_global_idj(i32 0) nounwind
  %.sum = add i32 %1, 1
  %2 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %.sum
  %3 = icmp ugt ptr addrspace(1) %2, inttoptr (i32 10 to ptr addrspace(1))
  %.sum2.pn.v = select i1 %3, i32 633, i32 111
  %.sum2.pn = add i32 %.sum2.pn.v, %.sum
  %.0 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %.sum2.pn
  %4 = load i32, ptr addrspace(1) %.0, align 4
  %5 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %1
  store i32 %4, ptr addrspace(1) %5, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!sycl.kernels = !{!0}

!0 = !{ptr @test_add, !1, !1, !"", !"int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *, int __attribute__((address_space(1))) *", !"opencl_test_add_locals_anchor"}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{!"int*", !"int*", !"int*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null}
