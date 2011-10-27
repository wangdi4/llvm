; RUN: python %S/../test_deploy.py %s.in .
; RUN: llvm-as %s -o alloca.ll.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1 -basedir=.
; RUN: NEATChecker -r %s -a alloca.ll.neat -t 0

; ModuleID = 'alloca.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%struct.str1 = type { i32, float, double, <3 x i32>, <4 x float>, [10 x <8 x double>] }

;CHECKNEAT: ACCURATE 41.00
;CHECKNEAT: ACCURATE 1 
;CHECKNEAT: ACCURATE 11
;CHECKNEAT: ACCURATE 21
;CHECKNEAT: ACCURATE 31

define void @allocaTest(float addrspace(1)* %input, float addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca float addrspace(1)*, align 4
  %output.addr = alloca float addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tmp = alloca float, align 4
  %tmpint = alloca i32, align 4
  %tmpdouble = alloca double, align 8
  %tmpint2 = alloca <2 x i32>, align 8
  %tmpdouble4 = alloca <4 x double>, align 32
  %tmpstruct = alloca %struct.str1, align 64
  store float addrspace(1)* %input, float addrspace(1)** %input.addr, align 4
  store float addrspace(1)* %output, float addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0)
  %tmp1 = load float addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp1, i32 %call
  %tmp2 = load float addrspace(1)* %arrayidx
  store float %tmp2, float* %tmp, align 4
  %tmp8 = load float* %tmp, align 4
  %call9 = call i32 @get_global_id(i32 0)
  %tmp10 = load float addrspace(1)** %output.addr, align 4
  %arrayidx11 = getelementptr inbounds float addrspace(1)* %tmp10, i32 %call9
  store float %tmp8, float addrspace(1)* %arrayidx11
  ret void
}

;CHECKNEAT: ACCURATE 41.00
;CHECKNEAT: ACCURATE 1 
;CHECKNEAT: ACCURATE 11
;CHECKNEAT: ACCURATE 21
;CHECKNEAT: ACCURATE 31

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*, float addrspace(1)*, i32)* @allocaTest, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, uint const", metadata !"opencl_allocaTest_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
