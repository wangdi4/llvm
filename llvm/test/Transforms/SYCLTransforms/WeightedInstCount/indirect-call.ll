; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,function(require<sycl-kernel-weighted-inst-count-analysis>)' %s -S

; check that WightedInstCounter doesn't crash on indirect calls.

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @foo(i32 %val, ptr addrspace(1) %ret) !kernel_arg_base_type !1 !arg_type_null_val !2 {
  %ptr = inttoptr i32 %val to ptr
  %call = call i32 %ptr(i32 %val)
  store i32 %call, ptr addrspace(1) %ret
  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @foo}
!1 = !{!"int", !"int*"}
!2 = !{i32 0, ptr addrspace(1) null}

