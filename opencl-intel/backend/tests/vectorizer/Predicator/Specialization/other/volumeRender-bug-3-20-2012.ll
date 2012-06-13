; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -loop-simplify -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: @d_render
; CHECK: phi-split-bb
; CHECK: exit_mask_non_bypass
; CHECK: in_mask_non_bypass
; CHECK: if.end
; CHECK: exit_mask_non_bypass
; CHECK: in_mask_non_bypass
; CHECK: ret

define i32 @intersectBox(<4 x float> %r_o, <4 x float> %r_d, <4 x float> %boxmin, <4 x float> %boxmax, float* %tnear, float* %tfar) nounwind {
entry:
  %r_o.addr = alloca <4 x float>, align 16
  %r_d.addr = alloca <4 x float>, align 16
  %boxmin.addr = alloca <4 x float>, align 16
  %boxmax.addr = alloca <4 x float>, align 16
  %tnear.addr = alloca float*, align 4
  %tfar.addr = alloca float*, align 4
  %invR = alloca <4 x float>, align 16
  %.compoundliteral = alloca <4 x float>, align 16
  %tbot = alloca <4 x float>, align 16
  %ttop = alloca <4 x float>, align 16
  %tmin = alloca <4 x float>, align 16
  %tmax = alloca <4 x float>, align 16
  %largest_tmin = alloca float, align 4
  %smallest_tmax = alloca float, align 4
  store <4 x float> %r_o, <4 x float>* %r_o.addr, align 16
  store <4 x float> %r_d, <4 x float>* %r_d.addr, align 16
  store <4 x float> %boxmin, <4 x float>* %boxmin.addr, align 16
  store <4 x float> %boxmax, <4 x float>* %boxmax.addr, align 16
  store float* %tnear, float** %tnear.addr, align 4
  store float* %tfar, float** %tfar.addr, align 4
  store <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral
  %tmp1 = load <4 x float>* %r_d.addr, align 16
  %div = fdiv <4 x float> %tmp, %tmp1
  store <4 x float> %div, <4 x float>* %invR, align 16
  %tmp3 = load <4 x float>* %invR, align 16
  %tmp4 = load <4 x float>* %boxmin.addr, align 16
  %tmp5 = load <4 x float>* %r_o.addr, align 16
  %sub = fsub <4 x float> %tmp4, %tmp5
  %mul = fmul <4 x float> %tmp3, %sub
  store <4 x float> %mul, <4 x float>* %tbot, align 16
  %tmp7 = load <4 x float>* %invR, align 16
  %tmp8 = load <4 x float>* %boxmax.addr, align 16
  %tmp9 = load <4 x float>* %r_o.addr, align 16
  %sub10 = fsub <4 x float> %tmp8, %tmp9
  %mul11 = fmul <4 x float> %tmp7, %sub10
  store <4 x float> %mul11, <4 x float>* %ttop, align 16
  %tmp13 = load <4 x float>* %ttop, align 16
  %tmp14 = load <4 x float>* %tbot, align 16
  %call = call <4 x float> @_Z3minDv4_fS_(<4 x float> %tmp13, <4 x float> %tmp14)
  store <4 x float> %call, <4 x float>* %tmin, align 16
  %tmp16 = load <4 x float>* %ttop, align 16
  %tmp17 = load <4 x float>* %tbot, align 16
  %call18 = call <4 x float> @_Z3maxDv4_fS_(<4 x float> %tmp16, <4 x float> %tmp17)
  store <4 x float> %call18, <4 x float>* %tmax, align 16
  %tmp20 = load <4 x float>* %tmin
  %tmp21 = extractelement <4 x float> %tmp20, i32 0
  %tmp22 = load <4 x float>* %tmin
  %tmp23 = extractelement <4 x float> %tmp22, i32 1
  %call24 = call float @_Z3maxff(float %tmp21, float %tmp23)
  %tmp25 = load <4 x float>* %tmin
  %tmp26 = extractelement <4 x float> %tmp25, i32 0
  %tmp27 = load <4 x float>* %tmin
  %tmp28 = extractelement <4 x float> %tmp27, i32 2
  %call29 = call float @_Z3maxff(float %tmp26, float %tmp28)
  %call30 = call float @_Z3maxff(float %call24, float %call29)
  store float %call30, float* %largest_tmin, align 4
  %tmp32 = load <4 x float>* %tmax
  %tmp33 = extractelement <4 x float> %tmp32, i32 0
  %tmp34 = load <4 x float>* %tmax
  %tmp35 = extractelement <4 x float> %tmp34, i32 1
  %call36 = call float @_Z3minff(float %tmp33, float %tmp35)
  %tmp37 = load <4 x float>* %tmax
  %tmp38 = extractelement <4 x float> %tmp37, i32 0
  %tmp39 = load <4 x float>* %tmax
  %tmp40 = extractelement <4 x float> %tmp39, i32 2
  %call41 = call float @_Z3minff(float %tmp38, float %tmp40)
  %call42 = call float @_Z3minff(float %call36, float %call41)
  store float %call42, float* %smallest_tmax, align 4
  %tmp43 = load float* %largest_tmin, align 4
  %tmp44 = load float** %tnear.addr, align 4
  store float %tmp43, float* %tmp44
  %tmp45 = load float* %smallest_tmax, align 4
  %tmp46 = load float** %tfar.addr, align 4
  store float %tmp45, float* %tmp46
  %tmp47 = load float* %smallest_tmax, align 4
  %tmp48 = load float* %largest_tmin, align 4
  %cmp = fcmp ogt float %tmp47, %tmp48
  %conv = zext i1 %cmp to i32
  ret i32 %conv
}

