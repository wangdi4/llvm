; RUN: llvm-as %s -o %t.bc
; RUN: opt -linear-id-resolver -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'get_local_linear_id'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; CHECK: @check_get_local_linear_id
; CHECK-NOT: @_Z19get_local_linear_idv
; CHECK: @_Z12get_local_idj(i32 0)
; CHECK: @_Z12get_local_idj(i32 1)
; CHECK: @_Z12get_local_idj(i32 2)
; CHECK: @_Z14get_local_sizej(i32 0)
; CHECK: @_Z14get_local_sizej(i32 1)
; CHECK: ret

define spir_kernel void @check_get_local_linear_id(float addrspace(1)* %arr1, float addrspace(1)* %arr2) nounwind {
entry:
  %arr1.addr = alloca float addrspace(1)*, align 8
  %arr2.addr = alloca float addrspace(1)*, align 8
  %id = alloca i32, align 4
  store float addrspace(1)* %arr1, float addrspace(1)** %arr1.addr, align 8
  store float addrspace(1)* %arr2, float addrspace(1)** %arr2.addr, align 8
  %call = call spir_func i64 @_Z19get_local_linear_idv()
  %conv = trunc i64 %call to i32
  store i32 %conv, i32* %id, align 4
  %0 = load i32* %id, align 4
  %idxprom = sext i32 %0 to i64
  %1 = load float addrspace(1)** %arr2.addr, align 8
  %arrayidx = getelementptr inbounds float addrspace(1)* %1, i64 %idxprom
  %2 = load float addrspace(1)* %arrayidx, align 4
  %3 = load i32* %id, align 4
  %idxprom1 = sext i32 %3 to i64
  %4 = load float addrspace(1)** %arr1.addr, align 8
  %arrayidx2 = getelementptr inbounds float addrspace(1)* %4, i64 %idxprom1
  store float %2, float addrspace(1)* %arrayidx2, align 4
  ret void
}

declare spir_func i64 @_Z19get_local_linear_idv()

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{}
!opencl.kernel_info = !{}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*)* @check_get_local_linear_id, metadata !1, metadata !2, metadata !3, metadata !4, metadata !5}
!1 = metadata !{metadata !"kernel_arg_addr_space", i32 1, i32 1}
!2 = metadata !{metadata !"kernel_arg_access_qual", metadata !"none", metadata !"none"}
!3 = metadata !{metadata !"kernel_arg_type", metadata !"float*", metadata !"float*"}
!4 = metadata !{metadata !"kernel_arg_type_qual", metadata !"", metadata !""}
!5 = metadata !{metadata !"kernel_arg_name", metadata !"arr1", metadata !"arr2"}
!6 = metadata !{i32 1, i32 0}
!7 = metadata !{i32 0, i32 0}
!8 = metadata !{}

