; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN: NEATChecker -r %s -a %s.neat -t 0

; ModuleID = 'fcmp_d.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

;CHECKNEAT: ACCURATE 1 ACCURATE 10 ACCURATE 41 ACCURATE 41 
;CHECKNEAT: ACCURATE 11 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 21 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 30 ACCURATE 41 ACCURATE 40 ACCURATE 41
;CHECKNEAT: ACCURATE 31 ACCURATE 41 ACCURATE 40 ACCURATE 41
;CHECKNEAT: ACCURATE 41 ACCURATE 41 ACCURATE 40 ACCURATE 11
;CHECKNEAT: ACCURATE 31 ACCURATE 41 ACCURATE 40 ACCURATE 11

define void @fcmp(<4 x double> addrspace(1)* %input, <4 x double> addrspace(1)* %output, i32 %buffer_size) nounwind {
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
  %4 = load i32* %tid, align 4
  %5 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx1 = getelementptr inbounds <4 x double> addrspace(1)* %5, i32 %4
  %6 = load <4 x double> addrspace(1)* %arrayidx1
  %7 = insertelement <4 x double> %6, double %3, i32 3
  store <4 x double> %7, <4 x double> addrspace(1)* %arrayidx1
  %8 = load i32* %tid, align 4
  %9 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx2 = getelementptr inbounds <4 x double> addrspace(1)* %9, i32 %8
  %10 = load <4 x double> addrspace(1)* %arrayidx2
  %11 = extractelement <4 x double> %10, i32 0
  %cmp = fcmp olt double %11, 2.000000e+01
  br i1 %cmp, label %if.then, label %if.else9

if.then:                                          ; preds = %entry
  %12 = load i32* %tid, align 4
  %13 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx3 = getelementptr inbounds <4 x double> addrspace(1)* %13, i32 %12
  %14 = load <4 x double> addrspace(1)* %arrayidx3
  %15 = extractelement <4 x double> %14, i32 1
  %cmp4 = fcmp ole double %15, 1.000000e+01
  br i1 %cmp4, label %if.then5, label %if.else

if.then5:                                         ; preds = %if.then
  %16 = load i32* %tid, align 4
  %17 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx6 = getelementptr inbounds <4 x double> addrspace(1)* %17, i32 %16
  %18 = load <4 x double> addrspace(1)* %arrayidx6
  %19 = extractelement <4 x double> %18, i32 0
  %add = fadd double %19, 5.000000e+00
  %20 = load i32* %tid, align 4
  %21 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx7 = getelementptr inbounds <4 x double> addrspace(1)* %21, i32 %20
  %22 = load <4 x double> addrspace(1)* %arrayidx7
  %23 = insertelement <4 x double> %22, double %add, i32 1
  store <4 x double> %23, <4 x double> addrspace(1)* %arrayidx7
  br label %if.end

if.else:                                          ; preds = %if.then
  %24 = load i32* %tid, align 4
  %25 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx8 = getelementptr inbounds <4 x double> addrspace(1)* %25, i32 %24
  %26 = load <4 x double> addrspace(1)* %arrayidx8
  %27 = insertelement <4 x double> %26, double 1.000000e+02, i32 0
  store <4 x double> %27, <4 x double> addrspace(1)* %arrayidx8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then5
  br label %if.end35

if.else9:                                         ; preds = %entry
  %28 = load i32* %tid, align 4
  %29 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx10 = getelementptr inbounds <4 x double> addrspace(1)* %29, i32 %28
  %30 = load <4 x double> addrspace(1)* %arrayidx10
  %31 = extractelement <4 x double> %30, i32 2
  %cmp11 = fcmp oeq double %31, 4.100000e+01
  br i1 %cmp11, label %if.then12, label %if.else14

if.then12:                                        ; preds = %if.else9
  %32 = load i32* %tid, align 4
  %33 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx13 = getelementptr inbounds <4 x double> addrspace(1)* %33, i32 %32
  %34 = load <4 x double> addrspace(1)* %arrayidx13
  %35 = insertelement <4 x double> %34, double 1.000000e+00, i32 2
  store <4 x double> %35, <4 x double> addrspace(1)* %arrayidx13
  br label %if.end34

if.else14:                                        ; preds = %if.else9
  %36 = load i32* %tid, align 4
  %37 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx15 = getelementptr inbounds <4 x double> addrspace(1)* %37, i32 %36
  %38 = load <4 x double> addrspace(1)* %arrayidx15
  %39 = extractelement <4 x double> %38, i32 3
  %cmp16 = fcmp ogt double %39, 2.000000e+01
  br i1 %cmp16, label %if.then17, label %if.else25

if.then17:                                        ; preds = %if.else14
  %40 = load i32* %tid, align 4
  %41 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx18 = getelementptr inbounds <4 x double> addrspace(1)* %41, i32 %40
  %42 = load <4 x double> addrspace(1)* %arrayidx18
  %43 = extractelement <4 x double> %42, i32 0
  %cmp19 = fcmp une double %43, 3.000000e+01
  br i1 %cmp19, label %if.then20, label %if.else22

if.then20:                                        ; preds = %if.then17
  %44 = load i32* %tid, align 4
  %45 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx21 = getelementptr inbounds <4 x double> addrspace(1)* %45, i32 %44
  %46 = load <4 x double> addrspace(1)* %arrayidx21
  %47 = insertelement <4 x double> %46, double 3.000000e+01, i32 0
  store <4 x double> %47, <4 x double> addrspace(1)* %arrayidx21
  br label %if.end24

if.else22:                                        ; preds = %if.then17
  %48 = load i32* %tid, align 4
  %49 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx23 = getelementptr inbounds <4 x double> addrspace(1)* %49, i32 %48
  %50 = load <4 x double> addrspace(1)* %arrayidx23
  %51 = insertelement <4 x double> %50, double 4.000000e+01, i32 1
  store <4 x double> %51, <4 x double> addrspace(1)* %arrayidx23
  br label %if.end24

if.end24:                                         ; preds = %if.else22, %if.then20
  br label %if.end33

if.else25:                                        ; preds = %if.else14
  %52 = load i32* %tid, align 4
  %53 = load <4 x double> addrspace(1)** %input.addr, align 4
  %arrayidx26 = getelementptr inbounds <4 x double> addrspace(1)* %53, i32 %52
  %54 = load <4 x double> addrspace(1)* %arrayidx26
  %55 = extractelement <4 x double> %54, i32 0
  %cmp27 = fcmp oge double %55, 4.100000e+01
  br i1 %cmp27, label %if.then28, label %if.else30

if.then28:                                        ; preds = %if.else25
  %56 = load i32* %tid, align 4
  %57 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx29 = getelementptr inbounds <4 x double> addrspace(1)* %57, i32 %56
  %58 = load <4 x double> addrspace(1)* %arrayidx29
  %59 = insertelement <4 x double> %58, double 4.500000e+01, i32 0
  store <4 x double> %59, <4 x double> addrspace(1)* %arrayidx29
  br label %if.end32

if.else30:                                        ; preds = %if.else25
  %60 = load i32* %tid, align 4
  %61 = load <4 x double> addrspace(1)** %output.addr, align 4
  %arrayidx31 = getelementptr inbounds <4 x double> addrspace(1)* %61, i32 %60
  %62 = load <4 x double> addrspace(1)* %arrayidx31
  %63 = insertelement <4 x double> %62, double 4.400000e+01, i32 0
  store <4 x double> %63, <4 x double> addrspace(1)* %arrayidx31
  br label %if.end32

if.end32:                                         ; preds = %if.else30, %if.then28
  br label %if.end33

if.end33:                                         ; preds = %if.end32, %if.end24
  br label %if.end34

if.end34:                                         ; preds = %if.end33, %if.then12
  br label %if.end35

if.end35:                                         ; preds = %if.end34, %if.end
  ret void
}

;CHECKNEAT: ACCURATE 0 ACCURATE 6 ACCURATE 41 ACCURATE 1
;CHECKNEAT: ACCURATE 100 ACCURATE 41 ACCURATE 41 ACCURATE 11
;CHECKNEAT: ACCURATE 0 ACCURATE 41 ACCURATE 1 ACCURATE 21 
;CHECKNEAT: ACCURATE 0 ACCURATE 40 ACCURATE 41 ACCURATE 30 
;CHECKNEAT: ACCURATE 30 ACCURATE 41 ACCURATE 41 ACCURATE 31
;CHECKNEAT: ACCURATE 45 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 44 ACCURATE 41 ACCURATE 41 ACCURATE 31

declare i32 @get_global_id(i32) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (<4 x double> addrspace(1)*, <4 x double> addrspace(1)*, i32)* @fcmp, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3, i32 3, i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
