; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN: NEATChecker -r %s -a %s.neat -t 0

;CHECKNEAT: ACCURATE -31 ACCURATE -30 ACCURATE -30 ACCURATE -4
;CHECKNEAT: ACCURATE 11 ACCURATE 12 ACCURATE 12 ACCURATE -4
;CHECKNEAT: ACCURATE -1 ACCURATE 0 ACCURATE 0 ACCURATE -4
;CHECKNEAT: ACCURATE -11 ACCURATE -10 ACCURATE -10 ACCURATE -4
;CHECKNEAT: ACCURATE 10 ACCURATE 11 ACCURATE 11 ACCURATE -4
; ModuleID = 'sitofp.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @SIToFPTest(i32 addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca i32 addrspace(1)*, align 4
  %output.addr = alloca <4 x float> addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %.compoundliteral = alloca <4 x float>, align 16
  %z = alloca i32, align 4
  store i32 addrspace(1)* %input, i32 addrspace(1)** %input.addr, align 4
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0)
  store i32 %call, i32* %tid, align 4
  store <4 x float> <float 1.000000e+001, float 1.100000e+001, float 1.100000e+001, float 1.200000e+001>, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral
  %tmp1 = load i32* %tid, align 4
  %tmp2 = load i32 addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %tmp2, i32 %tmp1
  %tmp3 = load i32 addrspace(1)* %arrayidx
  %conv = sitofp i32 %tmp3 to float
  %tmp4 = insertelement <4 x float> undef, float %conv, i32 0
  %splat = shufflevector <4 x float> %tmp4, <4 x float> %tmp4, <4 x i32> zeroinitializer
  %add = fadd <4 x float> %tmp, %splat
  %tmp5 = load i32* %tid, align 4
  %tmp6 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx7 = getelementptr inbounds <4 x float> addrspace(1)* %tmp6, i32 %tmp5
  store <4 x float> %add, <4 x float> addrspace(1)* %arrayidx7
  store i32 -4, i32* %z, align 4
  %tmp9 = load i32* %z, align 4
  %conv10 = sitofp i32 %tmp9 to float
  %tmp11 = load i32* %tid, align 4
  %tmp12 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx13 = getelementptr inbounds <4 x float> addrspace(1)* %tmp12, i32 %tmp11
  %tmp14 = load <4 x float> addrspace(1)* %arrayidx13
  %tmp15 = insertelement <4 x float> %tmp14, float %conv10, i32 3
  store <4 x float> %tmp15, <4 x float> addrspace(1)* %arrayidx13
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (i32 addrspace(1)*, <4 x float> addrspace(1)*, i32)* @SIToFPTest, metadata !1, metadata !1, metadata !"", metadata !"int __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const", metadata !"opencl_SIToFPTest_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
