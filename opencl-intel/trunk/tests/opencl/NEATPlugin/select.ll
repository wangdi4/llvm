; RUN: python %S/../test_deploy.py %s.in .
; RUN: llvm-as %s -o select.ll.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1 -basedir=.
; TODO add NEATChecker -r %s -a select.ll.neat -t 0

;CHECKNEAT: ACCURATE 41 ACCURATE 41 ACCURATE 21 ACCURATE 41
;CHECKNEAT: ACCURATE 1 ACCURATE 41 ACCURATE 41 ACCURATE 61 
;CHECKNEAT: ACCURATE 11 ACCURATE 41 ACCURATE 40 ACCURATE 41
;CHECKNEAT: ACCURATE 21 ACCURATE 41 ACCURATE 41 ACCURATE 11
;CHECKNEAT: ACCURATE 31 ACCURATE 41 ACCURATE 71 ACCURATE 41

;CHECKNEAT: ACCURATE 41 ACCURATE 41 ACCURATE 21 ACCURATE 41
;CHECKNEAT: ACCURATE 1 ACCURATE 1 ACCURATE 1 ACCURATE 41 
;CHECKNEAT: ACCURATE 11 ACCURATE 11 ACCURATE 40 ACCURATE 41
;CHECKNEAT: ACCURATE 41 ACCURATE 41 ACCURATE 21 ACCURATE 11
;CHECKNEAT: ACCURATE 41 ACCURATE 41 ACCURATE 31 ACCURATE 41

; ModuleID = 'select.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define void @SelectTest(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %buffer_size) nounwind {
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
  %cmp = fcmp olt float %tmp3, 1.500000e+001
  %conv = zext i1 %cmp to i32
  %tmp4 = load i32* %tid, align 4
  %tmp5 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx6 = getelementptr inbounds <4 x float> addrspace(1)* %tmp5, i32 %tmp4
  %tmp7 = load <4 x float> addrspace(1)* %arrayidx6
  %tmp8 = shufflevector <4 x float> %tmp7, <4 x float> undef, <2 x i32> zeroinitializer
  %tmp9 = load i32* %tid, align 4
  %tmp10 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx11 = getelementptr inbounds <4 x float> addrspace(1)* %tmp10, i32 %tmp9
  %tmp12 = load <4 x float> addrspace(1)* %arrayidx11
  %tmp13 = shufflevector <4 x float> %tmp12, <4 x float> undef, <2 x i32> <i32 1, i32 1>
  %0 = icmp ne i32 %conv, 0
  %1 = select i1 %0, <2 x float> %tmp8, <2 x float> %tmp13
  %tmp14 = load i32* %tid, align 4
  %tmp15 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx16 = getelementptr inbounds <4 x float> addrspace(1)* %tmp15, i32 %tmp14
  %tmp17 = load <4 x float> addrspace(1)* %arrayidx16
  %tmp18 = shufflevector <2 x float> %1, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %tmp19 = shufflevector <4 x float> %tmp17, <4 x float> %tmp18, <4 x i32> <i32 4, i32 5, i32 2, i32 3>
  store <4 x float> %tmp19, <4 x float> addrspace(1)* %arrayidx16
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32)* @SelectTest, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const", metadata !"opencl_SelectTest_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
