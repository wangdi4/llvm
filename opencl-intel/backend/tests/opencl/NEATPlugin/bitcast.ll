; RUN: python %S/../test_deploy.py %s.in .
; RUN: llvm-as %s -o bitcast.ll.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1 -basedir=.
; RUN: NEATChecker -r %s -a bitcast.ll.neat -t 0

;CHECKNEAT: UNKNOWN UNKNOWN UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN

; ModuleID = 'bitcast.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"


define void @BitcastTest(<4 x float> addrspace(1)* %input, <2 x double> addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4
  %output.addr = alloca <2 x double> addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %tmp = alloca <2 x double>, align 16
  %.compoundliteral = alloca <2 x double>, align 16
  %.compoundliteral5 = alloca <2 x double>, align 16
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr, align 4
  store <2 x double> addrspace(1)* %output, <2 x double> addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0)
  store i32 %call, i32* %tid, align 4
  %tmp1 = load i32* %tid, align 4
  %tmp2 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp2, i32 %tmp1
  %tmp3 = load <4 x float> addrspace(1)* %arrayidx
  %as_typen = bitcast <4 x float> %tmp3 to <2 x double>
  store <2 x double> zeroinitializer, <2 x double>* %.compoundliteral
  %tmp4 = load <2 x double>* %.compoundliteral
  %mul = fmul <2 x double> %as_typen, %tmp4
  store <2 x double> <double 1.000000e-001, double 2.000000e-001>, <2 x double>* %.compoundliteral5
  %tmp6 = load <2 x double>* %.compoundliteral5
  %add = fadd <2 x double> %mul, %tmp6
  store <2 x double> %add, <2 x double>* %tmp, align 16
  %tmp7 = load <2 x double>* %tmp, align 16
  %as_typen8 = bitcast <2 x double> %tmp7 to <4 x float>
  %tmp9 = load i32* %tid, align 4
  %tmp10 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx11 = getelementptr inbounds <4 x float> addrspace(1)* %tmp10, i32 %tmp9
  store <4 x float> %as_typen8, <4 x float> addrspace(1)* %arrayidx11
  %tmp12 = load <2 x double>* %tmp, align 16
  %tmp13 = load i32* %tid, align 4
  %tmp14 = load <2 x double> addrspace(1)** %output.addr, align 4
  %arrayidx15 = getelementptr inbounds <2 x double> addrspace(1)* %tmp14, i32 %tmp13
  store <2 x double> %tmp12, <2 x double> addrspace(1)* %arrayidx15
  ret void
}

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <2 x double> addrspace(1)*, i32)* @BitcastTest, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, double2 __attribute__((address_space(1))) *, uint const", metadata !"opencl_BitcastTest_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
