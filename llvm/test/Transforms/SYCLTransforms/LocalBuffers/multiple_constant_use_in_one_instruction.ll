; RUN: opt -S -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-local-buffers,check-debugify' %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -S -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers' %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

@a.__local = internal addrspace(3) global [3 x i8] undef, align 1
; CHECK-NOT: @a.__local = internal addrspace(3) global [3 x i8]

define void @kernel(ptr addrspace(1) %arrayidx) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  store i64 sub (i64 ptrtoint (ptr addrspace(3) getelementptr inbounds ([3 x i8], ptr addrspace(3) @a.__local, i64 0, i64 1) to i64), i64 ptrtoint (ptr addrspace(3) @a.__local to i64)), ptr addrspace(1) %arrayidx, align 8
  ret void
}

; CHECK: [[GEP0:%.*]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK-NEXT: [[GEP1:%.*]] = getelementptr inbounds [3 x i8], ptr addrspace(3) [[GEP0]], i64 0, i64 1
; CHECK-NEXT: [[PTRTOINT1:%.*]] = ptrtoint ptr addrspace(3) [[GEP1]] to i64
; CHECK-NEXT: [[PTRTOINT:%.*]] = ptrtoint ptr addrspace(3) [[GEP0]] to i64
; CHECK-NEXT: [[SUB:%.*]] = sub i64 [[PTRTOINT1]], [[PTRTOINT]]
; CHECK-NEXT: store i64 [[SUB]], ptr addrspace(1) %arrayidx, align 8

!sycl.kernels = !{!0}

!0 = !{ptr @kernel}
!1 = !{!"long*"}
!2 = !{ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
