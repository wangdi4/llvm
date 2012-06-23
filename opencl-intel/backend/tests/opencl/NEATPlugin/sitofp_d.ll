; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN: NEATChecker -r %s -a %s.neat -t 0

;CHECKNEAT: ACCURATE -31 ACCURATE -30 ACCURATE -30 ACCURATE -4
;CHECKNEAT: ACCURATE 11 ACCURATE 12 ACCURATE 12 ACCURATE -4
;CHECKNEAT: ACCURATE -1 ACCURATE 0 ACCURATE 0 ACCURATE -4
;CHECKNEAT: ACCURATE -11 ACCURATE -10 ACCURATE -10 ACCURATE -4
;CHECKNEAT: ACCURATE 10 ACCURATE 11 ACCURATE 11 ACCURATE -4

; ModuleID = 'sitofp_d.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @SIToFPTest(i64 addrspace(1)* %input, <4 x double> addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca i64 addrspace(1)*, align 4
  %output.addr = alloca <4 x double> addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %.compoundliteral = alloca <4 x double>, align 32
  %z = alloca i64, align 8
  store i64 addrspace(1)* %input, i64 addrspace(1)** %input.addr, align 4
  store <4 x double> addrspace(1)* %output, <4 x double> addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  store i32 %call, i32* %tid, align 4
  store <4 x double> <double 1.000000e+01, double 1.100000e+01, double 1.100000e+01, double 1.200000e+01>, <4 x double>* %.compoundliteral
  %0 = load <4 x double>* %.compoundliteral
  %1 = load i32* %tid, align 4
  %2 = load i64 addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds i64 addrspace(1)* %2, i32 %1
  %3 = load i64 addrspace(1)* %arrayidx
  %conv = sitofp i64 %3 to double
  %4 = insertelement <4 x double> undef, double %conv, i32 0
  %splat = shufflevector <4 x double> %4, <4 x double> %4, <4 x i32> zeroinitializer
  %add = fadd <4 x double> %0, %splat
  %5 = load i32* %tid, align 4
  %6 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx1 = getelementptr inbounds <4 x double> addrspace(1)* %6, i32 %5
  store <4 x double> %add, <4 x double> addrspace(1)* %arrayidx1
  store i64 -4, i64* %z, align 8
  %7 = load i64* %z, align 8
  %conv2 = sitofp i64 %7 to double
  %8 = load i32* %tid, align 4
  %9 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx3 = getelementptr inbounds <4 x double> addrspace(1)* %9, i32 %8
  %10 = load <4 x double> addrspace(1)* %arrayidx3
  %11 = insertelement <4 x double> %10, double %conv2, i32 3
  store <4 x double> %11, <4 x double> addrspace(1)* %arrayidx3
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (i64 addrspace(1)*, <4 x double> addrspace(1)*, i32)* @SIToFPTest, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