declare <4 x float> @_Z3minDv4_fS_(<4 x float>, <4 x float>)

declare <4 x float> @_Z3maxDv4_fS_(<4 x float>, <4 x float>)

declare float @_Z3maxff(float, float)

declare float @_Z3minff(float, float)

define i32 @rgbaFloatToInt(<4 x float> %rgba) nounwind {
entry:
  %rgba.addr = alloca <4 x float>, align 16
  store <4 x float> %rgba, <4 x float>* %rgba.addr, align 16
  %tmp = load <4 x float>* %rgba.addr
  %tmp1 = extractelement <4 x float> %tmp, i32 0
  %call = call float @_Z5clampfff(float %tmp1, float 0.000000e+000, float 1.000000e+000)
  %tmp2 = load <4 x float>* %rgba.addr
  %tmp3 = insertelement <4 x float> %tmp2, float %call, i32 0
  store <4 x float> %tmp3, <4 x float>* %rgba.addr
  %tmp4 = load <4 x float>* %rgba.addr
  %tmp5 = extractelement <4 x float> %tmp4, i32 1
  %call6 = call float @_Z5clampfff(float %tmp5, float 0.000000e+000, float 1.000000e+000)
  %tmp7 = load <4 x float>* %rgba.addr
  %tmp8 = insertelement <4 x float> %tmp7, float %call6, i32 1
  store <4 x float> %tmp8, <4 x float>* %rgba.addr
  %tmp9 = load <4 x float>* %rgba.addr
  %tmp10 = extractelement <4 x float> %tmp9, i32 2
  %call11 = call float @_Z5clampfff(float %tmp10, float 0.000000e+000, float 1.000000e+000)
  %tmp12 = load <4 x float>* %rgba.addr
  %tmp13 = insertelement <4 x float> %tmp12, float %call11, i32 2
  store <4 x float> %tmp13, <4 x float>* %rgba.addr
  %tmp14 = load <4 x float>* %rgba.addr
  %tmp15 = extractelement <4 x float> %tmp14, i32 3
  %call16 = call float @_Z5clampfff(float %tmp15, float 0.000000e+000, float 1.000000e+000)
  %tmp17 = load <4 x float>* %rgba.addr
  %tmp18 = insertelement <4 x float> %tmp17, float %call16, i32 3
  store <4 x float> %tmp18, <4 x float>* %rgba.addr
  %tmp19 = load <4 x float>* %rgba.addr
  %tmp20 = extractelement <4 x float> %tmp19, i32 3
  %mul = fmul float %tmp20, 2.550000e+002
  %conv = fptoui float %mul to i32
  %shl = shl i32 %conv, 24
  %tmp21 = load <4 x float>* %rgba.addr
  %tmp22 = extractelement <4 x float> %tmp21, i32 2
  %mul23 = fmul float %tmp22, 2.550000e+002
  %conv24 = fptoui float %mul23 to i32
  %shl25 = shl i32 %conv24, 16
  %or = or i32 %shl, %shl25
  %tmp26 = load <4 x float>* %rgba.addr
  %tmp27 = extractelement <4 x float> %tmp26, i32 1
  %mul28 = fmul float %tmp27, 2.550000e+002
  %conv29 = fptoui float %mul28 to i32
  %shl30 = shl i32 %conv29, 8
  %or31 = or i32 %or, %shl30
  %tmp32 = load <4 x float>* %rgba.addr
  %tmp33 = extractelement <4 x float> %tmp32, i32 0
  %mul34 = fmul float %tmp33, 2.550000e+002
  %conv35 = fptoui float %mul34 to i32
  %or36 = or i32 %or31, %conv35
  ret i32 %or36
}

