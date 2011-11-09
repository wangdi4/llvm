; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN: NEATChecker -r %s -a %s.neat -t 0

; ModuleID = 'call.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define float @func(float addrspace(1)* %b) nounwind {
entry:
  %b.addr = alloca float addrspace(1)*, align 4
  %tid = alloca i32, align 4
  store float addrspace(1)* %b, float addrspace(1)** %b.addr, align 4
  %call = call i32 @get_global_id(i32 0)
  store i32 %call, i32* %tid, align 4
  %tmp = load i32* %tid, align 4
  %tmp1 = load float addrspace(1)** %b.addr, align 4
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp1, i32 %tmp
  store float 3.000000e+000, float addrspace(1)* %arrayidx
  ret float 3.000000e+000
}

declare i32 @get_global_id(i32)

define void @call(float addrspace(1)* %a) nounwind {
entry:
  %a.addr = alloca float addrspace(1)*, align 4
  store float addrspace(1)* %a, float addrspace(1)** %a.addr, align 4
  %tmp = load float addrspace(1)** %a.addr, align 4
  %call = call float @func(float addrspace(1)* %tmp)
  ret void
}
;CHECKNEAT: ACCURATE 3
;CHECKNEAT: ACCURATE 3
;CHECKNEAT: ACCURATE 3
;CHECKNEAT: ACCURATE 3
;CHECKNEAT: ACCURATE 3

!opencl.kernels = !{!0}

!0 = metadata !{void (float addrspace(1)*)* @call, metadata !1, metadata !1, metadata !"", metadata !"float __attribute__((address_space(1))) *", metadata !"opencl_call_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
