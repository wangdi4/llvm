; RUN: python %S/../test_deploy.py %s.in .
; RUN: llvm-as %s -o return.ll.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1 -basedir=.
; RUN: NEATChecker -r %s -a return.ll.neat -t 0

; ModuleID = 'return.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

;CHECKNEAT: ACCURATE 41
;CHECKNEAT: ACCURATE 1 
;CHECKNEAT: ACCURATE 11
;CHECKNEAT: ACCURATE 21
;CHECKNEAT: ACCURATE 31
define float @retFunc(float %a, float %b) nounwind {
entry:
  %a.addr = alloca float, align 4
  %b.addr = alloca float, align 4
  store float %a, float* %a.addr, align 4
  store float %b, float* %b.addr, align 4
  %tmp = load float* %a.addr, align 4
  %tmp1 = load float* %b.addr, align 4
  %add = fadd float %tmp, %tmp1
  ret float %add
}

define void @returnTest(float addrspace(1)* %input, float addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca float addrspace(1)*, align 4
  %output.addr = alloca float addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  store float addrspace(1)* %input, float addrspace(1)** %input.addr, align 4
  store float addrspace(1)* %output, float addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0)
  %tmp = load float addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp, i32 %call
  %tmp1 = load float addrspace(1)* %arrayidx
  %call2 = call i32 @get_global_id(i32 0)
  %tmp3 = load float addrspace(1)** %input.addr, align 4
  %arrayidx4 = getelementptr inbounds float addrspace(1)* %tmp3, i32 %call2
  %tmp5 = load float addrspace(1)* %arrayidx4
  %call6 = call float @retFunc(float %tmp1, float %tmp5)
  %call7 = call i32 @get_global_id(i32 0)
  %tmp8 = load float addrspace(1)** %output.addr, align 4
  %arrayidx9 = getelementptr inbounds float addrspace(1)* %tmp8, i32 %call7
  store float %call6, float addrspace(1)* %arrayidx9
  ret void
}

;CHECKNEAT: ACCURATE 82
;CHECKNEAT: ACCURATE 2 
;CHECKNEAT: ACCURATE 22
;CHECKNEAT: ACCURATE 42
;CHECKNEAT: ACCURATE 62
declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32)* @returnTest, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint const", metadata !"opencl_returnTest_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
