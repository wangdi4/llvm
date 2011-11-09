; RUN: llvm-as %s -o %s.bin
; RUN111: SATest -OCL -REF -config=%s.cfg -neat=1
; TODO: add NEATChecker -r %s -a %s.neat -t 0
; TODO: Rewrite test to eliminate pointer bitcast instruction which is not supported by NEAT.

; ModuleID = 'insert-extractvalue.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%0 = type { <4 x float>, i32, [12 x i8] }
%struct._str1 = type { <4 x float>, i32 }

@InsertExtractValue.s = internal constant %0 { <4 x float> <float 1.000000e+000, float 2.000000e+000, float 3.000000e+000, float 4.000000e+000>, i32 0, [12 x i8] undef }, align 16

define void @InsertExtractValue(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4
  %output.addr = alloca <4 x float> addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %s = alloca %struct._str1, align 16
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr, align 4
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0)
  store i32 %call, i32* %tid, align 4
  %tmp = bitcast %struct._str1* %s to i8*
  call void @llvm.memcpy.p0i8.p0i8.i32(i8* %tmp, i8* bitcast (%0* @InsertExtractValue.s to i8*), i32 32, i32 16, i1 false)
  %tmp1 = getelementptr inbounds %struct._str1* %s, i32 0, i32 0
  %tmp2 = load <4 x float>* %tmp1, align 16
  %tmp3 = load i32* %tid, align 4
  %tmp4 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp4, i32 %tmp3
  store <4 x float> %tmp2, <4 x float> addrspace(1)* %arrayidx
  ret void
}

declare i32 @get_global_id(i32)

declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture, i32, i32, i1) nounwind

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32)* @InsertExtractValue, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const", metadata !"opencl_InsertExtractValue_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
