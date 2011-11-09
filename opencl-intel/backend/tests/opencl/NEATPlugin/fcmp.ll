; RUN: llvm-as %s -o %s.bin
; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; RUN: NEATChecker -r %s -a %s.neat -t 0

; ModuleID = 'fcmp.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

;CHECKNEAT: ACCURATE 1 ACCURATE 10 ACCURATE 41 ACCURATE 41 
;CHECKNEAT: ACCURATE 11 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 21 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 30 ACCURATE 41 ACCURATE 40 ACCURATE 41
;CHECKNEAT: ACCURATE 31 ACCURATE 41 ACCURATE 40 ACCURATE 41
;CHECKNEAT: ACCURATE 41 ACCURATE 41 ACCURATE 40 ACCURATE 11
;CHECKNEAT: ACCURATE 31 ACCURATE 41 ACCURATE 40 ACCURATE 11

define void @fcmp(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %buffer_size) nounwind {
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
  %tmp3 = extractelement <4 x float> %tmp2, i32 0
  %tmp4 = load i32* %tid, align 4
  %tmp5 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx6 = getelementptr inbounds <4 x float> addrspace(1)* %tmp5, i32 %tmp4
  %tmp7 = load <4 x float> addrspace(1)* %arrayidx6
  %tmp8 = insertelement <4 x float> %tmp7, float %tmp3, i32 3
  store <4 x float> %tmp8, <4 x float> addrspace(1)* %arrayidx6
  %tmp9 = load i32* %tid, align 4
  %tmp10 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx11 = getelementptr inbounds <4 x float> addrspace(1)* %tmp10, i32 %tmp9
  %tmp12 = load <4 x float> addrspace(1)* %arrayidx11
  %tmp13 = extractelement <4 x float> %tmp12, i32 0
  %cmp = fcmp olt float %tmp13, 2.000000e+001
  br i1 %cmp, label %if.then, label %if.else36

if.then:                                          ; preds = %entry
  %tmp14 = load i32* %tid, align 4
  %tmp15 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx16 = getelementptr inbounds <4 x float> addrspace(1)* %tmp15, i32 %tmp14
  %tmp17 = load <4 x float> addrspace(1)* %arrayidx16
  %tmp18 = extractelement <4 x float> %tmp17, i32 1
  %cmp19 = fcmp ole float %tmp18, 1.000000e+001
  br i1 %cmp19, label %if.then20, label %if.else

if.then20:                                        ; preds = %if.then
  %tmp21 = load i32* %tid, align 4
  %tmp22 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx23 = getelementptr inbounds <4 x float> addrspace(1)* %tmp22, i32 %tmp21
  %tmp24 = load <4 x float> addrspace(1)* %arrayidx23
  %tmp25 = extractelement <4 x float> %tmp24, i32 0
  %add = fadd float %tmp25, 5.000000e+000
  %tmp26 = load i32* %tid, align 4
  %tmp27 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx28 = getelementptr inbounds <4 x float> addrspace(1)* %tmp27, i32 %tmp26
  %tmp29 = load <4 x float> addrspace(1)* %arrayidx28
  %tmp30 = insertelement <4 x float> %tmp29, float %add, i32 1
  store <4 x float> %tmp30, <4 x float> addrspace(1)* %arrayidx28
  br label %if.end

if.else:                                          ; preds = %if.then
  %tmp31 = load i32* %tid, align 4
  %tmp32 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx33 = getelementptr inbounds <4 x float> addrspace(1)* %tmp32, i32 %tmp31
  %tmp34 = load <4 x float> addrspace(1)* %arrayidx33
  %tmp35 = insertelement <4 x float> %tmp34, float 1.000000e+002, i32 0
  store <4 x float> %tmp35, <4 x float> addrspace(1)* %arrayidx33
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then20
  br label %if.end98

if.else36:                                        ; preds = %entry
  %tmp37 = load i32* %tid, align 4
  %tmp38 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx39 = getelementptr inbounds <4 x float> addrspace(1)* %tmp38, i32 %tmp37
  %tmp40 = load <4 x float> addrspace(1)* %arrayidx39
  %tmp41 = extractelement <4 x float> %tmp40, i32 2
  %cmp42 = fcmp oeq float %tmp41, 4.100000e+001
  br i1 %cmp42, label %if.then43, label %if.else49

if.then43:                                        ; preds = %if.else36
  %tmp44 = load i32* %tid, align 4
  %tmp45 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx46 = getelementptr inbounds <4 x float> addrspace(1)* %tmp45, i32 %tmp44
  %tmp47 = load <4 x float> addrspace(1)* %arrayidx46
  %tmp48 = insertelement <4 x float> %tmp47, float 1.000000e+000, i32 2
  store <4 x float> %tmp48, <4 x float> addrspace(1)* %arrayidx46
  br label %if.end97

if.else49:                                        ; preds = %if.else36
  %tmp50 = load i32* %tid, align 4
  %tmp51 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx52 = getelementptr inbounds <4 x float> addrspace(1)* %tmp51, i32 %tmp50
  %tmp53 = load <4 x float> addrspace(1)* %arrayidx52
  %tmp54 = extractelement <4 x float> %tmp53, i32 3
  %cmp55 = fcmp ogt float %tmp54, 2.000000e+001
  br i1 %cmp55, label %if.then56, label %if.else76

if.then56:                                        ; preds = %if.else49
  %tmp57 = load i32* %tid, align 4
  %tmp58 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx59 = getelementptr inbounds <4 x float> addrspace(1)* %tmp58, i32 %tmp57
  %tmp60 = load <4 x float> addrspace(1)* %arrayidx59
  %tmp61 = extractelement <4 x float> %tmp60, i32 0
  %cmp62 = fcmp une float %tmp61, 3.000000e+001
  br i1 %cmp62, label %if.then63, label %if.else69

if.then63:                                        ; preds = %if.then56
  %tmp64 = load i32* %tid, align 4
  %tmp65 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx66 = getelementptr inbounds <4 x float> addrspace(1)* %tmp65, i32 %tmp64
  %tmp67 = load <4 x float> addrspace(1)* %arrayidx66
  %tmp68 = insertelement <4 x float> %tmp67, float 3.000000e+001, i32 0
  store <4 x float> %tmp68, <4 x float> addrspace(1)* %arrayidx66
  br label %if.end75

if.else69:                                        ; preds = %if.then56
  %tmp70 = load i32* %tid, align 4
  %tmp71 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx72 = getelementptr inbounds <4 x float> addrspace(1)* %tmp71, i32 %tmp70
  %tmp73 = load <4 x float> addrspace(1)* %arrayidx72
  %tmp74 = insertelement <4 x float> %tmp73, float 4.000000e+001, i32 1
  store <4 x float> %tmp74, <4 x float> addrspace(1)* %arrayidx72
  br label %if.end75

if.end75:                                         ; preds = %if.else69, %if.then63
  br label %if.end96

if.else76:                                        ; preds = %if.else49
  %tmp77 = load i32* %tid, align 4
  %tmp78 = load <4 x float> addrspace(1)** %input.addr, align 4
  %arrayidx79 = getelementptr inbounds <4 x float> addrspace(1)* %tmp78, i32 %tmp77
  %tmp80 = load <4 x float> addrspace(1)* %arrayidx79
  %tmp81 = extractelement <4 x float> %tmp80, i32 0
  %cmp82 = fcmp oge float %tmp81, 4.100000e+001
  br i1 %cmp82, label %if.then83, label %if.else89

if.then83:                                        ; preds = %if.else76
  %tmp84 = load i32* %tid, align 4
  %tmp85 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx86 = getelementptr inbounds <4 x float> addrspace(1)* %tmp85, i32 %tmp84
  %tmp87 = load <4 x float> addrspace(1)* %arrayidx86
  %tmp88 = insertelement <4 x float> %tmp87, float 4.500000e+001, i32 0
  store <4 x float> %tmp88, <4 x float> addrspace(1)* %arrayidx86
  br label %if.end95

if.else89:                                        ; preds = %if.else76
  %tmp90 = load i32* %tid, align 4
  %tmp91 = load <4 x float> addrspace(1)** %output.addr, align 4
  %arrayidx92 = getelementptr inbounds <4 x float> addrspace(1)* %tmp91, i32 %tmp90
  %tmp93 = load <4 x float> addrspace(1)* %arrayidx92
  %tmp94 = insertelement <4 x float> %tmp93, float 4.400000e+001, i32 0
  store <4 x float> %tmp94, <4 x float> addrspace(1)* %arrayidx92
  br label %if.end95

if.end95:                                         ; preds = %if.else89, %if.then83
  br label %if.end96

if.end96:                                         ; preds = %if.end95, %if.end75
  br label %if.end97

if.end97:                                         ; preds = %if.end96, %if.then43
  br label %if.end98

if.end98:                                         ; preds = %if.end97, %if.end
  ret void
}

;CHECKNEAT: ACCURATE 0 ACCURATE 6 ACCURATE 41 ACCURATE 1
;CHECKNEAT: ACCURATE 100 ACCURATE 41 ACCURATE 41 ACCURATE 11
;CHECKNEAT: ACCURATE 0 ACCURATE 41 ACCURATE 1 ACCURATE 21 
;CHECKNEAT: ACCURATE 0 ACCURATE 40 ACCURATE 41 ACCURATE 30 
;CHECKNEAT: ACCURATE 30 ACCURATE 41 ACCURATE 41 ACCURATE 31
;CHECKNEAT: ACCURATE 45 ACCURATE 41 ACCURATE 41 ACCURATE 41
;CHECKNEAT: ACCURATE 44 ACCURATE 41 ACCURATE 41 ACCURATE 31

declare i32 @get_global_id(i32)

!opencl.kernels = !{!0}

!0 = metadata !{void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32)* @fcmp, metadata !1, metadata !1, metadata !"", metadata !"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const", metadata !"opencl_fcmp_locals_anchor"}
!1 = metadata !{i32 0, i32 0, i32 0}
