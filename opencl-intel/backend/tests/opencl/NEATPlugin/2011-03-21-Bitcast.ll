; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1

; ModuleID = '2011-03-21-Bitcast.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @bitcast(float addrspace(1)* %input, float addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca float addrspace(1)*, align 4
  %output.addr = alloca float addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %f = alloca float, align 4
  store float addrspace(1)* %input, float addrspace(1)** %input.addr, align 4
  store float addrspace(1)* %output, float addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0)
  store i32 %call, i32* %tid, align 4
  %tmp = load i32* %tid, align 4
  %as_typen = bitcast i32 %tmp to float
  store float %as_typen, float* %f, align 4
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32)* @bitcast, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint const", metadata !"opencl_bitcast_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
