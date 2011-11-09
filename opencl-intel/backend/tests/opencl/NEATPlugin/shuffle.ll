; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN: NEATChecker -r %s -a %s.neat -t 0

; ModuleID = 'shuffle.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

;CHECKNEAT: ACCURATE 41 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 1 ACCURATE 41 ACCURATE 41 ACCURATE 41 
;CHECKNEAT: ACCURATE 11 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 21 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 31 ACCURATE 41 ACCURATE 41 ACCURATE 41

define void @ShuffleTest(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %buffer_size) nounwind {
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
  %tmp3 = shufflevector <4 x float> %tmp2, <4 x float> undef, <4 x i32> zeroinitializer
  %tmp4 = load i32* %tid, align 4
  %tmp5 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx6 = getelementptr inbounds <4 x float> addrspace(1)* %tmp5, i32 %tmp4
  store <4 x float> %tmp3, <4 x float> addrspace(1)* %arrayidx6
  ret void
}

;CHECKNEAT: ACCURATE 41 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 1 ACCURATE 1 ACCURATE 1 ACCURATE 1
;CHECKNEAT: ACCURATE 11 ACCURATE 11 ACCURATE 11 ACCURATE 11
;CHECKNEAT: ACCURATE 21 ACCURATE 21 ACCURATE 21 ACCURATE 21
;CHECKNEAT: ACCURATE 31 ACCURATE 31 ACCURATE 31 ACCURATE 31
declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32)* @ShuffleTest, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const", metadata !"opencl_ShuffleTest_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
