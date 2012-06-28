; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN: NEATChecker -r %s -a %s.neat -t 0

;CHECKNEAT: UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN UNKNOWN
;CHECKNEAT: UNKNOWN
;CHECKNEAT: UNKNOWN
;CHECKNEAT: UNKNOWN
;CHECKNEAT: UNKNOWN
;CHECKNEAT: UNKNOWN

; ModuleID = 'bitcast_f2d.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @BitcastTest_f2d(<2 x float> addrspace(1)* %input, double addrspace(1)* %output, i32 %buffer_size) nounwind {
entry:
  %input.addr = alloca <2 x float> addrspace(1)*, align 4
  %output.addr = alloca double addrspace(1)*, align 4
  %buffer_size.addr = alloca i32, align 4
  %tid = alloca i32, align 4
  %tmp = alloca double, align 8
  store <2 x float> addrspace(1)* %input, <2 x float> addrspace(1)** %input.addr, align 4
  store double addrspace(1)* %output, double addrspace(1)** %output.addr, align 4
  store i32 %buffer_size, i32* %buffer_size.addr, align 4
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  store i32 %call, i32* %tid, align 4
  %0 = load i32* %tid, align 4
  %1 = load <2 x float> addrspace(1)** %input.addr, align 4
  %arrayidx = getelementptr inbounds <2 x float> addrspace(1)* %1, i32 %0
  %2 = load <2 x float> addrspace(1)* %arrayidx
  %astype = bitcast <2 x float> %2 to double
  %mul = fmul double %astype, 0.000000e+00
  %add = fadd double %mul, 0x3FC99999A0000000
  store double %add, double* %tmp, align 8
  %3 = load double* %tmp, align 8
  %astype1 = bitcast double %3 to <2 x float>
  %4 = load i32* %tid, align 4
  %5 = load <2 x float> addrspace(1)** %input.addr, align 4
  %arrayidx2 = getelementptr inbounds <2 x float> addrspace(1)* %5, i32 %4
  store <2 x float> %astype1, <2 x float> addrspace(1)* %arrayidx2
  %6 = load double* %tmp, align 8
  %7 = load i32* %tid, align 4
  %8 = load double addrspace(1)** %output.addr, align 4
  %arrayidx3 = getelementptr inbounds double addrspace(1)* %8, i32 %7
  store double %6, double addrspace(1)* %arrayidx3
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (<2 x float> addrspace(1)*, double addrspace(1)*, i32)* @BitcastTest_f2d, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
