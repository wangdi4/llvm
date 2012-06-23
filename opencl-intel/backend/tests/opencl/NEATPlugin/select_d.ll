; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; TODO add NEATChecker -r %s -a %s.neat -t 0

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

; ModuleID = 'select_d.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @SelectTest(<4 x double> addrspace(1)* %input, <4 x double> addrspace(1)* %output, i32 %buffer_size) nounwind {
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
  %cmp = fcmp olt double %3, 1.500000e+01
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %4 = load i32* %tid, align 4
  %5 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx1 = getelementptr inbounds <4 x double> addrspace(1)* %5, i32 %4
  %6 = load <4 x double> addrspace(1)* %arrayidx1
  %7 = shufflevector <4 x double> %6, <4 x double> undef, <2 x i32> zeroinitializer
  br label %cond.end

cond.false:                                       ; preds = %entry
  %8 = load i32* %tid, align 4
  %9 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx2 = getelementptr inbounds <4 x double> addrspace(1)* %9, i32 %8
  %10 = load <4 x double> addrspace(1)* %arrayidx2
  %11 = shufflevector <4 x double> %10, <4 x double> undef, <2 x i32> <i32 1, i32 1>
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi <2 x double> [ %7, %cond.true ], [ %11, %cond.false ]
  %12 = load i32* %tid, align 4
  %13 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx3 = getelementptr inbounds <4 x double> addrspace(1)* %13, i32 %12
  %14 = load <4 x double> addrspace(1)* %arrayidx3
  %15 = shufflevector <2 x double> %cond, <2 x double> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %16 = shufflevector <4 x double> %14, <4 x double> %15, <4 x i32> <i32 4, i32 5, i32 2, i32 3>
  store <4 x double> %16, <4 x double> addrspace(1)* %arrayidx3
  ret void
}

declare i32 @get_global_id(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (<4 x double> addrspace(1)*, <4 x double> addrspace(1)*, i32)* @SelectTest, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
