; RUN: python %S/../test_deploy.py %s.in .
; RUN: llvm-as %s -o insert-extractelement.ll.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1 -basedir=.
; RUN: NEATChecker -r %s -a insert-extractelement.ll.neat -t 0

; ModuleID = 'insert-extractelement.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

;CHECKNEAT: ACCURATE 41 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 1 ACCURATE 41 ACCURATE 41 ACCURATE 41 
;CHECKNEAT: ACCURATE 11 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 21 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 31 ACCURATE 41 ACCURATE 41 ACCURATE 41
define void @InsertExtractElement(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4
  %output.addr = alloca <4 x float> addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr, align 4
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0)
  store i32 %call, i32* %tid, align 4
  %tmp = load i32* %tid, align 4
  %tmp1 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp1, i32 %tmp
  %tmp2 = load <4 x float> addrspace(1)* %arrayidx
  %tmp3 = extractelement <4 x float> %tmp2, i32 0
  %add = fadd float %tmp3, 5.000000e+000
  %tmp4 = load i32* %tid, align 4
  %tmp5 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx6 = getelementptr inbounds <4 x float> addrspace(1)* %tmp5, i32 %tmp4
  %tmp7 = load <4 x float> addrspace(1)* %arrayidx6
  %tmp8 = insertelement <4 x float> %tmp7, float %add, i32 0
  store <4 x float> %tmp8, <4 x float> addrspace(1)* %arrayidx6
  %tmp9 = load i32* %tid, align 4
  %tmp10 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx11 = getelementptr inbounds <4 x float> addrspace(1)* %tmp10, i32 %tmp9
  %tmp12 = load <4 x float> addrspace(1)* %arrayidx11
  %tmp13 = extractelement <4 x float> %tmp12, i32 0
  %add14 = fadd float %tmp13, 5.000000e+000
  %tmp15 = load i32* %tid, align 4
  %tmp16 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx17 = getelementptr inbounds <4 x float> addrspace(1)* %tmp16, i32 %tmp15
  %tmp18 = load <4 x float> addrspace(1)* %arrayidx17
  %tmp19 = insertelement <4 x float> %tmp18, float %add14, i32 1
  store <4 x float> %tmp19, <4 x float> addrspace(1)* %arrayidx17
  %tmp20 = load i32* %tid, align 4
  %tmp21 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx22 = getelementptr inbounds <4 x float> addrspace(1)* %tmp21, i32 %tmp20
  %tmp23 = load <4 x float> addrspace(1)* %arrayidx22
  %tmp24 = extractelement <4 x float> %tmp23, i32 0
  %add25 = fadd float %tmp24, 5.000000e+000
  %tmp26 = load i32* %tid, align 4
  %tmp27 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx28 = getelementptr inbounds <4 x float> addrspace(1)* %tmp27, i32 %tmp26
  %tmp29 = load <4 x float> addrspace(1)* %arrayidx28
  %tmp30 = insertelement <4 x float> %tmp29, float %add25, i32 2
  store <4 x float> %tmp30, <4 x float> addrspace(1)* %arrayidx28
  %tmp31 = load i32* %tid, align 4
  %tmp32 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx33 = getelementptr inbounds <4 x float> addrspace(1)* %tmp32, i32 %tmp31
  %tmp34 = load <4 x float> addrspace(1)* %arrayidx33
  %tmp35 = extractelement <4 x float> %tmp34, i32 0
  %add36 = fadd float %tmp35, 5.000000e+000
  %tmp37 = load i32* %tid, align 4
  %tmp38 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx39 = getelementptr inbounds <4 x float> addrspace(1)* %tmp38, i32 %tmp37
  %tmp40 = load <4 x float> addrspace(1)* %arrayidx39
  %tmp41 = insertelement <4 x float> %tmp40, float %add36, i32 3
  store <4 x float> %tmp41, <4 x float> addrspace(1)* %arrayidx39
  ret void
}

;CHECKNEAT: ACCURATE 46 ACCURATE 46 ACCURATE 46 ACCURATE 46
;CHECKNEAT: ACCURATE 6 ACCURATE 6 ACCURATE 6 ACCURATE 6
;CHECKNEAT: ACCURATE 16 ACCURATE 16 ACCURATE 16 ACCURATE 16
;CHECKNEAT: ACCURATE 26 ACCURATE 26 ACCURATE 26 ACCURATE 26
;CHECKNEAT: ACCURATE 36 ACCURATE 36 ACCURATE 36 ACCURATE 36
declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32)* @InsertExtractElement, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const", metadata !"opencl_InsertExtractElement_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