declare float @_Z5clampfff(float, float, float)

define void @d_render(i32 addrspace(1)* %d_output, i32 %imageW, i32 %imageH, float %density, float %brightness, float %transferOffset, float %transferScale, float addrspace(2)* %invViewMatrix) nounwind {
entry:
  %d_output.addr = alloca i32 addrspace(1)*, align 4
  %imageW.addr = alloca i32, align 4
  %imageH.addr = alloca i32, align 4
  %density.addr = alloca float, align 4
  %brightness.addr = alloca float, align 4
  %transferOffset.addr = alloca float, align 4
  %transferScale.addr = alloca float, align 4
  %invViewMatrix.addr = alloca float addrspace(2)*, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %u = alloca float, align 4
  %v = alloca float, align 4
  %boxMin = alloca <4 x float>, align 16
  %.compoundliteral = alloca <4 x float>, align 16
  %boxMax = alloca <4 x float>, align 16
  %.compoundliteral15 = alloca <4 x float>, align 16
  %eyeRay_o = alloca <4 x float>, align 16
  %eyeRay_d = alloca <4 x float>, align 16
  %.compoundliteral19 = alloca <4 x float>, align 16
  %temp = alloca <4 x float>, align 16
  %.compoundliteral33 = alloca <4 x float>, align 16
  %.compoundliteral43 = alloca <4 x float>, align 16
  %.compoundliteral65 = alloca <4 x float>, align 16
  %.compoundliteral87 = alloca <4 x float>, align 16
  %tnear = alloca float, align 4
  %tfar = alloca float, align 4
  %hit = alloca i32, align 4
  %i = alloca i32, align 4
  %.compoundliteral141 = alloca <4 x float>, align 16
  %t = alloca float, align 4
  %i146 = alloca i32, align 4
  %pos = alloca <4 x float>, align 16
  %col = alloca <4 x float>, align 16
  %.compoundliteral161 = alloca <4 x float>, align 16
  %a = alloca float, align 4
  %.compoundliteral178 = alloca <4 x float>, align 16
  %i214 = alloca i32, align 4
  store i32 addrspace(1)* %d_output, i32 addrspace(1)** %d_output.addr, align 4
  store i32 %imageW, i32* %imageW.addr, align 4
  store i32 %imageH, i32* %imageH.addr, align 4
  store float %density, float* %density.addr, align 4
  store float %brightness, float* %brightness.addr, align 4
  store float %transferOffset, float* %transferOffset.addr, align 4
  store float %transferScale, float* %transferScale.addr, align 4
  store float addrspace(2)* %invViewMatrix, float addrspace(2)** %invViewMatrix.addr, align 4
  %call = call i32 @get_global_id(i32 0)
  store i32 %call, i32* %x, align 4
  %call1 = call i32 @get_global_id(i32 1)
  store i32 %call1, i32* %y, align 4
  %tmp = load i32* %x, align 4
  %conv = uitofp i32 %tmp to float
  %tmp2 = load i32* %imageW.addr, align 4
  %conv3 = uitofp i32 %tmp2 to float
  %div = fdiv float %conv, %conv3
  %mul = fmul float %div, 2.000000e+000
  %sub = fsub float %mul, 1.000000e+000
  store float %sub, float* %u, align 4
  %tmp5 = load i32* %y, align 4
  %conv6 = uitofp i32 %tmp5 to float
  %tmp7 = load i32* %imageH.addr, align 4
  %conv8 = uitofp i32 %tmp7 to float
  %div9 = fdiv float %conv6, %conv8
  %mul10 = fmul float %div9, 2.000000e+000
  %sub11 = fsub float %mul10, 1.000000e+000
  store float %sub11, float* %v, align 4
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float 1.000000e+000>, <4 x float>* %.compoundliteral
  %tmp13 = load <4 x float>* %.compoundliteral
  store <4 x float> %tmp13, <4 x float>* %boxMin, align 16
  store <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float>* %.compoundliteral15
  %tmp16 = load <4 x float>* %.compoundliteral15
  store <4 x float> %tmp16, <4 x float>* %boxMax, align 16
  %tmp20 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx = getelementptr inbounds float addrspace(2)* %tmp20, i32 3
  %tmp21 = load float addrspace(2)* %arrayidx
  %vecinit = insertelement <4 x float> undef, float %tmp21, i32 0
  %tmp22 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx23 = getelementptr inbounds float addrspace(2)* %tmp22, i32 7
  %tmp24 = load float addrspace(2)* %arrayidx23
  %vecinit25 = insertelement <4 x float> %vecinit, float %tmp24, i32 1
  %tmp26 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx27 = getelementptr inbounds float addrspace(2)* %tmp26, i32 11
  %tmp28 = load float addrspace(2)* %arrayidx27
  %vecinit29 = insertelement <4 x float> %vecinit25, float %tmp28, i32 2
  %vecinit30 = insertelement <4 x float> %vecinit29, float 1.000000e+000, i32 3
  store <4 x float> %vecinit30, <4 x float>* %.compoundliteral19
  %tmp31 = load <4 x float>* %.compoundliteral19
  store <4 x float> %tmp31, <4 x float>* %eyeRay_o, align 16
  %tmp34 = load float* %u, align 4
  %vecinit35 = insertelement <4 x float> undef, float %tmp34, i32 0
  %tmp36 = load float* %v, align 4
  %vecinit37 = insertelement <4 x float> %vecinit35, float %tmp36, i32 1
  %vecinit38 = insertelement <4 x float> %vecinit37, float -2.000000e+000, i32 2
  %vecinit39 = insertelement <4 x float> %vecinit38, float 0.000000e+000, i32 3
  store <4 x float> %vecinit39, <4 x float>* %.compoundliteral33
  %tmp40 = load <4 x float>* %.compoundliteral33
  %call41 = call <4 x float> @_Z9normalizeDv4_f(<4 x float> %tmp40)
  store <4 x float> %call41, <4 x float>* %temp, align 16
  %tmp42 = load <4 x float>* %temp, align 16
  %tmp44 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx45 = getelementptr inbounds float addrspace(2)* %tmp44, i32 0
  %tmp46 = load float addrspace(2)* %arrayidx45
  %vecinit47 = insertelement <4 x float> undef, float %tmp46, i32 0
  %tmp48 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx49 = getelementptr inbounds float addrspace(2)* %tmp48, i32 1
  %tmp50 = load float addrspace(2)* %arrayidx49
  %vecinit51 = insertelement <4 x float> %vecinit47, float %tmp50, i32 1
  %tmp52 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx53 = getelementptr inbounds float addrspace(2)* %tmp52, i32 2
  %tmp54 = load float addrspace(2)* %arrayidx53
  %vecinit55 = insertelement <4 x float> %vecinit51, float %tmp54, i32 2
  %tmp56 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx57 = getelementptr inbounds float addrspace(2)* %tmp56, i32 3
  %tmp58 = load float addrspace(2)* %arrayidx57
  %vecinit59 = insertelement <4 x float> %vecinit55, float %tmp58, i32 3
  store <4 x float> %vecinit59, <4 x float>* %.compoundliteral43
  %tmp60 = load <4 x float>* %.compoundliteral43
  %call61 = call float @_Z3dotDv4_fS_(<4 x float> %tmp42, <4 x float> %tmp60)
  %tmp62 = load <4 x float>* %eyeRay_d
  %tmp63 = insertelement <4 x float> %tmp62, float %call61, i32 0
  store <4 x float> %tmp63, <4 x float>* %eyeRay_d
  %tmp64 = load <4 x float>* %temp, align 16
  %tmp66 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx67 = getelementptr inbounds float addrspace(2)* %tmp66, i32 4
  %tmp68 = load float addrspace(2)* %arrayidx67
  %vecinit69 = insertelement <4 x float> undef, float %tmp68, i32 0
  %tmp70 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx71 = getelementptr inbounds float addrspace(2)* %tmp70, i32 5
  %tmp72 = load float addrspace(2)* %arrayidx71
  %vecinit73 = insertelement <4 x float> %vecinit69, float %tmp72, i32 1
  %tmp74 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx75 = getelementptr inbounds float addrspace(2)* %tmp74, i32 6
  %tmp76 = load float addrspace(2)* %arrayidx75
  %vecinit77 = insertelement <4 x float> %vecinit73, float %tmp76, i32 2
  %tmp78 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx79 = getelementptr inbounds float addrspace(2)* %tmp78, i32 7
  %tmp80 = load float addrspace(2)* %arrayidx79
  %vecinit81 = insertelement <4 x float> %vecinit77, float %tmp80, i32 3
  store <4 x float> %vecinit81, <4 x float>* %.compoundliteral65
  %tmp82 = load <4 x float>* %.compoundliteral65
  %call83 = call float @_Z3dotDv4_fS_(<4 x float> %tmp64, <4 x float> %tmp82)
  %tmp84 = load <4 x float>* %eyeRay_d
  %tmp85 = insertelement <4 x float> %tmp84, float %call83, i32 1
  store <4 x float> %tmp85, <4 x float>* %eyeRay_d
  %tmp86 = load <4 x float>* %temp, align 16
  %tmp88 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx89 = getelementptr inbounds float addrspace(2)* %tmp88, i32 8
  %tmp90 = load float addrspace(2)* %arrayidx89
  %vecinit91 = insertelement <4 x float> undef, float %tmp90, i32 0
  %tmp92 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx93 = getelementptr inbounds float addrspace(2)* %tmp92, i32 9
  %tmp94 = load float addrspace(2)* %arrayidx93
  %vecinit95 = insertelement <4 x float> %vecinit91, float %tmp94, i32 1
  %tmp96 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx97 = getelementptr inbounds float addrspace(2)* %tmp96, i32 10
  %tmp98 = load float addrspace(2)* %arrayidx97
  %vecinit99 = insertelement <4 x float> %vecinit95, float %tmp98, i32 2
  %tmp100 = load float addrspace(2)** %invViewMatrix.addr, align 4
  %arrayidx101 = getelementptr inbounds float addrspace(2)* %tmp100, i32 11
  %tmp102 = load float addrspace(2)* %arrayidx101
  %vecinit103 = insertelement <4 x float> %vecinit99, float %tmp102, i32 3
  store <4 x float> %vecinit103, <4 x float>* %.compoundliteral87
  %tmp104 = load <4 x float>* %.compoundliteral87
  %call105 = call float @_Z3dotDv4_fS_(<4 x float> %tmp86, <4 x float> %tmp104)
  %tmp106 = load <4 x float>* %eyeRay_d
  %tmp107 = insertelement <4 x float> %tmp106, float %call105, i32 2
  store <4 x float> %tmp107, <4 x float>* %eyeRay_d
  %tmp108 = load <4 x float>* %eyeRay_d
  %tmp109 = insertelement <4 x float> %tmp108, float 0.000000e+000, i32 3
  store <4 x float> %tmp109, <4 x float>* %eyeRay_d
  %tmp113 = load <4 x float>* %eyeRay_o, align 16
  %tmp114 = load <4 x float>* %eyeRay_d, align 16
  %tmp115 = load <4 x float>* %boxMin, align 16
  %tmp116 = load <4 x float>* %boxMax, align 16
  %call117 = call i32 @intersectBox(<4 x float> %tmp113, <4 x float> %tmp114, <4 x float> %tmp115, <4 x float> %tmp116, float* %tnear, float* %tfar)
  store i32 %call117, i32* %hit, align 4
  %tmp118 = load i32* %hit, align 4
  %tobool = icmp ne i32 %tmp118, 0
  br i1 %tobool, label %if.end135, label %if.then

if.then:                                          ; preds = %entry
  %tmp119 = load i32* %x, align 4
  %tmp120 = load i32* %imageW.addr, align 4
  %cmp = icmp ult i32 %tmp119, %tmp120
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %if.then
  %tmp122 = load i32* %y, align 4
  %tmp123 = load i32* %imageH.addr, align 4
  %cmp124 = icmp ult i32 %tmp122, %tmp123
  br i1 %cmp124, label %if.then126, label %if.end

if.then126:                                       ; preds = %land.lhs.true
  %tmp128 = load i32* %y, align 4
  %tmp129 = load i32* %imageW.addr, align 4
  %mul130 = mul i32 %tmp128, %tmp129
  %tmp131 = load i32* %x, align 4
  %add = add i32 %mul130, %tmp131
  store i32 %add, i32* %i, align 4
  %tmp132 = load i32* %i, align 4
  %tmp133 = load i32 addrspace(1)** %d_output.addr, align 4
  %arrayidx134 = getelementptr inbounds i32 addrspace(1)* %tmp133, i32 %tmp132
  store i32 0, i32 addrspace(1)* %arrayidx134
  br label %if.end

if.end:                                           ; preds = %if.then126, %land.lhs.true, %if.then
  br label %if.end225

if.end135:                                        ; preds = %entry
  %tmp136 = load float* %tnear, align 4
  %cmp137 = fcmp olt float %tmp136, 0.000000e+000
  br i1 %cmp137, label %if.then139, label %if.end140

if.then139:                                       ; preds = %if.end135
  store float 0.000000e+000, float* %tnear, align 4
  br label %if.end140

if.end140:                                        ; preds = %if.then139, %if.end135
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral141
  %tmp142 = load <4 x float>* %.compoundliteral141
  store <4 x float> %tmp142, <4 x float>* %temp, align 16
  %tmp144 = load float* %tfar, align 4
  store float %tmp144, float* %t, align 4
  store i32 0, i32* %i146, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end140
  %tmp147 = load i32* %i146, align 4
  %cmp148 = icmp ult i32 %tmp147, 500
  br i1 %cmp148, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp151 = load <4 x float>* %eyeRay_o, align 16
  %tmp152 = load <4 x float>* %eyeRay_d, align 16
  %tmp153 = load float* %t, align 4
  %tmp154 = insertelement <4 x float> undef, float %tmp153, i32 0
  %splat = shufflevector <4 x float> %tmp154, <4 x float> %tmp154, <4 x i32> zeroinitializer
  %mul155 = fmul <4 x float> %tmp152, %splat
  %add156 = fadd <4 x float> %tmp151, %mul155
  store <4 x float> %add156, <4 x float>* %pos, align 16
  %tmp157 = load <4 x float>* %pos, align 16
  %mul158 = fmul <4 x float> %tmp157, <float 5.000000e-001, float 5.000000e-001, float 5.000000e-001, float 5.000000e-001>
  %add159 = fadd <4 x float> %mul158, <float 5.000000e-001, float 5.000000e-001, float 5.000000e-001, float 5.000000e-001>
  store <4 x float> %add159, <4 x float>* %pos, align 16
  %tmp162 = load <4 x float>* %pos
  %tmp163 = extractelement <4 x float> %tmp162, i32 0
  %0 = shufflevector <4 x float> %tmp162, <4 x float> undef, <4 x i32> <i32 0, i32 undef, i32 undef, i32 undef>
  %tmp164 = load <4 x float>* %pos
  %tmp165 = extractelement <4 x float> %tmp164, i32 1
  %1 = shufflevector <4 x float> %tmp162, <4 x float> %tmp164, <4 x i32> <i32 0, i32 5, i32 undef, i32 undef>
  %tmp166 = load <4 x float>* %pos
  %tmp167 = extractelement <4 x float> %tmp166, i32 2
  %vecinit168 = insertelement <4 x float> %1, float %tmp167, i32 2
  %vecinit169 = insertelement <4 x float> %vecinit168, float 2.500000e-001, i32 3
  store <4 x float> %vecinit169, <4 x float>* %.compoundliteral161
  %tmp170 = load <4 x float>* %.compoundliteral161
  store <4 x float> %tmp170, <4 x float>* %col, align 16
  %tmp172 = load <4 x float>* %col
  %tmp173 = extractelement <4 x float> %tmp172, i32 3
  %tmp174 = load float* %density.addr, align 4
  %mul175 = fmul float %tmp173, %tmp174
  store float %mul175, float* %a, align 4
  %tmp176 = load <4 x float>* %temp, align 16
  %tmp177 = load <4 x float>* %col, align 16
  %tmp179 = load float* %a, align 4
  %vecinit180 = insertelement <4 x float> undef, float %tmp179, i32 0
  %tmp181 = load float* %a, align 4
  %vecinit182 = insertelement <4 x float> %vecinit180, float %tmp181, i32 1
  %tmp183 = load float* %a, align 4
  %vecinit184 = insertelement <4 x float> %vecinit182, float %tmp183, i32 2
  %tmp185 = load float* %a, align 4
  %vecinit186 = insertelement <4 x float> %vecinit184, float %tmp185, i32 3
  store <4 x float> %vecinit186, <4 x float>* %.compoundliteral178
  %tmp187 = load <4 x float>* %.compoundliteral178
  %call188 = call <4 x float> @_Z3mixDv4_fS_S_(<4 x float> %tmp176, <4 x float> %tmp177, <4 x float> %tmp187)
  store <4 x float> %call188, <4 x float>* %temp, align 16
  %tmp189 = load float* %t, align 4
  %sub190 = fsub float %tmp189, 0x3F847AE140000000
  store float %sub190, float* %t, align 4
  %tmp191 = load float* %t, align 4
  %tmp192 = load float* %tnear, align 4
  %cmp193 = fcmp olt float %tmp191, %tmp192
  br i1 %cmp193, label %if.then195, label %if.end196

if.then195:                                       ; preds = %for.body
  br label %for.end

if.end196:                                        ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end196
  %tmp197 = load i32* %i146, align 4
  %inc = add i32 %tmp197, 1
  store i32 %inc, i32* %i146, align 4
  br label %for.cond

for.end:                                          ; preds = %if.then195, %for.cond
  %tmp198 = load float* %brightness.addr, align 4
  %tmp199 = insertelement <4 x float> undef, float %tmp198, i32 0
  %splat200 = shufflevector <4 x float> %tmp199, <4 x float> %tmp199, <4 x i32> zeroinitializer
  %tmp201 = load <4 x float>* %temp, align 16
  %mul202 = fmul <4 x float> %tmp201, %splat200
  store <4 x float> %mul202, <4 x float>* %temp, align 16
  %tmp203 = load i32* %x, align 4
  %tmp204 = load i32* %imageW.addr, align 4
  %cmp205 = icmp ult i32 %tmp203, %tmp204
  br i1 %cmp205, label %land.lhs.true207, label %if.end225

land.lhs.true207:                                 ; preds = %for.end
  %tmp208 = load i32* %y, align 4
  %tmp209 = load i32* %imageH.addr, align 4
  %cmp210 = icmp ult i32 %tmp208, %tmp209
  br i1 %cmp210, label %if.then212, label %if.end225

if.then212:                                       ; preds = %land.lhs.true207
  %tmp215 = load i32* %y, align 4
  %tmp216 = load i32* %imageW.addr, align 4
  %mul217 = mul i32 %tmp215, %tmp216
  %tmp218 = load i32* %x, align 4
  %add219 = add i32 %mul217, %tmp218
  store i32 %add219, i32* %i214, align 4
  %tmp220 = load <4 x float>* %temp, align 16
  %call221 = call i32 @rgbaFloatToInt(<4 x float> %tmp220)
  %tmp222 = load i32* %i214, align 4
  %tmp223 = load i32 addrspace(1)** %d_output.addr, align 4
  %arrayidx224 = getelementptr inbounds i32 addrspace(1)* %tmp223, i32 %tmp222
  store i32 %call221, i32 addrspace(1)* %arrayidx224
  br label %if.end225

if.end225:                                        ; preds = %if.then212, %land.lhs.true207, %for.end, %if.end
  ret void
}

