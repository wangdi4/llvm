; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-internalize-func,globaldce' -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK-NOT: @_NSConcreteGlobalBlock
; CHECK-NOT: external

@_NSConcreteGlobalBlock = external global ptr
@.str = private unnamed_addr constant [9 x i8] c"i12@?0i8\00", align 1
@__block_descriptor_tmp = internal constant { i64, i64, ptr, ptr } { i64 0, i64 32, ptr @.str, ptr null }

define spir_kernel void @block_typedef_reassign(ptr addrspace(1) %res) nounwind !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_type_qual !5 !kernel_arg_name !6 !arg_type_null_val !7 {
  ret void
}

!1 = !{i32 1}
!2 = !{!"none"}
!3 = !{!"int*"}
!5 = !{!""}
!6 = !{!"res"}
!7 = !{ptr addrspace(1) null}

; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing variable 1
; DEBUGIFY-NOT: WARNING
