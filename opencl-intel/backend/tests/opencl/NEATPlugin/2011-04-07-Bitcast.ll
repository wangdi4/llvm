; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; XFAIL:

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @BitcastPointer(float addrspace(1)* %input, float addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %f_vec = alloca <3 x float>, align 4
  %1 = bitcast <3 x float>* %f_vec to i8*

  ret void
}

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32)* @BitcastPointer, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint const", metadata !"opencl_bitcast_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
