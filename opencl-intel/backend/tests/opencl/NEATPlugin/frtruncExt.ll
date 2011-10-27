; RUN: python %S/../test_deploy.py %s.in .
; RUN: llvm-as %s -o frtruncExt.ll.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1 -basedir=.
; RUN: NEATChecker -r %s -a frtruncExt.ll.neat -t 0

; ModuleID = 'frtruncExt.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

;CHECKNEAT: ACCURATE 1 ACCURATE 1 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 11 ACCURATE 11 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 21 ACCURATE 21 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 30 ACCURATE 30 ACCURATE 40 ACCURATE 41
;CHECKNEAT: ACCURATE 31 ACCURATE 31 ACCURATE 40 ACCURATE 41
;CHECKNEAT: ACCURATE 41 ACCURATE 41 ACCURATE 40 ACCURATE 11
;CHECKNEAT: ACCURATE 31 ACCURATE 31 ACCURATE 40 ACCURATE 11
define void @ftruncExt(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4
  %output.addr = alloca <4 x float> addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %ext = alloca double, align 8
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr, align 4
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0)
  store i32 %call, i32* %tid, align 4
  %tmp = load i32* %tid, align 4
  %tmp1 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp1, i32 %tmp
  %tmp2 = load <4 x float> addrspace(1)* %arrayidx
  %tmp3 = extractelement <4 x float> %tmp2, i32 1
  %conv = fpext float %tmp3 to double
  store double %conv, double* %ext, align 8
  %tmp4 = load double* %ext, align 8
  %add = fadd double %tmp4, 4.000000e+000
  %conv5 = fptrunc double %add to float
  %tmp6 = load i32* %tid, align 4
  %tmp7 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx8 = getelementptr inbounds <4 x float> addrspace(1)* %tmp7, i32 %tmp6
  %tmp9 = load <4 x float> addrspace(1)* %arrayidx8
  %tmp10 = insertelement <4 x float> %tmp9, float %conv5, i32 3
  store <4 x float> %tmp10, <4 x float> addrspace(1)* %arrayidx8
  ret void
}

;CHECKNEAT: ACCURATE 0 ACCURATE 41 ACCURATE 41 ACCURATE 5
;CHECKNEAT: ACCURATE 0 ACCURATE 41 ACCURATE 41 ACCURATE 15 
;CHECKNEAT: ACCURATE 0 ACCURATE 41 ACCURATE 41 ACCURATE 25 
;CHECKNEAT: ACCURATE 0 ACCURATE 41 ACCURATE 41 ACCURATE 34 
;CHECKNEAT: ACCURATE 0 ACCURATE 41 ACCURATE 41 ACCURATE 35 
;CHECKNEAT: ACCURATE 0 ACCURATE 41 ACCURATE 41 ACCURATE 45 
;CHECKNEAT: ACCURATE 0 ACCURATE 41 ACCURATE 41 ACCURATE 35 
declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32)* @ftruncExt, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const", metadata !"opencl_ftruncExt_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
