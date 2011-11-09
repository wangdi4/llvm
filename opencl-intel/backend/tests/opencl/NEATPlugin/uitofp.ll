; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN: NEATChecker -r %s -a %s.neat -t 0

;CHECKNEAT: ACCURATE 51 ACCURATE 52 ACCURATE 52 ACCURATE 4
;CHECKNEAT: ACCURATE 51 ACCURATE 52 ACCURATE 52 ACCURATE 4
;CHECKNEAT: ACCURATE 51 ACCURATE 52 ACCURATE 52 ACCURATE 4
;CHECKNEAT: ACCURATE 51 ACCURATE 52 ACCURATE 52 ACCURATE 4
;CHECKNEAT: ACCURATE 51 ACCURATE 52 ACCURATE 52 ACCURATE 4
; ModuleID = 'uitofp.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @UIToFPTest(<4 x i32> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x i32> addrspace(1)*, align 4
  %output.addr = alloca <4 x float> addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %.compoundliteral = alloca <4 x float>, align 16
  %z = alloca i32, align 4
  store <4 x i32> addrspace(1)* %input, <4 x i32> addrspace(1)** %input.addr, align 4
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0)
  store i32 %call, i32* %tid, align 4
  store <4 x float> <float 1.000000e+001, float 1.100000e+001, float 1.100000e+001, float 1.200000e+001>, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral
  %tmp1 = load i32* %tid, align 4
  %tmp2 = load <4 x i32> addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds <4 x i32> addrspace(1)* %tmp2, i32 %tmp1
  %tmp3 = load <4 x i32> addrspace(1)* %arrayidx
  %tmp4 = extractelement <4 x i32> %tmp3, i32 1
  %conv = uitofp i32 %tmp4 to float
  %tmp5 = insertelement <4 x float> undef, float %conv, i32 0
  %splat = shufflevector <4 x float> %tmp5, <4 x float> %tmp5, <4 x i32> zeroinitializer
  %add = fadd <4 x float> %tmp, %splat
  %tmp6 = load i32* %tid, align 4
  %tmp7 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx8 = getelementptr inbounds <4 x float> addrspace(1)* %tmp7, i32 %tmp6
  store <4 x float> %add, <4 x float> addrspace(1)* %arrayidx8
  store i32 4, i32* %z, align 4
  %tmp10 = load i32* %z, align 4
  %conv11 = uitofp i32 %tmp10 to float
  %tmp12 = load i32* %tid, align 4
  %tmp13 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx14 = getelementptr inbounds <4 x float> addrspace(1)* %tmp13, i32 %tmp12
  %tmp15 = load <4 x float> addrspace(1)* %arrayidx14
  %tmp16 = insertelement <4 x float> %tmp15, float %conv11, i32 3
  store <4 x float> %tmp16, <4 x float> addrspace(1)* %arrayidx14
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x i32> addrspace(1)*, <4 x float> addrspace(1)*, i32)* @UIToFPTest, metadata !1, metadata !1, metadata !"", metadata !"uint4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const", metadata !"opencl_UIToFPTest_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
