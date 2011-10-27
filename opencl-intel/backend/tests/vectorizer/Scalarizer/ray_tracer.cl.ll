; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\ray_tracer.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@PI = addrspace(1) constant float 0x400921FB60000000, align 4 ; <float addrspace(1)*> [#uses=2]
@g_renderWidth = addrspace(1) global float 5.120000e+002, align 4 ; <float addrspace(1)*> [#uses=1]
@g_renderHeight = addrspace(1) global float 5.120000e+002, align 4 ; <float addrspace(1)*> [#uses=1]
@SPECULAR_EXPONENT = addrspace(1) global float 5.000000e+001, align 4 ; <float addrspace(1)*> [#uses=1]
@g_maxRayShots = addrspace(1) global i32 4, align 4 ; <i32 addrspace(1)*> [#uses=1]
@g_numberOfSpheres = addrspace(1) global i32 35, align 4 ; <i32 addrspace(1)*> [#uses=1]
@g_numberOfSphereParameters = addrspace(1) global i32 13, align 4 ; <i32 addrspace(1)*> [#uses=1]
@g_pSphereArray = common global float addrspace(1)* null, align 4 ; <float addrspace(1)**> [#uses=1]
@g_viewPlaneDistance = common addrspace(1) global float 0.000000e+000, align 4 ; <float addrspace(1)*> [#uses=1]
@g_lightPos = common addrspace(1) global <4 x float> zeroinitializer, align 16 ; <<4 x float> addrspace(1)*> [#uses=1]
@opencl_evaluateDependents_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_evaluateDependents_parameters = appending global [182 x i8] c"float __attribute__((address_space(1))) *, float4 const, float const, float4 const, float4 const, uint const, uint const, uint const, int const, int const, float const, float4 const\00", section "llvm.metadata" ; <[182 x i8]*> [#uses=1]
@opencl_wlraytracer_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlraytracer_parameters = appending global [137 x i8] c"float4 *, float __attribute__((address_space(1))) *, uint const, uint const, uint const, int const, int const, float const, float4 const\00", section "llvm.metadata" ; <[137 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, <4 x float>, float, <4 x float>, <4 x float>, i32, i32, i32, i32, i32, float, <4 x float>)* @evaluateDependents to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_evaluateDependents_locals to i8*), i8* getelementptr inbounds ([182 x i8]* @opencl_evaluateDependents_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float>*, float addrspace(1)*, i32, i32, i32, i32, i32, float, <4 x float>)* @wlraytracer to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlraytracer_locals to i8*), i8* getelementptr inbounds ([137 x i8]* @opencl_wlraytracer_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @evaluateDependents(float addrspace(1)* %pSphereArray, <4 x float> %sphere0Position, float %sphere0Radius, <4 x float> %sphere0Color, <4 x float> %sphere0Material, i32 %numberOfSpheres, i32 %numberOfSphereParameters, i32 %maxRayShots, i32 %renderWidth, i32 %renderHeight, float %viewPlaneDistance, <4 x float> %lightPos) nounwind {
entry:
  %pSphereArray.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=41]
  %sphere0Position.addr = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=5]
  %sphere0Radius.addr = alloca float, align 4     ; <float*> [#uses=2]
  %sphere0Color.addr = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=5]
  %sphere0Material.addr = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=5]
  %numberOfSpheres.addr = alloca i32, align 4     ; <i32*> [#uses=3]
  %numberOfSphereParameters.addr = alloca i32, align 4 ; <i32*> [#uses=5]
  %maxRayShots.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %renderWidth.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %renderHeight.addr = alloca i32, align 4        ; <i32*> [#uses=2]
  %viewPlaneDistance.addr = alloca float, align 4 ; <float*> [#uses=2]
  %lightPos.addr = alloca <4 x float>, align 16   ; <<4 x float>*> [#uses=2]
  %pbkVariable = alloca i32, align 4              ; <i32*> [#uses=4]
  %i = alloca i32, align 4                        ; <i32*> [#uses=17]
  %ifloat = alloca float, align 4                 ; <float*> [#uses=9]
  store float addrspace(1)* %pSphereArray, float addrspace(1)** %pSphereArray.addr
  store <4 x float> %sphere0Position, <4 x float>* %sphere0Position.addr
  store float %sphere0Radius, float* %sphere0Radius.addr
  store <4 x float> %sphere0Color, <4 x float>* %sphere0Color.addr
  store <4 x float> %sphere0Material, <4 x float>* %sphere0Material.addr
  store i32 %numberOfSpheres, i32* %numberOfSpheres.addr
  store i32 %numberOfSphereParameters, i32* %numberOfSphereParameters.addr
  store i32 %maxRayShots, i32* %maxRayShots.addr
  store i32 %renderWidth, i32* %renderWidth.addr
  store i32 %renderHeight, i32* %renderHeight.addr
  store float %viewPlaneDistance, float* %viewPlaneDistance.addr
  store <4 x float> %lightPos, <4 x float>* %lightPos.addr
  %tmp = load i32* %numberOfSpheres.addr          ; <i32> [#uses=1]
  store i32 %tmp, i32 addrspace(1)* @g_numberOfSpheres
  %tmp1 = load i32* %numberOfSphereParameters.addr ; <i32> [#uses=1]
  store i32 %tmp1, i32 addrspace(1)* @g_numberOfSphereParameters
  %tmp2 = load i32* %maxRayShots.addr             ; <i32> [#uses=1]
  store i32 %tmp2, i32 addrspace(1)* @g_maxRayShots
  %tmp3 = load i32* %renderWidth.addr             ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp3 to float               ; <float> [#uses=1]
  store float %conv, float addrspace(1)* @g_renderWidth
  %tmp4 = load i32* %renderHeight.addr            ; <i32> [#uses=1]
  %conv5 = sitofp i32 %tmp4 to float              ; <float> [#uses=1]
  store float %conv5, float addrspace(1)* @g_renderHeight
  %tmp6 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  store float addrspace(1)* %tmp6, float addrspace(1)** @g_pSphereArray
  %tmp7 = load float* %viewPlaneDistance.addr     ; <float> [#uses=1]
  store float %tmp7, float addrspace(1)* @g_viewPlaneDistance
  %tmp8 = load <4 x float>* %lightPos.addr        ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp8, <4 x float> addrspace(1)* @g_lightPos
  %tmp9 = load <4 x float>* %sphere0Position.addr ; <<4 x float>> [#uses=1]
  %tmp10 = extractelement <4 x float> %tmp9, i32 0 ; <float> [#uses=1]
  %tmp11 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp11, i32 0 ; <float addrspace(1)*> [#uses=1]
  store float %tmp10, float addrspace(1)* %arrayidx
  %tmp12 = load <4 x float>* %sphere0Position.addr ; <<4 x float>> [#uses=1]
  %tmp13 = extractelement <4 x float> %tmp12, i32 1 ; <float> [#uses=1]
  %tmp14 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx15 = getelementptr inbounds float addrspace(1)* %tmp14, i32 1 ; <float addrspace(1)*> [#uses=1]
  store float %tmp13, float addrspace(1)* %arrayidx15
  %tmp16 = load <4 x float>* %sphere0Position.addr ; <<4 x float>> [#uses=1]
  %tmp17 = extractelement <4 x float> %tmp16, i32 2 ; <float> [#uses=1]
  %tmp18 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx19 = getelementptr inbounds float addrspace(1)* %tmp18, i32 2 ; <float addrspace(1)*> [#uses=1]
  store float %tmp17, float addrspace(1)* %arrayidx19
  %tmp20 = load <4 x float>* %sphere0Position.addr ; <<4 x float>> [#uses=1]
  %tmp21 = extractelement <4 x float> %tmp20, i32 3 ; <float> [#uses=1]
  %tmp22 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx23 = getelementptr inbounds float addrspace(1)* %tmp22, i32 3 ; <float addrspace(1)*> [#uses=1]
  store float %tmp21, float addrspace(1)* %arrayidx23
  %tmp24 = load float* %sphere0Radius.addr        ; <float> [#uses=1]
  %tmp25 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx26 = getelementptr inbounds float addrspace(1)* %tmp25, i32 4 ; <float addrspace(1)*> [#uses=1]
  store float %tmp24, float addrspace(1)* %arrayidx26
  %tmp27 = load <4 x float>* %sphere0Color.addr   ; <<4 x float>> [#uses=1]
  %tmp28 = extractelement <4 x float> %tmp27, i32 0 ; <float> [#uses=1]
  %tmp29 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx30 = getelementptr inbounds float addrspace(1)* %tmp29, i32 5 ; <float addrspace(1)*> [#uses=1]
  store float %tmp28, float addrspace(1)* %arrayidx30
  %tmp31 = load <4 x float>* %sphere0Color.addr   ; <<4 x float>> [#uses=1]
  %tmp32 = extractelement <4 x float> %tmp31, i32 1 ; <float> [#uses=1]
  %tmp33 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx34 = getelementptr inbounds float addrspace(1)* %tmp33, i32 6 ; <float addrspace(1)*> [#uses=1]
  store float %tmp32, float addrspace(1)* %arrayidx34
  %tmp35 = load <4 x float>* %sphere0Color.addr   ; <<4 x float>> [#uses=1]
  %tmp36 = extractelement <4 x float> %tmp35, i32 2 ; <float> [#uses=1]
  %tmp37 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx38 = getelementptr inbounds float addrspace(1)* %tmp37, i32 7 ; <float addrspace(1)*> [#uses=1]
  store float %tmp36, float addrspace(1)* %arrayidx38
  %tmp39 = load <4 x float>* %sphere0Color.addr   ; <<4 x float>> [#uses=1]
  %tmp40 = extractelement <4 x float> %tmp39, i32 3 ; <float> [#uses=1]
  %tmp41 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx42 = getelementptr inbounds float addrspace(1)* %tmp41, i32 8 ; <float addrspace(1)*> [#uses=1]
  store float %tmp40, float addrspace(1)* %arrayidx42
  %tmp43 = load <4 x float>* %sphere0Material.addr ; <<4 x float>> [#uses=1]
  %tmp44 = extractelement <4 x float> %tmp43, i32 0 ; <float> [#uses=1]
  %tmp45 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx46 = getelementptr inbounds float addrspace(1)* %tmp45, i32 9 ; <float addrspace(1)*> [#uses=1]
  store float %tmp44, float addrspace(1)* %arrayidx46
  %tmp47 = load <4 x float>* %sphere0Material.addr ; <<4 x float>> [#uses=1]
  %tmp48 = extractelement <4 x float> %tmp47, i32 1 ; <float> [#uses=1]
  %tmp49 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx50 = getelementptr inbounds float addrspace(1)* %tmp49, i32 10 ; <float addrspace(1)*> [#uses=1]
  store float %tmp48, float addrspace(1)* %arrayidx50
  %tmp51 = load <4 x float>* %sphere0Material.addr ; <<4 x float>> [#uses=1]
  %tmp52 = extractelement <4 x float> %tmp51, i32 2 ; <float> [#uses=1]
  %tmp53 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx54 = getelementptr inbounds float addrspace(1)* %tmp53, i32 11 ; <float addrspace(1)*> [#uses=1]
  store float %tmp52, float addrspace(1)* %arrayidx54
  %tmp55 = load <4 x float>* %sphere0Material.addr ; <<4 x float>> [#uses=1]
  %tmp56 = extractelement <4 x float> %tmp55, i32 3 ; <float> [#uses=1]
  %tmp57 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx58 = getelementptr inbounds float addrspace(1)* %tmp57, i32 12 ; <float addrspace(1)*> [#uses=1]
  store float %tmp56, float addrspace(1)* %arrayidx58
  %tmp59 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx60 = getelementptr inbounds float addrspace(1)* %tmp59, i32 13 ; <float addrspace(1)*> [#uses=1]
  store float 0.000000e+000, float addrspace(1)* %arrayidx60
  %tmp61 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx62 = getelementptr inbounds float addrspace(1)* %tmp61, i32 14 ; <float addrspace(1)*> [#uses=1]
  store float -1.003000e+003, float addrspace(1)* %arrayidx62
  %tmp63 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx64 = getelementptr inbounds float addrspace(1)* %tmp63, i32 15 ; <float addrspace(1)*> [#uses=1]
  store float -8.000000e+000, float addrspace(1)* %arrayidx64
  %tmp65 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx66 = getelementptr inbounds float addrspace(1)* %tmp65, i32 16 ; <float addrspace(1)*> [#uses=1]
  store float 0.000000e+000, float addrspace(1)* %arrayidx66
  %tmp67 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx68 = getelementptr inbounds float addrspace(1)* %tmp67, i32 17 ; <float addrspace(1)*> [#uses=1]
  store float 1.000000e+003, float addrspace(1)* %arrayidx68
  %tmp69 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx70 = getelementptr inbounds float addrspace(1)* %tmp69, i32 18 ; <float addrspace(1)*> [#uses=1]
  store float 0x3FE3333340000000, float addrspace(1)* %arrayidx70
  %tmp71 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx72 = getelementptr inbounds float addrspace(1)* %tmp71, i32 19 ; <float addrspace(1)*> [#uses=1]
  store float 0x3FE3333340000000, float addrspace(1)* %arrayidx72
  %tmp73 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx74 = getelementptr inbounds float addrspace(1)* %tmp73, i32 20 ; <float addrspace(1)*> [#uses=1]
  store float 0x3FE3333340000000, float addrspace(1)* %arrayidx74
  %tmp75 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx76 = getelementptr inbounds float addrspace(1)* %tmp75, i32 21 ; <float addrspace(1)*> [#uses=1]
  store float 1.000000e+000, float addrspace(1)* %arrayidx76
  %tmp77 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx78 = getelementptr inbounds float addrspace(1)* %tmp77, i32 22 ; <float addrspace(1)*> [#uses=1]
  store float 0x3FB99999A0000000, float addrspace(1)* %arrayidx78
  %tmp79 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx80 = getelementptr inbounds float addrspace(1)* %tmp79, i32 23 ; <float addrspace(1)*> [#uses=1]
  store float 0x3FE99999A0000000, float addrspace(1)* %arrayidx80
  %tmp81 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx82 = getelementptr inbounds float addrspace(1)* %tmp81, i32 24 ; <float addrspace(1)*> [#uses=1]
  store float 5.000000e-001, float addrspace(1)* %arrayidx82
  %tmp83 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx84 = getelementptr inbounds float addrspace(1)* %tmp83, i32 25 ; <float addrspace(1)*> [#uses=1]
  store float 5.000000e-001, float addrspace(1)* %arrayidx84
  store i32 22, i32* %pbkVariable
  %tmp87 = load i32* %numberOfSphereParameters.addr ; <i32> [#uses=1]
  %mul = mul i32 %tmp87, 2                        ; <i32> [#uses=1]
  store i32 %mul, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp88 = load i32* %i                           ; <i32> [#uses=1]
  %tmp89 = load i32* %numberOfSpheres.addr        ; <i32> [#uses=1]
  %tmp90 = load i32* %numberOfSphereParameters.addr ; <i32> [#uses=1]
  %mul91 = mul i32 %tmp89, %tmp90                 ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp88, %mul91              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp94 = load i32* %pbkVariable                 ; <i32> [#uses=1]
  %conv95 = sitofp i32 %tmp94 to float            ; <float> [#uses=1]
  store float %conv95, float* %ifloat
  %tmp96 = load float* %ifloat                    ; <float> [#uses=1]
  %div = fdiv float %tmp96, 5.000000e+000         ; <float> [#uses=1]
  %call = call float @_Z3sinf(float %div)         ; <float> [#uses=1]
  %mul97 = fmul float %call, 6.000000e+000        ; <float> [#uses=1]
  %tmp98 = load i32* %i                           ; <i32> [#uses=1]
  %tmp99 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx100 = getelementptr inbounds float addrspace(1)* %tmp99, i32 %tmp98 ; <float addrspace(1)*> [#uses=1]
  store float %mul97, float addrspace(1)* %arrayidx100
  %tmp101 = load float* %ifloat                   ; <float> [#uses=1]
  %div102 = fdiv float %tmp101, 0x4010666660000000 ; <float> [#uses=1]
  %call103 = call float @_Z3sinf(float %div102)   ; <float> [#uses=1]
  %mul104 = fmul float %call103, 2.500000e+000    ; <float> [#uses=1]
  %tmp105 = load i32* %i                          ; <i32> [#uses=1]
  %add = add nsw i32 %tmp105, 1                   ; <i32> [#uses=1]
  %tmp106 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx107 = getelementptr inbounds float addrspace(1)* %tmp106, i32 %add ; <float addrspace(1)*> [#uses=1]
  store float %mul104, float addrspace(1)* %arrayidx107
  %tmp108 = load float* %ifloat                   ; <float> [#uses=1]
  %div109 = fdiv float %tmp108, 0x4008CCCCC0000000 ; <float> [#uses=1]
  %add110 = fadd float %div109, 0x3FF3333340000000 ; <float> [#uses=1]
  %call111 = call float @_Z3sinf(float %add110)   ; <float> [#uses=1]
  %mul112 = fmul float %call111, 1.000000e+001    ; <float> [#uses=1]
  %sub = fsub float -1.800000e+001, %mul112       ; <float> [#uses=1]
  %tmp113 = load i32* %i                          ; <i32> [#uses=1]
  %add114 = add nsw i32 %tmp113, 2                ; <i32> [#uses=1]
  %tmp115 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx116 = getelementptr inbounds float addrspace(1)* %tmp115, i32 %add114 ; <float addrspace(1)*> [#uses=1]
  store float %sub, float addrspace(1)* %arrayidx116
  %tmp117 = load i32* %i                          ; <i32> [#uses=1]
  %add118 = add nsw i32 %tmp117, 3                ; <i32> [#uses=1]
  %tmp119 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx120 = getelementptr inbounds float addrspace(1)* %tmp119, i32 %add118 ; <float addrspace(1)*> [#uses=1]
  store float 0.000000e+000, float addrspace(1)* %arrayidx120
  %tmp121 = load float* %ifloat                   ; <float> [#uses=1]
  %div122 = fdiv float %tmp121, 0x3FF570A3E0000000 ; <float> [#uses=1]
  %add123 = fadd float %div122, 0x4050533340000000 ; <float> [#uses=1]
  %call124 = call float @_Z3sinf(float %add123)   ; <float> [#uses=1]
  %mul125 = fmul float %call124, 5.000000e-001    ; <float> [#uses=1]
  %add126 = fadd float %mul125, 5.000000e-001     ; <float> [#uses=1]
  %call127 = call float @_Z3powff(float %add126, float 3.000000e+000) ; <float> [#uses=1]
  %mul128 = fmul float %call127, 1.000000e+000    ; <float> [#uses=1]
  %add129 = fadd float %mul128, 0x3FC99999A0000000 ; <float> [#uses=1]
  %tmp130 = load i32* %i                          ; <i32> [#uses=1]
  %add131 = add nsw i32 %tmp130, 4                ; <i32> [#uses=1]
  %tmp132 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx133 = getelementptr inbounds float addrspace(1)* %tmp132, i32 %add131 ; <float addrspace(1)*> [#uses=1]
  store float %add129, float addrspace(1)* %arrayidx133
  %tmp134 = load float* %ifloat                   ; <float> [#uses=1]
  %div135 = fdiv float %tmp134, 0x4000CCCCC0000000 ; <float> [#uses=1]
  %add136 = fadd float %div135, 0x3FF4CCCCC0000000 ; <float> [#uses=1]
  %call137 = call float @_Z3cosf(float %add136)   ; <float> [#uses=1]
  %mul138 = fmul float %call137, 5.000000e-001    ; <float> [#uses=1]
  %add139 = fadd float %mul138, 5.000000e-001     ; <float> [#uses=1]
  %tmp140 = load i32* %i                          ; <i32> [#uses=1]
  %add141 = add nsw i32 %tmp140, 5                ; <i32> [#uses=1]
  %tmp142 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx143 = getelementptr inbounds float addrspace(1)* %tmp142, i32 %add141 ; <float addrspace(1)*> [#uses=1]
  store float %add139, float addrspace(1)* %arrayidx143
  %tmp144 = load float* %ifloat                   ; <float> [#uses=1]
  %div145 = fdiv float %tmp144, 0x3FB99999A0000000 ; <float> [#uses=1]
  %add146 = fadd float %div145, 0x3FF4CCCCC0000000 ; <float> [#uses=1]
  %call147 = call float @_Z3cosf(float %add146)   ; <float> [#uses=1]
  %mul148 = fmul float %call147, 5.000000e-001    ; <float> [#uses=1]
  %add149 = fadd float %mul148, 5.000000e-001     ; <float> [#uses=1]
  %tmp150 = load i32* %i                          ; <i32> [#uses=1]
  %add151 = add nsw i32 %tmp150, 6                ; <i32> [#uses=1]
  %tmp152 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx153 = getelementptr inbounds float addrspace(1)* %tmp152, i32 %add151 ; <float addrspace(1)*> [#uses=1]
  store float %add149, float addrspace(1)* %arrayidx153
  %tmp154 = load float* %ifloat                   ; <float> [#uses=1]
  %div155 = fdiv float %tmp154, 0x4014666660000000 ; <float> [#uses=1]
  %add156 = fadd float %div155, 0x4019333340000000 ; <float> [#uses=1]
  %call157 = call float @_Z3cosf(float %add156)   ; <float> [#uses=1]
  %mul158 = fmul float %call157, 5.000000e-001    ; <float> [#uses=1]
  %add159 = fadd float %mul158, 5.000000e-001     ; <float> [#uses=1]
  %tmp160 = load i32* %i                          ; <i32> [#uses=1]
  %add161 = add nsw i32 %tmp160, 7                ; <i32> [#uses=1]
  %tmp162 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx163 = getelementptr inbounds float addrspace(1)* %tmp162, i32 %add161 ; <float addrspace(1)*> [#uses=1]
  store float %add159, float addrspace(1)* %arrayidx163
  %tmp164 = load i32* %i                          ; <i32> [#uses=1]
  %add165 = add nsw i32 %tmp164, 8                ; <i32> [#uses=1]
  %tmp166 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx167 = getelementptr inbounds float addrspace(1)* %tmp166, i32 %add165 ; <float addrspace(1)*> [#uses=1]
  store float 1.000000e+000, float addrspace(1)* %arrayidx167
  %tmp168 = load i32* %i                          ; <i32> [#uses=1]
  %add169 = add nsw i32 %tmp168, 9                ; <i32> [#uses=1]
  %tmp170 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx171 = getelementptr inbounds float addrspace(1)* %tmp170, i32 %add169 ; <float addrspace(1)*> [#uses=1]
  store float 0x3FB99999A0000000, float addrspace(1)* %arrayidx171
  %tmp172 = load i32* %i                          ; <i32> [#uses=1]
  %add173 = add nsw i32 %tmp172, 10               ; <i32> [#uses=1]
  %tmp174 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx175 = getelementptr inbounds float addrspace(1)* %tmp174, i32 %add173 ; <float addrspace(1)*> [#uses=1]
  store float 0x3FE6666660000000, float addrspace(1)* %arrayidx175
  %tmp176 = load i32* %i                          ; <i32> [#uses=1]
  %add177 = add nsw i32 %tmp176, 11               ; <i32> [#uses=1]
  %tmp178 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx179 = getelementptr inbounds float addrspace(1)* %tmp178, i32 %add177 ; <float addrspace(1)*> [#uses=1]
  store float 1.000000e+000, float addrspace(1)* %arrayidx179
  %tmp180 = load float* %ifloat                   ; <float> [#uses=1]
  %div181 = fdiv float %tmp180, 0x4000CCCCC0000000 ; <float> [#uses=1]
  %add182 = fadd float %div181, 0x3FF3E35400000000 ; <float> [#uses=1]
  %call183 = call float @_Z3sinf(float %add182)   ; <float> [#uses=1]
  %mul184 = fmul float %call183, 5.000000e-001    ; <float> [#uses=1]
  %add185 = fadd float %mul184, 5.000000e-001     ; <float> [#uses=1]
  %call186 = call float @_Z3powff(float %add185, float 5.000000e+000) ; <float> [#uses=1]
  %tmp187 = load i32* %i                          ; <i32> [#uses=1]
  %add188 = add nsw i32 %tmp187, 12               ; <i32> [#uses=1]
  %tmp189 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx190 = getelementptr inbounds float addrspace(1)* %tmp189, i32 %add188 ; <float addrspace(1)*> [#uses=1]
  store float %call186, float addrspace(1)* %arrayidx190
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp191 = load i32* %numberOfSphereParameters.addr ; <i32> [#uses=1]
  %tmp192 = load i32* %i                          ; <i32> [#uses=1]
  %add193 = add i32 %tmp192, %tmp191              ; <i32> [#uses=1]
  store i32 %add193, i32* %i
  %tmp194 = load i32* %pbkVariable                ; <i32> [#uses=1]
  %add195 = add nsw i32 %tmp194, 11               ; <i32> [#uses=1]
  store i32 %add195, i32* %pbkVariable
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare float @_Z3sinf(float)

declare float @_Z3powff(float, float)

declare float @_Z3cosf(float)

define void @shootRay(<4 x float> %origin, <4 x float> %dir, i32* %pHit, <4 x float>* %pPos, float* %pt, i32* %pSphereNum, float addrspace(1)* %pSphereArray, i32 %numberOfSpheres, i32 %numberOfSphereParameters) nounwind {
entry:
  %origin.addr = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=3]
  %dir.addr = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=3]
  %pHit.addr = alloca i32*, align 4               ; <i32**> [#uses=3]
  %pPos.addr = alloca <4 x float>*, align 4       ; <<4 x float>**> [#uses=2]
  %pt.addr = alloca float*, align 4               ; <float**> [#uses=5]
  %pSphereNum.addr = alloca i32*, align 4         ; <i32**> [#uses=2]
  %pSphereArray.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=6]
  %numberOfSpheres.addr = alloca i32, align 4     ; <i32*> [#uses=2]
  %numberOfSphereParameters.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %curT = alloca float, align 4                   ; <float*> [#uses=4]
  %B = alloca float, align 4                      ; <float*> [#uses=4]
  %C = alloca float, align 4                      ; <float*> [#uses=2]
  %disc = alloca float, align 4                   ; <float*> [#uses=3]
  %spherePos = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=2]
  %sphereToOrigin = alloca <4 x float>, align 16  ; <<4 x float>*> [#uses=4]
  %sphereRadius = alloca float, align 4           ; <float*> [#uses=3]
  %i = alloca i32, align 4                        ; <i32*> [#uses=10]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store <4 x float> %origin, <4 x float>* %origin.addr
  store <4 x float> %dir, <4 x float>* %dir.addr
  store i32* %pHit, i32** %pHit.addr
  store <4 x float>* %pPos, <4 x float>** %pPos.addr
  store float* %pt, float** %pt.addr
  store i32* %pSphereNum, i32** %pSphereNum.addr
  store float addrspace(1)* %pSphereArray, float addrspace(1)** %pSphereArray.addr
  store i32 %numberOfSpheres, i32* %numberOfSpheres.addr
  store i32 %numberOfSphereParameters, i32* %numberOfSphereParameters.addr
  %tmp = load i32** %pHit.addr                    ; <i32*> [#uses=1]
  store i32 0, i32* %tmp
  %tmp1 = load float** %pt.addr                   ; <float*> [#uses=1]
  store float 9.999900e+004, float* %tmp1
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp3 = load i32* %i                            ; <i32> [#uses=1]
  %tmp4 = load i32* %numberOfSpheres.addr         ; <i32> [#uses=1]
  %tmp5 = load i32* %numberOfSphereParameters.addr ; <i32> [#uses=1]
  %mul = mul i32 %tmp4, %tmp5                     ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp3, %mul                 ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp6 = load i32* %i                            ; <i32> [#uses=1]
  %tmp7 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp7, i32 %tmp6 ; <float addrspace(1)*> [#uses=1]
  %tmp8 = load float addrspace(1)* %arrayidx      ; <float> [#uses=1]
  %vecinit = insertelement <4 x float> undef, float %tmp8, i32 0 ; <<4 x float>> [#uses=1]
  %tmp9 = load i32* %i                            ; <i32> [#uses=1]
  %add = add nsw i32 %tmp9, 1                     ; <i32> [#uses=1]
  %tmp10 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx11 = getelementptr inbounds float addrspace(1)* %tmp10, i32 %add ; <float addrspace(1)*> [#uses=1]
  %tmp12 = load float addrspace(1)* %arrayidx11   ; <float> [#uses=1]
  %vecinit13 = insertelement <4 x float> %vecinit, float %tmp12, i32 1 ; <<4 x float>> [#uses=1]
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %add15 = add nsw i32 %tmp14, 2                  ; <i32> [#uses=1]
  %tmp16 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx17 = getelementptr inbounds float addrspace(1)* %tmp16, i32 %add15 ; <float addrspace(1)*> [#uses=1]
  %tmp18 = load float addrspace(1)* %arrayidx17   ; <float> [#uses=1]
  %vecinit19 = insertelement <4 x float> %vecinit13, float %tmp18, i32 2 ; <<4 x float>> [#uses=1]
  %tmp20 = load i32* %i                           ; <i32> [#uses=1]
  %add21 = add nsw i32 %tmp20, 3                  ; <i32> [#uses=1]
  %tmp22 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx23 = getelementptr inbounds float addrspace(1)* %tmp22, i32 %add21 ; <float addrspace(1)*> [#uses=1]
  %tmp24 = load float addrspace(1)* %arrayidx23   ; <float> [#uses=1]
  %vecinit25 = insertelement <4 x float> %vecinit19, float %tmp24, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit25, <4 x float>* %.compoundliteral
  %tmp26 = load <4 x float>* %.compoundliteral    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp26, <4 x float>* %spherePos
  %tmp27 = load i32* %i                           ; <i32> [#uses=1]
  %add28 = add nsw i32 %tmp27, 4                  ; <i32> [#uses=1]
  %tmp29 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx30 = getelementptr inbounds float addrspace(1)* %tmp29, i32 %add28 ; <float addrspace(1)*> [#uses=1]
  %tmp31 = load float addrspace(1)* %arrayidx30   ; <float> [#uses=1]
  store float %tmp31, float* %sphereRadius
  %tmp32 = load <4 x float>* %origin.addr         ; <<4 x float>> [#uses=1]
  %tmp33 = load <4 x float>* %spherePos           ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp32, %tmp33          ; <<4 x float>> [#uses=1]
  store <4 x float> %sub, <4 x float>* %sphereToOrigin
  %tmp34 = load <4 x float>* %sphereToOrigin      ; <<4 x float>> [#uses=1]
  %tmp35 = load <4 x float>* %dir.addr            ; <<4 x float>> [#uses=1]
  %call = call float @_Z3dotDv4_fS_(<4 x float> %tmp34, <4 x float> %tmp35) ; <float> [#uses=1]
  store float %call, float* %B
  %tmp36 = load <4 x float>* %sphereToOrigin      ; <<4 x float>> [#uses=1]
  %tmp37 = load <4 x float>* %sphereToOrigin      ; <<4 x float>> [#uses=1]
  %call38 = call float @_Z3dotDv4_fS_(<4 x float> %tmp36, <4 x float> %tmp37) ; <float> [#uses=1]
  %tmp39 = load float* %sphereRadius              ; <float> [#uses=1]
  %tmp40 = load float* %sphereRadius              ; <float> [#uses=1]
  %mul41 = fmul float %tmp39, %tmp40              ; <float> [#uses=1]
  %sub42 = fsub float %call38, %mul41             ; <float> [#uses=1]
  store float %sub42, float* %C
  %tmp43 = load float* %B                         ; <float> [#uses=1]
  %tmp44 = load float* %B                         ; <float> [#uses=1]
  %mul45 = fmul float %tmp43, %tmp44              ; <float> [#uses=1]
  %tmp46 = load float* %C                         ; <float> [#uses=1]
  %sub47 = fsub float %mul45, %tmp46              ; <float> [#uses=1]
  store float %sub47, float* %disc
  %tmp48 = load float* %disc                      ; <float> [#uses=1]
  %cmp49 = fcmp ogt float %tmp48, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp49, label %if.then, label %if.end68

if.then:                                          ; preds = %for.body
  %tmp50 = load float* %B                         ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %tmp50        ; <float> [#uses=1]
  %tmp51 = load float* %disc                      ; <float> [#uses=1]
  %call52 = call float @_Z4sqrtf(float %tmp51)    ; <float> [#uses=1]
  %sub53 = fsub float %neg, %call52               ; <float> [#uses=1]
  store float %sub53, float* %curT
  %tmp54 = load float* %curT                      ; <float> [#uses=1]
  %conv = fpext float %tmp54 to double            ; <double> [#uses=1]
  %cmp55 = fcmp ogt double %conv, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp55, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %if.then
  %tmp57 = load float* %curT                      ; <float> [#uses=1]
  %tmp58 = load float** %pt.addr                  ; <float*> [#uses=1]
  %tmp59 = load float* %tmp58                     ; <float> [#uses=1]
  %cmp60 = fcmp olt float %tmp57, %tmp59          ; <i1> [#uses=1]
  br i1 %cmp60, label %if.then62, label %if.end

if.then62:                                        ; preds = %land.lhs.true
  %tmp63 = load i32* %i                           ; <i32> [#uses=1]
  %tmp64 = load i32** %pSphereNum.addr            ; <i32*> [#uses=1]
  store i32 %tmp63, i32* %tmp64
  %tmp65 = load float* %curT                      ; <float> [#uses=1]
  %tmp66 = load float** %pt.addr                  ; <float*> [#uses=1]
  store float %tmp65, float* %tmp66
  %tmp67 = load i32** %pHit.addr                  ; <i32*> [#uses=1]
  store i32 1, i32* %tmp67
  br label %if.end

if.end:                                           ; preds = %if.then62, %land.lhs.true, %if.then
  br label %if.end68

if.end68:                                         ; preds = %if.end, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end68
  %tmp69 = load i32* %numberOfSphereParameters.addr ; <i32> [#uses=1]
  %tmp70 = load i32* %i                           ; <i32> [#uses=1]
  %add71 = add i32 %tmp70, %tmp69                 ; <i32> [#uses=1]
  store i32 %add71, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp72 = load <4 x float>* %origin.addr         ; <<4 x float>> [#uses=1]
  %tmp73 = load <4 x float>* %dir.addr            ; <<4 x float>> [#uses=1]
  %tmp74 = load float** %pt.addr                  ; <float*> [#uses=1]
  %tmp75 = load float* %tmp74                     ; <float> [#uses=1]
  %tmp76 = insertelement <4 x float> undef, float %tmp75, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp76, <4 x float> %tmp76, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul77 = fmul <4 x float> %tmp73, %splat        ; <<4 x float>> [#uses=1]
  %add78 = fadd <4 x float> %tmp72, %mul77        ; <<4 x float>> [#uses=1]
  %tmp79 = load <4 x float>** %pPos.addr          ; <<4 x float>*> [#uses=1]
  store <4 x float> %add78, <4 x float>* %tmp79
  ret void
}

declare float @_Z3dotDv4_fS_(<4 x float>, <4 x float>)

declare float @_Z4sqrtf(float)

define <4 x float> @evaluatePixel(float addrspace(1)* %pSphereArray, <2 x i32> %pixel, i32 %numberOfSpheres, i32 %numberOfSphereParameters, i32 %maxRayShots, i32 %renderWidth, i32 %renderHeight, float %viewPlaneDistance, <4 x float> %lightPos) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %pSphereArray.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=16]
  %pixel.addr = alloca <2 x i32>, align 8         ; <<2 x i32>*> [#uses=3]
  %numberOfSpheres.addr = alloca i32, align 4     ; <i32*> [#uses=3]
  %numberOfSphereParameters.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %maxRayShots.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %renderWidth.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %renderHeight.addr = alloca i32, align 4        ; <i32*> [#uses=2]
  %viewPlaneDistance.addr = alloca float, align 4 ; <float*> [#uses=2]
  %lightPos.addr = alloca <4 x float>, align 16   ; <<4 x float>*> [#uses=2]
  %dst = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=6]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %origin = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=3]
  %.compoundliteral2 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %dir = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=8]
  %.compoundliteral5 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %sphereNum = alloca i32, align 4                ; <i32*> [#uses=15]
  %spherePos = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=2]
  %sphereRadius = alloca float, align 4           ; <float*> [#uses=2]
  %sphereColor = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=5]
  %sphereMaterial = alloca <4 x float>, align 16  ; <<4 x float>*> [#uses=6]
  %hitPoint = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=5]
  %t = alloca float, align 4                      ; <float*> [#uses=3]
  %hit = alloca i32, align 4                      ; <i32*> [#uses=2]
  %sphereHit = alloca <4 x float>, align 16       ; <<4 x float>*> [#uses=2]
  %n = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=7]
  %lightVector = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=3]
  %lightVectorLen = alloca float, align 4         ; <float*> [#uses=3]
  %l = alloca <4 x float>, align 16               ; <<4 x float>*> [#uses=4]
  %lReflect = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=2]
  %dirReflect = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=4]
  %shadowTest = alloca i32, align 4               ; <i32*> [#uses=5]
  %temp = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=1]
  %temp2 = alloca i32, align 4                    ; <i32*> [#uses=1]
  %rayShots = alloca i32, align 4                 ; <i32*> [#uses=6]
  %colorScale = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=4]
  %.compoundliteral45 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %specular = alloca float, align 4               ; <float*> [#uses=4]
  %diffuse = alloca float, align 4                ; <float*> [#uses=5]
  %lightVal = alloca float, align 4               ; <float*> [#uses=2]
  %phi = alloca float, align 4                    ; <float*> [#uses=3]
  %uv = alloca <2 x float>, align 8               ; <<2 x float>*> [#uses=3]
  %.compoundliteral64 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral93 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral119 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral205 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  %.compoundliteral211 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %.compoundliteral212 = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store float addrspace(1)* %pSphereArray, float addrspace(1)** %pSphereArray.addr
  store <2 x i32> %pixel, <2 x i32>* %pixel.addr
  store i32 %numberOfSpheres, i32* %numberOfSpheres.addr
  store i32 %numberOfSphereParameters, i32* %numberOfSphereParameters.addr
  store i32 %maxRayShots, i32* %maxRayShots.addr
  store i32 %renderWidth, i32* %renderWidth.addr
  store i32 %renderHeight, i32* %renderHeight.addr
  store float %viewPlaneDistance, float* %viewPlaneDistance.addr
  store <4 x float> %lightPos, <4 x float>* %lightPos.addr
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp, <4 x float>* %dst
  store <4 x float> zeroinitializer, <4 x float>* %.compoundliteral2
  %tmp3 = load <4 x float>* %.compoundliteral2    ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp3, <4 x float>* %origin
  %tmp6 = load <2 x i32>* %pixel.addr             ; <<2 x i32>> [#uses=1]
  %tmp7 = extractelement <2 x i32> %tmp6, i32 0   ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp7 to float               ; <float> [#uses=1]
  %mul = fmul float 2.000000e+000, %conv          ; <float> [#uses=1]
  %tmp8 = load i32* %renderWidth.addr             ; <i32> [#uses=1]
  %conv9 = sitofp i32 %tmp8 to float              ; <float> [#uses=3]
  %cmp = fcmp oeq float 0.000000e+000, %conv9     ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %conv9 ; <float> [#uses=0]
  %div = fdiv float %mul, %conv9                  ; <float> [#uses=1]
  %sub = fsub float %div, 1.000000e+000           ; <float> [#uses=1]
  %vecinit = insertelement <4 x float> undef, float %sub, i32 0 ; <<4 x float>> [#uses=1]
  %tmp10 = load <2 x i32>* %pixel.addr            ; <<2 x i32>> [#uses=1]
  %tmp11 = extractelement <2 x i32> %tmp10, i32 1 ; <i32> [#uses=1]
  %conv12 = sitofp i32 %tmp11 to float            ; <float> [#uses=1]
  %mul13 = fmul float -2.000000e+000, %conv12     ; <float> [#uses=1]
  %tmp14 = load i32* %renderHeight.addr           ; <i32> [#uses=1]
  %conv15 = sitofp i32 %tmp14 to float            ; <float> [#uses=3]
  %cmp16 = fcmp oeq float 0.000000e+000, %conv15  ; <i1> [#uses=1]
  %sel17 = select i1 %cmp16, float 1.000000e+000, float %conv15 ; <float> [#uses=0]
  %div18 = fdiv float %mul13, %conv15             ; <float> [#uses=1]
  %add = fadd float %div18, 1.000000e+000         ; <float> [#uses=1]
  %vecinit19 = insertelement <4 x float> %vecinit, float %add, i32 1 ; <<4 x float>> [#uses=1]
  %tmp20 = load float* %viewPlaneDistance.addr    ; <float> [#uses=1]
  %neg = fsub float -0.000000e+000, %tmp20        ; <float> [#uses=1]
  %vecinit21 = insertelement <4 x float> %vecinit19, float %neg, i32 2 ; <<4 x float>> [#uses=1]
  %vecinit22 = insertelement <4 x float> %vecinit21, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit22, <4 x float>* %.compoundliteral5
  %tmp23 = load <4 x float>* %.compoundliteral5   ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp23, <4 x float>* %dir
  %tmp43 = load i32* %maxRayShots.addr            ; <i32> [#uses=1]
  store i32 %tmp43, i32* %rayShots
  store <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 0.000000e+000>, <4 x float>* %.compoundliteral45
  %tmp46 = load <4 x float>* %.compoundliteral45  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp46, <4 x float>* %colorScale
  br label %while.cond

while.cond:                                       ; preds = %if.end309, %entry
  %tmp52 = load i32* %rayShots                    ; <i32> [#uses=1]
  %cmp53 = icmp sgt i32 %tmp52, 0                 ; <i1> [#uses=1]
  br i1 %cmp53, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %tmp55 = load <4 x float>* %dir                 ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z9normalizeDv4_f(<4 x float> %tmp55) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %dir
  %tmp56 = load <4 x float>* %origin              ; <<4 x float>> [#uses=1]
  %tmp57 = load <4 x float>* %dir                 ; <<4 x float>> [#uses=1]
  %tmp58 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %tmp59 = load i32* %numberOfSpheres.addr        ; <i32> [#uses=1]
  %tmp60 = load i32* %numberOfSphereParameters.addr ; <i32> [#uses=1]
  call void @shootRay(<4 x float> %tmp56, <4 x float> %tmp57, i32* %hit, <4 x float>* %hitPoint, float* %t, i32* %sphereNum, float addrspace(1)* %tmp58, i32 %tmp59, i32 %tmp60)
  %tmp61 = load i32* %hit                         ; <i32> [#uses=1]
  %cmp62 = icmp ne i32 %tmp61, 0                  ; <i1> [#uses=1]
  br i1 %cmp62, label %if.then, label %if.else308

if.then:                                          ; preds = %while.body
  %tmp65 = load i32* %sphereNum                   ; <i32> [#uses=1]
  %tmp66 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp66, i32 %tmp65 ; <float addrspace(1)*> [#uses=1]
  %tmp67 = load float addrspace(1)* %arrayidx     ; <float> [#uses=1]
  %vecinit68 = insertelement <4 x float> undef, float %tmp67, i32 0 ; <<4 x float>> [#uses=1]
  %tmp69 = load i32* %sphereNum                   ; <i32> [#uses=1]
  %add70 = add nsw i32 %tmp69, 1                  ; <i32> [#uses=1]
  %tmp71 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx72 = getelementptr inbounds float addrspace(1)* %tmp71, i32 %add70 ; <float addrspace(1)*> [#uses=1]
  %tmp73 = load float addrspace(1)* %arrayidx72   ; <float> [#uses=1]
  %vecinit74 = insertelement <4 x float> %vecinit68, float %tmp73, i32 1 ; <<4 x float>> [#uses=1]
  %tmp75 = load i32* %sphereNum                   ; <i32> [#uses=1]
  %add76 = add nsw i32 %tmp75, 2                  ; <i32> [#uses=1]
  %tmp77 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx78 = getelementptr inbounds float addrspace(1)* %tmp77, i32 %add76 ; <float addrspace(1)*> [#uses=1]
  %tmp79 = load float addrspace(1)* %arrayidx78   ; <float> [#uses=1]
  %vecinit80 = insertelement <4 x float> %vecinit74, float %tmp79, i32 2 ; <<4 x float>> [#uses=1]
  %tmp81 = load i32* %sphereNum                   ; <i32> [#uses=1]
  %add82 = add nsw i32 %tmp81, 3                  ; <i32> [#uses=1]
  %tmp83 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx84 = getelementptr inbounds float addrspace(1)* %tmp83, i32 %add82 ; <float addrspace(1)*> [#uses=1]
  %tmp85 = load float addrspace(1)* %arrayidx84   ; <float> [#uses=1]
  %vecinit86 = insertelement <4 x float> %vecinit80, float %tmp85, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit86, <4 x float>* %.compoundliteral64
  %tmp87 = load <4 x float>* %.compoundliteral64  ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp87, <4 x float>* %spherePos
  %tmp88 = load i32* %sphereNum                   ; <i32> [#uses=1]
  %add89 = add nsw i32 %tmp88, 4                  ; <i32> [#uses=1]
  %tmp90 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx91 = getelementptr inbounds float addrspace(1)* %tmp90, i32 %add89 ; <float addrspace(1)*> [#uses=1]
  %tmp92 = load float addrspace(1)* %arrayidx91   ; <float> [#uses=1]
  store float %tmp92, float* %sphereRadius
  %tmp94 = load i32* %sphereNum                   ; <i32> [#uses=1]
  %add95 = add nsw i32 %tmp94, 5                  ; <i32> [#uses=1]
  %tmp96 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx97 = getelementptr inbounds float addrspace(1)* %tmp96, i32 %add95 ; <float addrspace(1)*> [#uses=1]
  %tmp98 = load float addrspace(1)* %arrayidx97   ; <float> [#uses=1]
  %vecinit99 = insertelement <4 x float> undef, float %tmp98, i32 0 ; <<4 x float>> [#uses=1]
  %tmp100 = load i32* %sphereNum                  ; <i32> [#uses=1]
  %add101 = add nsw i32 %tmp100, 6                ; <i32> [#uses=1]
  %tmp102 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx103 = getelementptr inbounds float addrspace(1)* %tmp102, i32 %add101 ; <float addrspace(1)*> [#uses=1]
  %tmp104 = load float addrspace(1)* %arrayidx103 ; <float> [#uses=1]
  %vecinit105 = insertelement <4 x float> %vecinit99, float %tmp104, i32 1 ; <<4 x float>> [#uses=1]
  %tmp106 = load i32* %sphereNum                  ; <i32> [#uses=1]
  %add107 = add nsw i32 %tmp106, 7                ; <i32> [#uses=1]
  %tmp108 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx109 = getelementptr inbounds float addrspace(1)* %tmp108, i32 %add107 ; <float addrspace(1)*> [#uses=1]
  %tmp110 = load float addrspace(1)* %arrayidx109 ; <float> [#uses=1]
  %vecinit111 = insertelement <4 x float> %vecinit105, float %tmp110, i32 2 ; <<4 x float>> [#uses=1]
  %tmp112 = load i32* %sphereNum                  ; <i32> [#uses=1]
  %add113 = add nsw i32 %tmp112, 8                ; <i32> [#uses=1]
  %tmp114 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx115 = getelementptr inbounds float addrspace(1)* %tmp114, i32 %add113 ; <float addrspace(1)*> [#uses=1]
  %tmp116 = load float addrspace(1)* %arrayidx115 ; <float> [#uses=1]
  %vecinit117 = insertelement <4 x float> %vecinit111, float %tmp116, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit117, <4 x float>* %.compoundliteral93
  %tmp118 = load <4 x float>* %.compoundliteral93 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp118, <4 x float>* %sphereColor
  %tmp120 = load i32* %sphereNum                  ; <i32> [#uses=1]
  %add121 = add nsw i32 %tmp120, 9                ; <i32> [#uses=1]
  %tmp122 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx123 = getelementptr inbounds float addrspace(1)* %tmp122, i32 %add121 ; <float addrspace(1)*> [#uses=1]
  %tmp124 = load float addrspace(1)* %arrayidx123 ; <float> [#uses=1]
  %vecinit125 = insertelement <4 x float> undef, float %tmp124, i32 0 ; <<4 x float>> [#uses=1]
  %tmp126 = load i32* %sphereNum                  ; <i32> [#uses=1]
  %add127 = add nsw i32 %tmp126, 10               ; <i32> [#uses=1]
  %tmp128 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx129 = getelementptr inbounds float addrspace(1)* %tmp128, i32 %add127 ; <float addrspace(1)*> [#uses=1]
  %tmp130 = load float addrspace(1)* %arrayidx129 ; <float> [#uses=1]
  %vecinit131 = insertelement <4 x float> %vecinit125, float %tmp130, i32 1 ; <<4 x float>> [#uses=1]
  %tmp132 = load i32* %sphereNum                  ; <i32> [#uses=1]
  %add133 = add nsw i32 %tmp132, 11               ; <i32> [#uses=1]
  %tmp134 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx135 = getelementptr inbounds float addrspace(1)* %tmp134, i32 %add133 ; <float addrspace(1)*> [#uses=1]
  %tmp136 = load float addrspace(1)* %arrayidx135 ; <float> [#uses=1]
  %vecinit137 = insertelement <4 x float> %vecinit131, float %tmp136, i32 2 ; <<4 x float>> [#uses=1]
  %tmp138 = load i32* %sphereNum                  ; <i32> [#uses=1]
  %add139 = add nsw i32 %tmp138, 12               ; <i32> [#uses=1]
  %tmp140 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx141 = getelementptr inbounds float addrspace(1)* %tmp140, i32 %add139 ; <float addrspace(1)*> [#uses=1]
  %tmp142 = load float addrspace(1)* %arrayidx141 ; <float> [#uses=1]
  %vecinit143 = insertelement <4 x float> %vecinit137, float %tmp142, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %vecinit143, <4 x float>* %.compoundliteral119
  %tmp144 = load <4 x float>* %.compoundliteral119 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp144, <4 x float>* %sphereMaterial
  %tmp145 = load <4 x float>* %hitPoint           ; <<4 x float>> [#uses=1]
  %tmp146 = load <4 x float>* %spherePos          ; <<4 x float>> [#uses=1]
  %sub147 = fsub <4 x float> %tmp145, %tmp146     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub147, <4 x float>* %sphereHit
  %tmp148 = load <4 x float>* %sphereHit          ; <<4 x float>> [#uses=1]
  %tmp149 = load float* %sphereRadius             ; <float> [#uses=1]
  %tmp150 = insertelement <4 x float> undef, float %tmp149, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp150, <4 x float> %tmp150, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp151 = fcmp oeq <4 x float> zeroinitializer, %splat ; <<4 x i1>> [#uses=1]
  %sel152 = select <4 x i1> %cmp151, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat ; <<4 x float>> [#uses=0]
  %div153 = fdiv <4 x float> %tmp148, %splat      ; <<4 x float>> [#uses=1]
  store <4 x float> %div153, <4 x float>* %n
  %tmp154 = load <4 x float>* %lightPos.addr      ; <<4 x float>> [#uses=1]
  %tmp155 = load <4 x float>* %hitPoint           ; <<4 x float>> [#uses=1]
  %sub156 = fsub <4 x float> %tmp154, %tmp155     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub156, <4 x float>* %lightVector
  %tmp157 = load <4 x float>* %lightVector        ; <<4 x float>> [#uses=1]
  %call158 = call float @_Z6lengthDv4_f(<4 x float> %tmp157) ; <float> [#uses=1]
  store float %call158, float* %lightVectorLen
  %tmp159 = load <4 x float>* %lightVector        ; <<4 x float>> [#uses=1]
  %tmp160 = load float* %lightVectorLen           ; <float> [#uses=1]
  %tmp161 = insertelement <4 x float> undef, float %tmp160, i32 0 ; <<4 x float>> [#uses=2]
  %splat162 = shufflevector <4 x float> %tmp161, <4 x float> %tmp161, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %cmp163 = fcmp oeq <4 x float> zeroinitializer, %splat162 ; <<4 x i1>> [#uses=1]
  %sel164 = select <4 x i1> %cmp163, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat162 ; <<4 x float>> [#uses=0]
  %div165 = fdiv <4 x float> %tmp159, %splat162   ; <<4 x float>> [#uses=1]
  store <4 x float> %div165, <4 x float>* %l
  %tmp166 = load <4 x float>* %hitPoint           ; <<4 x float>> [#uses=1]
  %tmp167 = load <4 x float>* %l                  ; <<4 x float>> [#uses=1]
  %tmp168 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %tmp169 = load i32* %numberOfSpheres.addr       ; <i32> [#uses=1]
  %tmp170 = load i32* %numberOfSphereParameters.addr ; <i32> [#uses=1]
  call void @shootRay(<4 x float> %tmp166, <4 x float> %tmp167, i32* %shadowTest, <4 x float>* %temp, float* %t, i32* %temp2, float addrspace(1)* %tmp168, i32 %tmp169, i32 %tmp170)
  %tmp171 = load i32* %shadowTest                 ; <i32> [#uses=1]
  %cmp172 = icmp eq i32 %tmp171, 0                ; <i1> [#uses=1]
  br i1 %cmp172, label %if.then174, label %if.else

if.then174:                                       ; preds = %if.then
  store i32 1, i32* %shadowTest
  br label %if.end180

if.else:                                          ; preds = %if.then
  %tmp175 = load float* %t                        ; <float> [#uses=1]
  %tmp176 = load float* %lightVectorLen           ; <float> [#uses=1]
  %cmp177 = fcmp olt float %tmp175, %tmp176       ; <i1> [#uses=1]
  br i1 %cmp177, label %if.then179, label %if.end

if.then179:                                       ; preds = %if.else
  store i32 0, i32* %shadowTest
  br label %if.end

if.end:                                           ; preds = %if.then179, %if.else
  br label %if.end180

if.end180:                                        ; preds = %if.end, %if.then174
  %tmp181 = load <4 x float>* %l                  ; <<4 x float>> [#uses=1]
  %tmp182 = load <4 x float>* %n                  ; <<4 x float>> [#uses=1]
  %call183 = call float @_Z3dotDv4_fS_(<4 x float> %tmp181, <4 x float> %tmp182) ; <float> [#uses=1]
  store float %call183, float* %diffuse
  %tmp184 = load <4 x float>* %l                  ; <<4 x float>> [#uses=1]
  %tmp185 = load float* %diffuse                  ; <float> [#uses=1]
  %mul186 = fmul float 2.000000e+000, %tmp185     ; <float> [#uses=1]
  %tmp187 = insertelement <4 x float> undef, float %mul186, i32 0 ; <<4 x float>> [#uses=2]
  %splat188 = shufflevector <4 x float> %tmp187, <4 x float> %tmp187, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp189 = load <4 x float>* %n                  ; <<4 x float>> [#uses=1]
  %mul190 = fmul <4 x float> %splat188, %tmp189   ; <<4 x float>> [#uses=1]
  %sub191 = fsub <4 x float> %tmp184, %mul190     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub191, <4 x float>* %lReflect
  %tmp192 = load <4 x float>* %dir                ; <<4 x float>> [#uses=1]
  %tmp193 = load <4 x float>* %lReflect           ; <<4 x float>> [#uses=1]
  %call194 = call float @_Z3dotDv4_fS_(<4 x float> %tmp192, <4 x float> %tmp193) ; <float> [#uses=1]
  store float %call194, float* %specular
  %tmp195 = load float* %diffuse                  ; <float> [#uses=1]
  %call196 = call float @_Z3maxff(float %tmp195, float 0.000000e+000) ; <float> [#uses=1]
  store float %call196, float* %diffuse
  %tmp197 = load float* %specular                 ; <float> [#uses=1]
  %call198 = call float @_Z3maxff(float %tmp197, float 0.000000e+000) ; <float> [#uses=1]
  %tmp199 = load float addrspace(1)* @SPECULAR_EXPONENT ; <float> [#uses=1]
  %call200 = call float @_Z3powff(float %call198, float %tmp199) ; <float> [#uses=1]
  store float %call200, float* %specular
  %tmp201 = load i32* %sphereNum                  ; <i32> [#uses=1]
  %cmp202 = icmp eq i32 %tmp201, 13               ; <i1> [#uses=1]
  br i1 %cmp202, label %if.then204, label %if.end251

if.then204:                                       ; preds = %if.end180
  store <4 x float> <float 1.000000e+000, float 0.000000e+000, float 0.000000e+000, float 0.000000e+000>, <4 x float>* %.compoundliteral205
  %tmp206 = load <4 x float>* %.compoundliteral205 ; <<4 x float>> [#uses=1]
  %tmp207 = load <4 x float>* %n                  ; <<4 x float>> [#uses=1]
  %call208 = call float @_Z3dotDv4_fS_(<4 x float> %tmp206, <4 x float> %tmp207) ; <float> [#uses=1]
  %neg209 = fsub float -0.000000e+000, %call208   ; <float> [#uses=1]
  %call210 = call float @_Z4acosf(float %neg209)  ; <float> [#uses=1]
  store float %call210, float* %phi
  store <4 x float> <float 0.000000e+000, float 0.000000e+000, float 1.000000e+000, float 0.000000e+000>, <4 x float>* %.compoundliteral212
  %tmp213 = load <4 x float>* %.compoundliteral212 ; <<4 x float>> [#uses=1]
  %tmp214 = load <4 x float>* %n                  ; <<4 x float>> [#uses=1]
  %call215 = call float @_Z3dotDv4_fS_(<4 x float> %tmp213, <4 x float> %tmp214) ; <float> [#uses=1]
  %tmp216 = load float* %phi                      ; <float> [#uses=1]
  %call217 = call float @_Z3sinf(float %tmp216)   ; <float> [#uses=3]
  %cmp218 = fcmp oeq float 0.000000e+000, %call217 ; <i1> [#uses=1]
  %sel219 = select i1 %cmp218, float 1.000000e+000, float %call217 ; <float> [#uses=0]
  %div220 = fdiv float %call215, %call217         ; <float> [#uses=1]
  %call221 = call float @_Z4acosf(float %div220)  ; <float> [#uses=1]
  %tmp222 = load float addrspace(1)* @PI          ; <float> [#uses=1]
  %mul223 = fmul float 2.000000e+000, %tmp222     ; <float> [#uses=3]
  %cmp224 = fcmp oeq float 0.000000e+000, %mul223 ; <i1> [#uses=1]
  %sel225 = select i1 %cmp224, float 1.000000e+000, float %mul223 ; <float> [#uses=0]
  %div226 = fdiv float %call221, %mul223          ; <float> [#uses=1]
  %vecinit227 = insertelement <2 x float> undef, float %div226, i32 0 ; <<2 x float>> [#uses=1]
  %tmp228 = load float* %phi                      ; <float> [#uses=1]
  %tmp229 = load float addrspace(1)* @PI          ; <float> [#uses=3]
  %cmp230 = fcmp oeq float 0.000000e+000, %tmp229 ; <i1> [#uses=1]
  %sel231 = select i1 %cmp230, float 1.000000e+000, float %tmp229 ; <float> [#uses=0]
  %div232 = fdiv float %tmp228, %tmp229           ; <float> [#uses=1]
  %vecinit233 = insertelement <2 x float> %vecinit227, float %div232, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %vecinit233, <2 x float>* %.compoundliteral211
  %tmp234 = load <2 x float>* %.compoundliteral211 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp234, <2 x float>* %uv
  %tmp235 = load <2 x float>* %uv                 ; <<2 x float>> [#uses=1]
  %tmp236 = extractelement <2 x float> %tmp235, i32 0 ; <float> [#uses=1]
  %mul237 = fmul float %tmp236, 2.000000e+003     ; <float> [#uses=1]
  %call238 = call float @_Z5floorf(float %mul237) ; <float> [#uses=1]
  %tmp239 = load <2 x float>* %uv                 ; <<2 x float>> [#uses=1]
  %tmp240 = extractelement <2 x float> %tmp239, i32 1 ; <float> [#uses=1]
  %mul241 = fmul float %tmp240, 2.000000e+003     ; <float> [#uses=1]
  %call242 = call float @_Z5floorf(float %mul241) ; <float> [#uses=1]
  %add243 = fadd float %call238, %call242         ; <float> [#uses=1]
  %call244 = call float @_Z4fmodff(float %add243, float 2.000000e+000) ; <float> [#uses=1]
  %cmp245 = fcmp oeq float %call244, 0.000000e+000 ; <i1> [#uses=1]
  %cond = select i1 %cmp245, float 5.000000e-001, float 1.000000e+000 ; <float> [#uses=1]
  %tmp247 = insertelement <4 x float> undef, float %cond, i32 0 ; <<4 x float>> [#uses=2]
  %splat248 = shufflevector <4 x float> %tmp247, <4 x float> %tmp247, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp249 = load <4 x float>* %sphereColor        ; <<4 x float>> [#uses=1]
  %mul250 = fmul <4 x float> %tmp249, %splat248   ; <<4 x float>> [#uses=1]
  store <4 x float> %mul250, <4 x float>* %sphereColor
  br label %if.end251

if.end251:                                        ; preds = %if.then204, %if.end180
  %tmp252 = load <4 x float>* %sphereMaterial     ; <<4 x float>> [#uses=1]
  %tmp253 = extractelement <4 x float> %tmp252, i32 0 ; <float> [#uses=1]
  %tmp254 = load i32* %shadowTest                 ; <i32> [#uses=1]
  %conv255 = sitofp i32 %tmp254 to float          ; <float> [#uses=1]
  %tmp256 = load float* %diffuse                  ; <float> [#uses=1]
  %tmp257 = load <4 x float>* %sphereMaterial     ; <<4 x float>> [#uses=1]
  %tmp258 = extractelement <4 x float> %tmp257, i32 1 ; <float> [#uses=1]
  %mul259 = fmul float %tmp256, %tmp258           ; <float> [#uses=1]
  %tmp260 = load float* %specular                 ; <float> [#uses=1]
  %tmp261 = load <4 x float>* %sphereMaterial     ; <<4 x float>> [#uses=1]
  %tmp262 = extractelement <4 x float> %tmp261, i32 2 ; <float> [#uses=1]
  %mul263 = fmul float %tmp260, %tmp262           ; <float> [#uses=1]
  %add264 = fadd float %mul259, %mul263           ; <float> [#uses=1]
  %mul265 = fmul float %conv255, %add264          ; <float> [#uses=1]
  %add266 = fadd float %tmp253, %mul265           ; <float> [#uses=1]
  store float %add266, float* %lightVal
  %tmp267 = load <4 x float>* %colorScale         ; <<4 x float>> [#uses=1]
  %tmp268 = load float* %lightVal                 ; <float> [#uses=1]
  %tmp269 = insertelement <4 x float> undef, float %tmp268, i32 0 ; <<4 x float>> [#uses=2]
  %splat270 = shufflevector <4 x float> %tmp269, <4 x float> %tmp269, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul271 = fmul <4 x float> %tmp267, %splat270   ; <<4 x float>> [#uses=1]
  %tmp272 = load <4 x float>* %sphereColor        ; <<4 x float>> [#uses=1]
  %mul273 = fmul <4 x float> %mul271, %tmp272     ; <<4 x float>> [#uses=1]
  %tmp274 = load <4 x float>* %dst                ; <<4 x float>> [#uses=1]
  %add275 = fadd <4 x float> %tmp274, %mul273     ; <<4 x float>> [#uses=1]
  store <4 x float> %add275, <4 x float>* %dst
  %tmp276 = load <4 x float>* %dst                ; <<4 x float>> [#uses=1]
  %call277 = call <4 x float> @_Z5clampDv4_fff(<4 x float> %tmp276, float 0.000000e+000, float 1.000000e+000) ; <<4 x float>> [#uses=1]
  store <4 x float> %call277, <4 x float>* %dst
  %tmp278 = load <4 x float>* %sphereMaterial     ; <<4 x float>> [#uses=1]
  %tmp279 = extractelement <4 x float> %tmp278, i32 3 ; <float> [#uses=1]
  %cmp280 = fcmp ogt float %tmp279, 0.000000e+000 ; <i1> [#uses=1]
  br i1 %cmp280, label %if.then282, label %if.else306

if.then282:                                       ; preds = %if.end251
  %tmp283 = load <4 x float>* %dir                ; <<4 x float>> [#uses=1]
  %tmp284 = load <4 x float>* %dir                ; <<4 x float>> [#uses=1]
  %tmp285 = load <4 x float>* %n                  ; <<4 x float>> [#uses=1]
  %call286 = call float @_Z3dotDv4_fS_(<4 x float> %tmp284, <4 x float> %tmp285) ; <float> [#uses=1]
  %mul287 = fmul float 2.000000e+000, %call286    ; <float> [#uses=1]
  %tmp288 = insertelement <4 x float> undef, float %mul287, i32 0 ; <<4 x float>> [#uses=2]
  %splat289 = shufflevector <4 x float> %tmp288, <4 x float> %tmp288, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp290 = load <4 x float>* %n                  ; <<4 x float>> [#uses=1]
  %mul291 = fmul <4 x float> %splat289, %tmp290   ; <<4 x float>> [#uses=1]
  %sub292 = fsub <4 x float> %tmp283, %mul291     ; <<4 x float>> [#uses=1]
  store <4 x float> %sub292, <4 x float>* %dirReflect
  %tmp293 = load <4 x float>* %dirReflect         ; <<4 x float>> [#uses=1]
  %call294 = call <4 x float> @_Z9normalizeDv4_f(<4 x float> %tmp293) ; <<4 x float>> [#uses=1]
  store <4 x float> %call294, <4 x float>* %dirReflect
  %tmp295 = load <4 x float>* %hitPoint           ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp295, <4 x float>* %origin
  %tmp296 = load <4 x float>* %dirReflect         ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp296, <4 x float>* %dir
  %tmp297 = load i32* %rayShots                   ; <i32> [#uses=1]
  %dec = add nsw i32 %tmp297, -1                  ; <i32> [#uses=1]
  store i32 %dec, i32* %rayShots
  %tmp298 = load <4 x float>* %sphereMaterial     ; <<4 x float>> [#uses=1]
  %tmp299 = extractelement <4 x float> %tmp298, i32 3 ; <float> [#uses=1]
  %tmp300 = insertelement <4 x float> undef, float %tmp299, i32 0 ; <<4 x float>> [#uses=2]
  %splat301 = shufflevector <4 x float> %tmp300, <4 x float> %tmp300, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp302 = load <4 x float>* %sphereColor        ; <<4 x float>> [#uses=1]
  %mul303 = fmul <4 x float> %splat301, %tmp302   ; <<4 x float>> [#uses=1]
  %tmp304 = load <4 x float>* %colorScale         ; <<4 x float>> [#uses=1]
  %mul305 = fmul <4 x float> %tmp304, %mul303     ; <<4 x float>> [#uses=1]
  store <4 x float> %mul305, <4 x float>* %colorScale
  br label %if.end307

if.else306:                                       ; preds = %if.end251
  store i32 0, i32* %rayShots
  br label %if.end307

if.end307:                                        ; preds = %if.else306, %if.then282
  br label %if.end309

if.else308:                                       ; preds = %while.body
  store i32 0, i32* %rayShots
  br label %if.end309

if.end309:                                        ; preds = %if.else308, %if.end307
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %tmp310 = load <4 x float>* %dst                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp310, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z9normalizeDv4_f(<4 x float>)

declare float @_Z6lengthDv4_f(<4 x float>)

declare float @_Z3maxff(float, float)

declare float @_Z4acosf(float)

declare float @_Z4fmodff(float, float)

declare float @_Z5floorf(float)

declare <4 x float> @_Z5clampDv4_fff(<4 x float>, float, float)

define <4 x float> @dummy() nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %result = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %.compoundliteral = alloca <4 x float>, align 16 ; <<4 x float>*> [#uses=2]
  store <4 x float> <float 5.000000e-001, float 0.000000e+000, float 0.000000e+000, float 1.000000e+000>, <4 x float>* %.compoundliteral
  %tmp = load <4 x float>* %.compoundliteral      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp, <4 x float>* %result
  %tmp1 = load <4 x float>* %result               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp1, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

define void @wlraytracer(<4 x float>* %pOutput, float addrspace(1)* %pSphereArray, i32 %numberOfSpheres, i32 %numberOfSphereParameters, i32 %maxRayShots, i32 %renderWidth, i32 %renderHeight, float %viewPlaneDistance, <4 x float> %lightPos) nounwind {
entry:
  %pOutput.addr = alloca <4 x float>*, align 4    ; <<4 x float>**> [#uses=2]
  %pSphereArray.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %numberOfSpheres.addr = alloca i32, align 4     ; <i32*> [#uses=2]
  %numberOfSphereParameters.addr = alloca i32, align 4 ; <i32*> [#uses=2]
  %maxRayShots.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %renderWidth.addr = alloca i32, align 4         ; <i32*> [#uses=7]
  %renderHeight.addr = alloca i32, align 4        ; <i32*> [#uses=5]
  %viewPlaneDistance.addr = alloca float, align 4 ; <float*> [#uses=2]
  %lightPos.addr = alloca <4 x float>, align 16   ; <<4 x float>*> [#uses=2]
  %global_size = alloca i32, align 4              ; <i32*> [#uses=4]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=3]
  %count = alloca i32, align 4                    ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=6]
  %startingColumn = alloca i32, align 4           ; <i32*> [#uses=3]
  %column = alloca i32, align 4                   ; <i32*> [#uses=6]
  %thePixel = alloca <2 x i32>, align 8           ; <<2 x i32>*> [#uses=5]
  %i = alloca i32, align 4                        ; <i32*> [#uses=4]
  store <4 x float>* %pOutput, <4 x float>** %pOutput.addr
  store float addrspace(1)* %pSphereArray, float addrspace(1)** %pSphereArray.addr
  store i32 %numberOfSpheres, i32* %numberOfSpheres.addr
  store i32 %numberOfSphereParameters, i32* %numberOfSphereParameters.addr
  store i32 %maxRayShots, i32* %maxRayShots.addr
  store i32 %renderWidth, i32* %renderWidth.addr
  store i32 %renderHeight, i32* %renderHeight.addr
  store float %viewPlaneDistance, float* %viewPlaneDistance.addr
  store <4 x float> %lightPos, <4 x float>* %lightPos.addr
  %call = call i32 @get_global_size(i32 0)        ; <i32> [#uses=1]
  store i32 %call, i32* %global_size
  %call1 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call1, i32* %global_id
  %tmp = load i32* %renderWidth.addr              ; <i32> [#uses=1]
  %tmp2 = load i32* %renderHeight.addr            ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp2                      ; <i32> [#uses=1]
  %tmp3 = load i32* %global_size                  ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp3                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp3         ; <i32> [#uses=1]
  %div = udiv i32 %mul, %sel                      ; <i32> [#uses=1]
  store i32 %div, i32* %count
  %tmp5 = load i32* %renderWidth.addr             ; <i32> [#uses=1]
  %tmp6 = load i32* %renderHeight.addr            ; <i32> [#uses=1]
  %mul7 = mul i32 %tmp5, %tmp6                    ; <i32> [#uses=1]
  %tmp8 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul9 = mul i32 %mul7, %tmp8                    ; <i32> [#uses=1]
  %tmp10 = load i32* %global_size                 ; <i32> [#uses=2]
  %cmp11 = icmp eq i32 0, %tmp10                  ; <i1> [#uses=1]
  %sel12 = select i1 %cmp11, i32 1, i32 %tmp10    ; <i32> [#uses=1]
  %div13 = udiv i32 %mul9, %sel12                 ; <i32> [#uses=1]
  store i32 %div13, i32* %index
  %tmp15 = load i32* %renderHeight.addr           ; <i32> [#uses=1]
  %tmp16 = load i32* %global_id                   ; <i32> [#uses=1]
  %mul17 = mul i32 %tmp15, %tmp16                 ; <i32> [#uses=1]
  %tmp18 = load i32* %global_size                 ; <i32> [#uses=2]
  %cmp19 = icmp eq i32 0, %tmp18                  ; <i1> [#uses=1]
  %sel20 = select i1 %cmp19, i32 1, i32 %tmp18    ; <i32> [#uses=1]
  %div21 = udiv i32 %mul17, %sel20                ; <i32> [#uses=1]
  store i32 %div21, i32* %row
  %tmp23 = load i32* %index                       ; <i32> [#uses=1]
  %tmp24 = load i32* %row                         ; <i32> [#uses=1]
  %tmp25 = load i32* %renderWidth.addr            ; <i32> [#uses=1]
  %mul26 = mul i32 %tmp24, %tmp25                 ; <i32> [#uses=1]
  %sub = sub i32 %tmp23, %mul26                   ; <i32> [#uses=1]
  store i32 %sub, i32* %startingColumn
  %tmp28 = load i32* %startingColumn              ; <i32> [#uses=1]
  store i32 %tmp28, i32* %column
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp31 = load i32* %i                           ; <i32> [#uses=1]
  %tmp32 = load i32* %count                       ; <i32> [#uses=1]
  %cmp33 = icmp ult i32 %tmp31, %tmp32            ; <i1> [#uses=1]
  br i1 %cmp33, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp34 = load i32* %column                      ; <i32> [#uses=1]
  %tmp35 = load <2 x i32>* %thePixel              ; <<2 x i32>> [#uses=1]
  %tmp36 = insertelement <2 x i32> %tmp35, i32 %tmp34, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp36, <2 x i32>* %thePixel
  %tmp37 = load i32* %row                         ; <i32> [#uses=1]
  %tmp38 = load <2 x i32>* %thePixel              ; <<2 x i32>> [#uses=1]
  %tmp39 = insertelement <2 x i32> %tmp38, i32 %tmp37, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp39, <2 x i32>* %thePixel
  %tmp40 = load float addrspace(1)** %pSphereArray.addr ; <float addrspace(1)*> [#uses=1]
  %tmp41 = load <2 x i32>* %thePixel              ; <<2 x i32>> [#uses=1]
  %tmp42 = load i32* %numberOfSpheres.addr        ; <i32> [#uses=1]
  %tmp43 = load i32* %numberOfSphereParameters.addr ; <i32> [#uses=1]
  %tmp44 = load i32* %maxRayShots.addr            ; <i32> [#uses=1]
  %tmp45 = load i32* %renderWidth.addr            ; <i32> [#uses=1]
  %tmp46 = load i32* %renderHeight.addr           ; <i32> [#uses=1]
  %tmp47 = load float* %viewPlaneDistance.addr    ; <float> [#uses=1]
  %tmp48 = load <4 x float>* %lightPos.addr       ; <<4 x float>> [#uses=1]
  %call49 = call <4 x float> @evaluatePixel(float addrspace(1)* %tmp40, <2 x i32> %tmp41, i32 %tmp42, i32 %tmp43, i32 %tmp44, i32 %tmp45, i32 %tmp46, float %tmp47, <4 x float> %tmp48) ; <<4 x float>> [#uses=1]
  %tmp50 = load i32* %row                         ; <i32> [#uses=1]
  %tmp51 = load i32* %renderWidth.addr            ; <i32> [#uses=1]
  %mul52 = mul i32 %tmp50, %tmp51                 ; <i32> [#uses=1]
  %tmp53 = load i32* %column                      ; <i32> [#uses=1]
  %add = add i32 %mul52, %tmp53                   ; <i32> [#uses=1]
  %tmp54 = load <4 x float>** %pOutput.addr       ; <<4 x float>*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float>* %tmp54, i32 %add ; <<4 x float>*> [#uses=1]
  store <4 x float> %call49, <4 x float>* %arrayidx
  %tmp55 = load i32* %column                      ; <i32> [#uses=1]
  %inc = add i32 %tmp55, 1                        ; <i32> [#uses=2]
  store i32 %inc, i32* %column
  %tmp56 = load i32* %renderWidth.addr            ; <i32> [#uses=1]
  %cmp57 = icmp uge i32 %inc, %tmp56              ; <i1> [#uses=1]
  br i1 %cmp57, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %tmp58 = load i32* %startingColumn              ; <i32> [#uses=1]
  store i32 %tmp58, i32* %column
  %tmp59 = load i32* %row                         ; <i32> [#uses=1]
  %inc60 = add i32 %tmp59, 1                      ; <i32> [#uses=1]
  store i32 %inc60, i32* %row
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %tmp61 = load i32* %i                           ; <i32> [#uses=1]
  %inc62 = add nsw i32 %tmp61, 1                  ; <i32> [#uses=1]
  store i32 %inc62, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare i32 @get_global_size(i32)

declare i32 @get_global_id(i32)
