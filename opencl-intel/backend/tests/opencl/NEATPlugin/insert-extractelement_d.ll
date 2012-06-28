; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN: NEATChecker -r %s -a %s.neat -t 0

; ModuleID = 'insert-extractelement_d.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

;CHECKNEAT: ACCURATE 41 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 1 ACCURATE 41 ACCURATE 41 ACCURATE 41 
;CHECKNEAT: ACCURATE 11 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 21 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 31 ACCURATE 41 ACCURATE 41 ACCURATE 41

define void @InsertExtractElement(<4 x double> addrspace(1)* %input, <4 x double> addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <4 x double> addrspace(1)*, align 4
  %output.addr = alloca <4 x double> addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  store <4 x double> addrspace(1)* %input, <4 x double> addrspace(1)** %input.addr, align 4
  store <4 x double> addrspace(1)* %output, <4 x double> addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  store i32 %call, i32* %tid, align 4
  %0 = load i32* %tid, align 4
  %1 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds <4 x double> addrspace(1)* %1, i32 %0
  %2 = load <4 x double> addrspace(1)* %arrayidx
  %3 = extractelement <4 x double> %2, i32 0
  %add = fadd double %3, 5.000000e+00
  %4 = load i32* %tid, align 4
  %5 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx1 = getelementptr inbounds <4 x double> addrspace(1)* %5, i32 %4
  %6 = load <4 x double> addrspace(1)* %arrayidx1
  %7 = insertelement <4 x double> %6, double %add, i32 0
  store <4 x double> %7, <4 x double> addrspace(1)* %arrayidx1
  %8 = load i32* %tid, align 4
  %9 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx2 = getelementptr inbounds <4 x double> addrspace(1)* %9, i32 %8
  %10 = load <4 x double> addrspace(1)* %arrayidx2
  %11 = extractelement <4 x double> %10, i32 0
  %add3 = fadd double %11, 5.000000e+00
  %12 = load i32* %tid, align 4
  %13 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx4 = getelementptr inbounds <4 x double> addrspace(1)* %13, i32 %12
  %14 = load <4 x double> addrspace(1)* %arrayidx4
  %15 = insertelement <4 x double> %14, double %add3, i32 1
  store <4 x double> %15, <4 x double> addrspace(1)* %arrayidx4
  %16 = load i32* %tid, align 4
  %17 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx5 = getelementptr inbounds <4 x double> addrspace(1)* %17, i32 %16
  %18 = load <4 x double> addrspace(1)* %arrayidx5
  %19 = extractelement <4 x double> %18, i32 0
  %add6 = fadd double %19, 5.000000e+00
  %20 = load i32* %tid, align 4
  %21 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx7 = getelementptr inbounds <4 x double> addrspace(1)* %21, i32 %20
  %22 = load <4 x double> addrspace(1)* %arrayidx7
  %23 = insertelement <4 x double> %22, double %add6, i32 2
  store <4 x double> %23, <4 x double> addrspace(1)* %arrayidx7
  %24 = load i32* %tid, align 4
  %25 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx8 = getelementptr inbounds <4 x double> addrspace(1)* %25, i32 %24
  %26 = load <4 x double> addrspace(1)* %arrayidx8
  %27 = extractelement <4 x double> %26, i32 0
  %add9 = fadd double %27, 5.000000e+00
  %28 = load i32* %tid, align 4
  %29 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx10 = getelementptr inbounds <4 x double> addrspace(1)* %29, i32 %28
  %30 = load <4 x double> addrspace(1)* %arrayidx10
  %31 = insertelement <4 x double> %30, double %add9, i32 3
  store <4 x double> %31, <4 x double> addrspace(1)* %arrayidx10
  ret void
}

;CHECKNEAT: ACCURATE 46 ACCURATE 46 ACCURATE 46 ACCURATE 46
;CHECKNEAT: ACCURATE 6 ACCURATE 6 ACCURATE 6 ACCURATE 6
;CHECKNEAT: ACCURATE 16 ACCURATE 16 ACCURATE 16 ACCURATE 16
;CHECKNEAT: ACCURATE 26 ACCURATE 26 ACCURATE 26 ACCURATE 26
;CHECKNEAT: ACCURATE 36 ACCURATE 36 ACCURATE 36 ACCURATE 36

declare i32 @get_global_id(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (<4 x double> addrspace(1)*, <4 x double> addrspace(1)*, i32)* @InsertExtractElement, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