declare i32 @get_global_id(i32)

declare <4 x float> @_Z9normalizeDv4_f(<4 x float>)

declare float @_Z3dotDv4_fS_(<4 x float>, <4 x float>)

declare <4 x float> @_Z3mixDv4_fS_S_(<4 x float>, <4 x float>, <4 x float>)

!opencl.kernels = !{!0}
!opencl.build.options = !{}

!0 = metadata !{void (i32 addrspace(1)*, i32, i32, float, float, float, float, float addrspace(2)*)* @d_render, metadata !1, metadata !1, metadata !"", metadata !"uint __attribute__((address_space(1))) *, uint, uint, float, float, float, float, float __attribute__((address_space(2))) *", metadata !"opencl_d_render_locals_anchor", metadata !2, metadata !3, metadata !4, metadata !5, metadata !""}
!1 = metadata !{i32 0, i32 0, i32 0}
!2 = metadata !{i32 1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 2}
!3 = metadata !{i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3}
!4 = metadata !{metadata !"uint*", metadata !"uint", metadata !"uint", metadata !"float", metadata !"float", metadata !"float", metadata !"float", metadata !"float*"}
!5 = metadata !{metadata !"d_output", metadata !"imageW", metadata !"imageH", metadata !"density", metadata !"brightness", metadata !"transferOffset", metadata !"transferScale", metadata !"invViewMatrix"}


