; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\intel_bilateral2D.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct.anon = type <{ i32, float, float, i32, i32 }>

@opencl_evaluateDependents_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_evaluateDependents_parameters = appending global [86 x i8] c"float2 const, float const, uint const, kernelArgs __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[86 x i8]*> [#uses=1]
@opencl_intel_bilateral2D_2D_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_intel_bilateral2D_2D_parameters = appending global [145 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, int, int, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[145 x i8]*> [#uses=1]
@opencl_intel_bilateral2DScalar_2D_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_intel_bilateral2DScalar_2D_parameters = appending global [145 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, int, int, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[145 x i8]*> [#uses=1]
@opencl_intel_bilateral2D_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_intel_bilateral2D_parameters = appending global [157 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, int, int, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[157 x i8]*> [#uses=1]
@opencl_intel_bilateral2DScalar_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_intel_bilateral2DScalar_parameters = appending global [157 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const, int, int, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[157 x i8]*> [#uses=1]
@opencl_metadata = appending global [5 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<2 x float>, float, i32, %struct.anon addrspace(1)*)* @evaluateDependents to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_evaluateDependents_locals to i8*), i8* getelementptr inbounds ([86 x i8]* @opencl_evaluateDependents_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, %struct.anon addrspace(2)*)* @intel_bilateral2D_2D to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_intel_bilateral2D_2D_locals to i8*), i8* getelementptr inbounds ([145 x i8]* @opencl_intel_bilateral2D_2D_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, %struct.anon addrspace(2)*)* @intel_bilateral2DScalar_2D to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_intel_bilateral2DScalar_2D_locals to i8*), i8* getelementptr inbounds ([145 x i8]* @opencl_intel_bilateral2DScalar_2D_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32, %struct.anon addrspace(2)*)* @intel_bilateral2D to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_intel_bilateral2D_locals to i8*), i8* getelementptr inbounds ([157 x i8]* @opencl_intel_bilateral2D_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32, %struct.anon addrspace(2)*)* @intel_bilateral2DScalar to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_intel_bilateral2DScalar_locals to i8*), i8* getelementptr inbounds ([157 x i8]* @opencl_intel_bilateral2DScalar_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[5 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define void @evaluateDependents(<2 x float> %spatialSigma, float %rangeSigma, i32 %colorize, %struct.anon addrspace(1)* %pArgs) nounwind {
entry:
  %spatialSigma.addr = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=7]
  %rangeSigma.addr = alloca float, align 4        ; <float*> [#uses=4]
  %colorize.addr = alloca i32, align 4            ; <i32*> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(1)*, align 4 ; <%struct.anon addrspace(1)**> [#uses=8]
  store <2 x float> %spatialSigma, <2 x float>* %spatialSigma.addr
  store float %rangeSigma, float* %rangeSigma.addr
  store i32 %colorize, i32* %colorize.addr
  store %struct.anon addrspace(1)* %pArgs, %struct.anon addrspace(1)** %pArgs.addr
  %tmp = load i32* %colorize.addr                 ; <i32> [#uses=1]
  %tmp1 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp2 = getelementptr inbounds %struct.anon addrspace(1)* %tmp1, i32 0, i32 0 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %tmp, i32 addrspace(1)* %tmp2
  %tmp3 = load <2 x float>* %spatialSigma.addr    ; <<2 x float>> [#uses=1]
  %tmp4 = extractelement <2 x float> %tmp3, i32 0 ; <float> [#uses=1]
  %cmp = fcmp ogt float %tmp4, 0.000000e+000      ; <i1> [#uses=1]
  br i1 %cmp, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %entry
  %tmp5 = load <2 x float>* %spatialSigma.addr    ; <<2 x float>> [#uses=1]
  %tmp6 = extractelement <2 x float> %tmp5, i32 1 ; <float> [#uses=1]
  %cmp7 = fcmp ogt float %tmp6, 0.000000e+000     ; <i1> [#uses=1]
  br i1 %cmp7, label %if.then, label %if.else

if.then:                                          ; preds = %land.lhs.true
  %tmp8 = load <2 x float>* %spatialSigma.addr    ; <<2 x float>> [#uses=1]
  %tmp9 = extractelement <2 x float> %tmp8, i32 0 ; <float> [#uses=1]
  %mul = fmul float 2.000000e+000, %tmp9          ; <float> [#uses=1]
  %tmp10 = load <2 x float>* %spatialSigma.addr   ; <<2 x float>> [#uses=1]
  %tmp11 = extractelement <2 x float> %tmp10, i32 1 ; <float> [#uses=1]
  %mul12 = fmul float %mul, %tmp11                ; <float> [#uses=3]
  %cmp13 = fcmp oeq float 0.000000e+000, %mul12   ; <i1> [#uses=1]
  %sel = select i1 %cmp13, float 1.000000e+000, float %mul12 ; <float> [#uses=0]
  %div = fdiv float 1.000000e+000, %mul12         ; <float> [#uses=1]
  %tmp14 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp15 = getelementptr inbounds %struct.anon addrspace(1)* %tmp14, i32 0, i32 1 ; <float addrspace(1)*> [#uses=1]
  store float %div, float addrspace(1)* %tmp15
  br label %if.end

if.else:                                          ; preds = %land.lhs.true, %entry
  %tmp16 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp17 = getelementptr inbounds %struct.anon addrspace(1)* %tmp16, i32 0, i32 1 ; <float addrspace(1)*> [#uses=1]
  store float 1.000000e+000, float addrspace(1)* %tmp17
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %tmp18 = load float* %rangeSigma.addr           ; <float> [#uses=1]
  %cmp19 = fcmp ogt float %tmp18, 0.000000e+000   ; <i1> [#uses=1]
  br i1 %cmp19, label %if.then20, label %if.else30

if.then20:                                        ; preds = %if.end
  %tmp21 = load float* %rangeSigma.addr           ; <float> [#uses=1]
  %mul22 = fmul float 2.000000e+000, %tmp21       ; <float> [#uses=1]
  %tmp23 = load float* %rangeSigma.addr           ; <float> [#uses=1]
  %mul24 = fmul float %mul22, %tmp23              ; <float> [#uses=3]
  %cmp25 = fcmp oeq float 0.000000e+000, %mul24   ; <i1> [#uses=1]
  %sel26 = select i1 %cmp25, float 1.000000e+000, float %mul24 ; <float> [#uses=0]
  %div27 = fdiv float 1.000000e+000, %mul24       ; <float> [#uses=1]
  %tmp28 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp29 = getelementptr inbounds %struct.anon addrspace(1)* %tmp28, i32 0, i32 2 ; <float addrspace(1)*> [#uses=1]
  store float %div27, float addrspace(1)* %tmp29
  br label %if.end33

if.else30:                                        ; preds = %if.end
  %tmp31 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp32 = getelementptr inbounds %struct.anon addrspace(1)* %tmp31, i32 0, i32 2 ; <float addrspace(1)*> [#uses=1]
  store float 1.000000e+000, float addrspace(1)* %tmp32
  br label %if.end33

if.end33:                                         ; preds = %if.else30, %if.then20
  %tmp34 = load <2 x float>* %spatialSigma.addr   ; <<2 x float>> [#uses=1]
  %tmp35 = extractelement <2 x float> %tmp34, i32 0 ; <float> [#uses=1]
  %mul36 = fmul float 0x40012B0200000000, %tmp35  ; <float> [#uses=1]
  %call = call float @_Z4ceilf(float %mul36)      ; <float> [#uses=1]
  %conv = fptosi float %call to i32               ; <i32> [#uses=1]
  %tmp37 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp38 = getelementptr inbounds %struct.anon addrspace(1)* %tmp37, i32 0, i32 3 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %conv, i32 addrspace(1)* %tmp38
  %tmp39 = load <2 x float>* %spatialSigma.addr   ; <<2 x float>> [#uses=1]
  %tmp40 = extractelement <2 x float> %tmp39, i32 1 ; <float> [#uses=1]
  %mul41 = fmul float 0x40012B0200000000, %tmp40  ; <float> [#uses=1]
  %call42 = call float @_Z4ceilf(float %mul41)    ; <float> [#uses=1]
  %conv43 = fptosi float %call42 to i32           ; <i32> [#uses=1]
  %tmp44 = load %struct.anon addrspace(1)** %pArgs.addr ; <%struct.anon addrspace(1)*> [#uses=1]
  %tmp45 = getelementptr inbounds %struct.anon addrspace(1)* %tmp44, i32 0, i32 4 ; <i32 addrspace(1)*> [#uses=1]
  store i32 %conv43, i32 addrspace(1)* %tmp45
  ret void
}

declare float @_Z4ceilf(float)

; CHECK: ret
define float @rgbToLuminance(<4 x float> %rgbIn) nounwind {
entry:
  %retval = alloca float, align 4                 ; <float*> [#uses=2]
  %rgbIn.addr = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=2]
  %kToLum = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  store <4 x float> %rgbIn, <4 x float>* %rgbIn.addr
  store <4 x float> <float 0x3FD4FC5040000000, float 0x3FE4FB7EA0000000, float 0x3F90CB2960000000, float 0.000000e+000>, <4 x float>* %kToLum
  %tmp = load <4 x float>* %rgbIn.addr            ; <<4 x float>> [#uses=1]
  %tmp1 = load <4 x float>* %kToLum               ; <<4 x float>> [#uses=1]
  %call = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp, <4 x float> %tmp1) ; <float> [#uses=1]
  store float %call, float* %retval
  %0 = load float* %retval                        ; <float> [#uses=1]
  ret float %0
}

declare float @_Z3dotU8__vector4fS_(<4 x float>, <4 x float>)

; CHECK: ret
define <4 x float> @evaluatePixelScalar(float addrspace(1)* %inputImage, <2 x i32> %outCrd, <2 x i32> %imgSize, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=3]
  %outCrd.addr = alloca <2 x i32>, align 8        ; <<2 x i32>*> [#uses=8]
  %imgSize.addr = alloca <2 x i32>, align 8       ; <<2 x i32>*> [#uses=5]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=16]
  %iZero = alloca <2 x i32>, align 8              ; <<2 x i32>*> [#uses=1]
  %dst = alloca [4 x float], align 4              ; <[4 x float]*> [#uses=6]
  %scale = alloca float, align 4                  ; <float*> [#uses=4]
  %negOffsetX = alloca i32, align 4               ; <i32*> [#uses=2]
  %posOffsetX = alloca i32, align 4               ; <i32*> [#uses=2]
  %negOffsetY = alloca i32, align 4               ; <i32*> [#uses=2]
  %posOffsetY = alloca i32, align 4               ; <i32*> [#uses=2]
  %y = alloca i32, align 4                        ; <i32*> [#uses=7]
  %x = alloca i32, align 4                        ; <i32*> [#uses=7]
  %sCrd = alloca <2 x i32>, align 8               ; <<2 x i32>*> [#uses=2]
  %v3 = alloca <2 x i32>, align 8                 ; <<2 x i32>*> [#uses=3]
  %wD = alloca float, align 4                     ; <float*> [#uses=3]
  %difference = alloca [4 x float], align 4       ; <[4 x float]*> [#uses=11]
  %curVal = alloca [4 x float], align 4           ; <[4 x float]*> [#uses=9]
  %ch = alloca i32, align 4                       ; <i32*> [#uses=9]
  %origSrc = alloca float, align 4                ; <float*> [#uses=2]
  %diffLum = alloca float, align 4                ; <float*> [#uses=3]
  %curLum = alloca float, align 4                 ; <float*> [#uses=2]
  %wR = alloca float, align 4                     ; <float*> [#uses=3]
  %ch225 = alloca i32, align 4                    ; <i32*> [#uses=6]
  %dst1 = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=9]
  %tmp_scale = alloca float, align 4              ; <float*> [#uses=5]
  store float addrspace(1)* %inputImage, float addrspace(1)** %inputImage.addr
  store <2 x i32> %outCrd, <2 x i32>* %outCrd.addr
  store <2 x i32> %imgSize, <2 x i32>* %imgSize.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  store <2 x i32> zeroinitializer, <2 x i32>* %iZero
  %tmp = bitcast [4 x float]* %dst to i8*         ; <i8*> [#uses=1]
  call void @llvm.memset.i32(i8* %tmp, i8 0, i32 16, i32 4)
  store float 0.000000e+000, float* %scale
  %tmp3 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp4 = getelementptr inbounds %struct.anon addrspace(2)* %tmp3, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp5 = load i32 addrspace(2)* %tmp4            ; <i32> [#uses=1]
  %tmp6 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp7 = getelementptr inbounds %struct.anon addrspace(2)* %tmp6, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp8 = load i32 addrspace(2)* %tmp7            ; <i32> [#uses=1]
  %tmp9 = load <2 x i32>* %outCrd.addr            ; <<2 x i32>> [#uses=1]
  %tmp10 = extractelement <2 x i32> %tmp9, i32 0  ; <i32> [#uses=1]
  %call = call i32 @_Z3minii(i32 %tmp8, i32 %tmp10) ; <i32> [#uses=1]
  %sub = sub i32 %tmp5, %call                     ; <i32> [#uses=1]
  store i32 %sub, i32* %negOffsetX
  %tmp12 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp13 = getelementptr inbounds %struct.anon addrspace(2)* %tmp12, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp14 = load i32 addrspace(2)* %tmp13          ; <i32> [#uses=1]
  %tmp15 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp16 = getelementptr inbounds %struct.anon addrspace(2)* %tmp15, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp17 = load i32 addrspace(2)* %tmp16          ; <i32> [#uses=1]
  %tmp18 = load <2 x i32>* %imgSize.addr          ; <<2 x i32>> [#uses=1]
  %tmp19 = extractelement <2 x i32> %tmp18, i32 0 ; <i32> [#uses=1]
  %tmp20 = load <2 x i32>* %outCrd.addr           ; <<2 x i32>> [#uses=1]
  %tmp21 = extractelement <2 x i32> %tmp20, i32 0 ; <i32> [#uses=1]
  %sub22 = sub i32 %tmp19, %tmp21                 ; <i32> [#uses=1]
  %sub23 = sub i32 %sub22, 1                      ; <i32> [#uses=1]
  %call24 = call i32 @_Z3minii(i32 %tmp17, i32 %sub23) ; <i32> [#uses=1]
  %sub25 = sub i32 %tmp14, %call24                ; <i32> [#uses=1]
  store i32 %sub25, i32* %posOffsetX
  %tmp27 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp28 = getelementptr inbounds %struct.anon addrspace(2)* %tmp27, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp29 = load i32 addrspace(2)* %tmp28          ; <i32> [#uses=1]
  %tmp30 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp31 = getelementptr inbounds %struct.anon addrspace(2)* %tmp30, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp32 = load i32 addrspace(2)* %tmp31          ; <i32> [#uses=1]
  %tmp33 = load <2 x i32>* %outCrd.addr           ; <<2 x i32>> [#uses=1]
  %tmp34 = extractelement <2 x i32> %tmp33, i32 1 ; <i32> [#uses=1]
  %call35 = call i32 @_Z3minii(i32 %tmp32, i32 %tmp34) ; <i32> [#uses=1]
  %sub36 = sub i32 %tmp29, %call35                ; <i32> [#uses=1]
  store i32 %sub36, i32* %negOffsetY
  %tmp38 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp39 = getelementptr inbounds %struct.anon addrspace(2)* %tmp38, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp40 = load i32 addrspace(2)* %tmp39          ; <i32> [#uses=1]
  %tmp41 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp42 = getelementptr inbounds %struct.anon addrspace(2)* %tmp41, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp43 = load i32 addrspace(2)* %tmp42          ; <i32> [#uses=1]
  %tmp44 = load <2 x i32>* %imgSize.addr          ; <<2 x i32>> [#uses=1]
  %tmp45 = extractelement <2 x i32> %tmp44, i32 1 ; <i32> [#uses=1]
  %tmp46 = load <2 x i32>* %outCrd.addr           ; <<2 x i32>> [#uses=1]
  %tmp47 = extractelement <2 x i32> %tmp46, i32 1 ; <i32> [#uses=1]
  %sub48 = sub i32 %tmp45, %tmp47                 ; <i32> [#uses=1]
  %sub49 = sub i32 %sub48, 1                      ; <i32> [#uses=1]
  %call50 = call i32 @_Z3minii(i32 %tmp43, i32 %sub49) ; <i32> [#uses=1]
  %sub51 = sub i32 %tmp40, %call50                ; <i32> [#uses=1]
  store i32 %sub51, i32* %posOffsetY
  %tmp53 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp54 = getelementptr inbounds %struct.anon addrspace(2)* %tmp53, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp55 = load i32 addrspace(2)* %tmp54          ; <i32> [#uses=1]
  %neg = sub i32 0, %tmp55                        ; <i32> [#uses=1]
  %tmp56 = load i32* %negOffsetY                  ; <i32> [#uses=1]
  %add = add nsw i32 %neg, %tmp56                 ; <i32> [#uses=1]
  store i32 %add, i32* %y
  br label %for.cond

for.cond:                                         ; preds = %for.inc257, %entry
  %tmp57 = load i32* %y                           ; <i32> [#uses=1]
  %tmp58 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp59 = getelementptr inbounds %struct.anon addrspace(2)* %tmp58, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp60 = load i32 addrspace(2)* %tmp59          ; <i32> [#uses=1]
  %tmp61 = load i32* %posOffsetY                  ; <i32> [#uses=1]
  %sub62 = sub i32 %tmp60, %tmp61                 ; <i32> [#uses=1]
  %cmp = icmp sle i32 %tmp57, %sub62              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end260

for.body:                                         ; preds = %for.cond
  %tmp64 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp65 = getelementptr inbounds %struct.anon addrspace(2)* %tmp64, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp66 = load i32 addrspace(2)* %tmp65          ; <i32> [#uses=1]
  %neg67 = sub i32 0, %tmp66                      ; <i32> [#uses=1]
  %tmp68 = load i32* %negOffsetX                  ; <i32> [#uses=1]
  %add69 = add nsw i32 %neg67, %tmp68             ; <i32> [#uses=1]
  store i32 %add69, i32* %x
  br label %for.cond70

for.cond70:                                       ; preds = %for.inc253, %for.body
  %tmp71 = load i32* %x                           ; <i32> [#uses=1]
  %tmp72 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp73 = getelementptr inbounds %struct.anon addrspace(2)* %tmp72, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp74 = load i32 addrspace(2)* %tmp73          ; <i32> [#uses=1]
  %tmp75 = load i32* %posOffsetX                  ; <i32> [#uses=1]
  %sub76 = sub i32 %tmp74, %tmp75                 ; <i32> [#uses=1]
  %cmp77 = icmp sle i32 %tmp71, %sub76            ; <i1> [#uses=1]
  br i1 %cmp77, label %for.body78, label %for.end256

for.body78:                                       ; preds = %for.cond70
  %tmp80 = load i32* %x                           ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %tmp80, i32 0 ; <<2 x i32>> [#uses=1]
  %tmp81 = load i32* %y                           ; <i32> [#uses=1]
  %vecinit82 = insertelement <2 x i32> %vecinit, i32 %tmp81, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit82, <2 x i32>* %sCrd
  %tmp84 = load <2 x i32>* %outCrd.addr           ; <<2 x i32>> [#uses=1]
  %tmp85 = load <2 x i32>* %sCrd                  ; <<2 x i32>> [#uses=1]
  %add86 = add nsw <2 x i32> %tmp84, %tmp85       ; <<2 x i32>> [#uses=1]
  store <2 x i32> %add86, <2 x i32>* %v3
  %tmp88 = load i32* %x                           ; <i32> [#uses=1]
  %tmp89 = load i32* %x                           ; <i32> [#uses=1]
  %mul = mul i32 %tmp88, %tmp89                   ; <i32> [#uses=1]
  %tmp90 = load i32* %y                           ; <i32> [#uses=1]
  %tmp91 = load i32* %y                           ; <i32> [#uses=1]
  %mul92 = mul i32 %tmp90, %tmp91                 ; <i32> [#uses=1]
  %add93 = add nsw i32 %mul, %mul92               ; <i32> [#uses=1]
  %neg94 = sub i32 0, %add93                      ; <i32> [#uses=1]
  %conv = sitofp i32 %neg94 to float              ; <float> [#uses=1]
  %tmp95 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp96 = getelementptr inbounds %struct.anon addrspace(2)* %tmp95, i32 0, i32 1 ; <float addrspace(2)*> [#uses=1]
  %tmp97 = load float addrspace(2)* %tmp96        ; <float> [#uses=1]
  %mul98 = fmul float %conv, %tmp97               ; <float> [#uses=1]
  %call99 = call float @_Z3expf(float %mul98)     ; <float> [#uses=1]
  store float %call99, float* %wD
  store i32 0, i32* %ch
  br label %for.cond103

for.cond103:                                      ; preds = %for.inc, %for.body78
  %tmp104 = load i32* %ch                         ; <i32> [#uses=1]
  %cmp105 = icmp slt i32 %tmp104, 4               ; <i1> [#uses=1]
  br i1 %cmp105, label %for.body107, label %for.end

for.body107:                                      ; preds = %for.cond103
  %tmp109 = load <2 x i32>* %imgSize.addr         ; <<2 x i32>> [#uses=1]
  %tmp110 = extractelement <2 x i32> %tmp109, i32 0 ; <i32> [#uses=1]
  %tmp111 = load <2 x i32>* %outCrd.addr          ; <<2 x i32>> [#uses=1]
  %tmp112 = extractelement <2 x i32> %tmp111, i32 1 ; <i32> [#uses=1]
  %mul113 = mul i32 %tmp110, %tmp112              ; <i32> [#uses=1]
  %tmp114 = load <2 x i32>* %outCrd.addr          ; <<2 x i32>> [#uses=1]
  %tmp115 = extractelement <2 x i32> %tmp114, i32 0 ; <i32> [#uses=1]
  %add116 = add nsw i32 %mul113, %tmp115          ; <i32> [#uses=1]
  %mul117 = mul i32 4, %add116                    ; <i32> [#uses=1]
  %tmp118 = load i32* %ch                         ; <i32> [#uses=1]
  %add119 = add nsw i32 %mul117, %tmp118          ; <i32> [#uses=1]
  %tmp120 = load float addrspace(1)** %inputImage.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* %tmp120, i32 %add119 ; <float addrspace(1)*> [#uses=1]
  %tmp121 = load float addrspace(1)* %arrayidx    ; <float> [#uses=1]
  store float %tmp121, float* %origSrc
  %tmp122 = load <2 x i32>* %imgSize.addr         ; <<2 x i32>> [#uses=1]
  %tmp123 = extractelement <2 x i32> %tmp122, i32 0 ; <i32> [#uses=1]
  %tmp124 = load <2 x i32>* %v3                   ; <<2 x i32>> [#uses=1]
  %tmp125 = extractelement <2 x i32> %tmp124, i32 1 ; <i32> [#uses=1]
  %mul126 = mul i32 %tmp123, %tmp125              ; <i32> [#uses=1]
  %tmp127 = load <2 x i32>* %v3                   ; <<2 x i32>> [#uses=1]
  %tmp128 = extractelement <2 x i32> %tmp127, i32 0 ; <i32> [#uses=1]
  %add129 = add nsw i32 %mul126, %tmp128          ; <i32> [#uses=1]
  %mul130 = mul i32 4, %add129                    ; <i32> [#uses=1]
  %tmp131 = load i32* %ch                         ; <i32> [#uses=1]
  %add132 = add nsw i32 %mul130, %tmp131          ; <i32> [#uses=1]
  %tmp133 = load float addrspace(1)** %inputImage.addr ; <float addrspace(1)*> [#uses=1]
  %arrayidx134 = getelementptr inbounds float addrspace(1)* %tmp133, i32 %add132 ; <float addrspace(1)*> [#uses=1]
  %tmp135 = load float addrspace(1)* %arrayidx134 ; <float> [#uses=1]
  %tmp136 = load i32* %ch                         ; <i32> [#uses=1]
  %arraydecay = getelementptr inbounds [4 x float]* %curVal, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx137 = getelementptr inbounds float* %arraydecay, i32 %tmp136 ; <float*> [#uses=1]
  store float %tmp135, float* %arrayidx137
  %tmp138 = load float* %origSrc                  ; <float> [#uses=1]
  %tmp139 = load i32* %ch                         ; <i32> [#uses=1]
  %arraydecay140 = getelementptr inbounds [4 x float]* %curVal, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx141 = getelementptr inbounds float* %arraydecay140, i32 %tmp139 ; <float*> [#uses=1]
  %tmp142 = load float* %arrayidx141              ; <float> [#uses=1]
  %sub143 = fsub float %tmp138, %tmp142           ; <float> [#uses=1]
  %call144 = call float @_Z4fabsf(float %sub143)  ; <float> [#uses=1]
  %tmp145 = load i32* %ch                         ; <i32> [#uses=1]
  %arraydecay146 = getelementptr inbounds [4 x float]* %difference, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx147 = getelementptr inbounds float* %arraydecay146, i32 %tmp145 ; <float*> [#uses=1]
  store float %call144, float* %arrayidx147
  br label %for.inc

for.inc:                                          ; preds = %for.body107
  %tmp148 = load i32* %ch                         ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp148, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %ch
  br label %for.cond103

for.end:                                          ; preds = %for.cond103
  %tmp150 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp151 = getelementptr inbounds %struct.anon addrspace(2)* %tmp150, i32 0, i32 0 ; <i32 addrspace(2)*> [#uses=1]
  %tmp152 = load i32 addrspace(2)* %tmp151        ; <i32> [#uses=1]
  %tobool = icmp ne i32 %tmp152, 0                ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %for.end
  %arraydecay153 = getelementptr inbounds [4 x float]* %difference, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx154 = getelementptr inbounds float* %arraydecay153, i32 3 ; <float*> [#uses=1]
  store float 0.000000e+000, float* %arrayidx154
  %arraydecay155 = getelementptr inbounds [4 x float]* %difference, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx156 = getelementptr inbounds float* %arraydecay155, i32 0 ; <float*> [#uses=1]
  %tmp157 = load float* %arrayidx156              ; <float> [#uses=1]
  %arraydecay158 = getelementptr inbounds [4 x float]* %difference, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx159 = getelementptr inbounds float* %arraydecay158, i32 0 ; <float*> [#uses=1]
  %tmp160 = load float* %arrayidx159              ; <float> [#uses=1]
  %mul161 = fmul float %tmp157, %tmp160           ; <float> [#uses=1]
  %arraydecay162 = getelementptr inbounds [4 x float]* %difference, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx163 = getelementptr inbounds float* %arraydecay162, i32 1 ; <float*> [#uses=1]
  %tmp164 = load float* %arrayidx163              ; <float> [#uses=1]
  %arraydecay165 = getelementptr inbounds [4 x float]* %difference, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx166 = getelementptr inbounds float* %arraydecay165, i32 1 ; <float*> [#uses=1]
  %tmp167 = load float* %arrayidx166              ; <float> [#uses=1]
  %mul168 = fmul float %tmp164, %tmp167           ; <float> [#uses=1]
  %add169 = fadd float %mul161, %mul168           ; <float> [#uses=1]
  %arraydecay170 = getelementptr inbounds [4 x float]* %difference, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx171 = getelementptr inbounds float* %arraydecay170, i32 2 ; <float*> [#uses=1]
  %tmp172 = load float* %arrayidx171              ; <float> [#uses=1]
  %arraydecay173 = getelementptr inbounds [4 x float]* %difference, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx174 = getelementptr inbounds float* %arraydecay173, i32 2 ; <float*> [#uses=1]
  %tmp175 = load float* %arrayidx174              ; <float> [#uses=1]
  %mul176 = fmul float %tmp172, %tmp175           ; <float> [#uses=1]
  %add177 = fadd float %add169, %mul176           ; <float> [#uses=1]
  store float %add177, float* %diffLum
  br label %if.end

if.else:                                          ; preds = %for.end
  %arraydecay178 = getelementptr inbounds [4 x float]* %difference, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx179 = getelementptr inbounds float* %arraydecay178, i32 0 ; <float*> [#uses=1]
  %tmp180 = load float* %arrayidx179              ; <float> [#uses=1]
  %mul181 = fmul float %tmp180, 0x3FD4FC5040000000 ; <float> [#uses=1]
  %arraydecay182 = getelementptr inbounds [4 x float]* %difference, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx183 = getelementptr inbounds float* %arraydecay182, i32 1 ; <float*> [#uses=1]
  %tmp184 = load float* %arrayidx183              ; <float> [#uses=1]
  %mul185 = fmul float %tmp184, 0x3FE4FB7EA0000000 ; <float> [#uses=1]
  %add186 = fadd float %mul181, %mul185           ; <float> [#uses=1]
  %arraydecay187 = getelementptr inbounds [4 x float]* %difference, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx188 = getelementptr inbounds float* %arraydecay187, i32 2 ; <float*> [#uses=1]
  %tmp189 = load float* %arrayidx188              ; <float> [#uses=1]
  %mul190 = fmul float %tmp189, 0x3F90CB2960000000 ; <float> [#uses=1]
  %add191 = fadd float %add186, %mul190           ; <float> [#uses=1]
  store float %add191, float* %diffLum
  %arraydecay193 = getelementptr inbounds [4 x float]* %curVal, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx194 = getelementptr inbounds float* %arraydecay193, i32 0 ; <float*> [#uses=1]
  %tmp195 = load float* %arrayidx194              ; <float> [#uses=1]
  %mul196 = fmul float %tmp195, 0x3FD4FC5040000000 ; <float> [#uses=1]
  %arraydecay197 = getelementptr inbounds [4 x float]* %curVal, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx198 = getelementptr inbounds float* %arraydecay197, i32 1 ; <float*> [#uses=1]
  %tmp199 = load float* %arrayidx198              ; <float> [#uses=1]
  %mul200 = fmul float %tmp199, 0x3FE4FB7EA0000000 ; <float> [#uses=1]
  %add201 = fadd float %mul196, %mul200           ; <float> [#uses=1]
  %arraydecay202 = getelementptr inbounds [4 x float]* %curVal, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx203 = getelementptr inbounds float* %arraydecay202, i32 2 ; <float*> [#uses=1]
  %tmp204 = load float* %arrayidx203              ; <float> [#uses=1]
  %mul205 = fmul float %tmp204, 0x3F90CB2960000000 ; <float> [#uses=1]
  %add206 = fadd float %add201, %mul205           ; <float> [#uses=1]
  store float %add206, float* %curLum
  %tmp207 = load float* %curLum                   ; <float> [#uses=1]
  %arraydecay208 = getelementptr inbounds [4 x float]* %curVal, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx209 = getelementptr inbounds float* %arraydecay208, i32 2 ; <float*> [#uses=2]
  store float %tmp207, float* %arrayidx209
  %tmp210 = load float* %arrayidx209              ; <float> [#uses=1]
  %arraydecay211 = getelementptr inbounds [4 x float]* %curVal, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx212 = getelementptr inbounds float* %arraydecay211, i32 1 ; <float*> [#uses=2]
  store float %tmp210, float* %arrayidx212
  %tmp213 = load float* %arrayidx212              ; <float> [#uses=1]
  %arraydecay214 = getelementptr inbounds [4 x float]* %curVal, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx215 = getelementptr inbounds float* %arraydecay214, i32 0 ; <float*> [#uses=1]
  store float %tmp213, float* %arrayidx215
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %tmp217 = load float* %diffLum                  ; <float> [#uses=1]
  %tmp218 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp219 = getelementptr inbounds %struct.anon addrspace(2)* %tmp218, i32 0, i32 2 ; <float addrspace(2)*> [#uses=1]
  %tmp220 = load float addrspace(2)* %tmp219      ; <float> [#uses=1]
  %neg221 = fsub float -0.000000e+000, %tmp220    ; <float> [#uses=1]
  %mul222 = fmul float %tmp217, %neg221           ; <float> [#uses=1]
  %call223 = call float @_Z3expf(float %mul222)   ; <float> [#uses=1]
  store float %call223, float* %wR
  store i32 0, i32* %ch225
  br label %for.cond226

for.cond226:                                      ; preds = %for.inc244, %if.end
  %tmp227 = load i32* %ch225                      ; <i32> [#uses=1]
  %cmp228 = icmp slt i32 %tmp227, 4               ; <i1> [#uses=1]
  br i1 %cmp228, label %for.body230, label %for.end247

for.body230:                                      ; preds = %for.cond226
  %tmp231 = load i32* %ch225                      ; <i32> [#uses=1]
  %arraydecay232 = getelementptr inbounds [4 x float]* %curVal, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx233 = getelementptr inbounds float* %arraydecay232, i32 %tmp231 ; <float*> [#uses=1]
  %tmp234 = load float* %arrayidx233              ; <float> [#uses=1]
  %tmp235 = load float* %wD                       ; <float> [#uses=1]
  %mul236 = fmul float %tmp234, %tmp235           ; <float> [#uses=1]
  %tmp237 = load float* %wR                       ; <float> [#uses=1]
  %mul238 = fmul float %mul236, %tmp237           ; <float> [#uses=1]
  %tmp239 = load i32* %ch225                      ; <i32> [#uses=1]
  %arraydecay240 = getelementptr inbounds [4 x float]* %dst, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx241 = getelementptr inbounds float* %arraydecay240, i32 %tmp239 ; <float*> [#uses=2]
  %tmp242 = load float* %arrayidx241              ; <float> [#uses=1]
  %add243 = fadd float %tmp242, %mul238           ; <float> [#uses=1]
  store float %add243, float* %arrayidx241
  br label %for.inc244

for.inc244:                                       ; preds = %for.body230
  %tmp245 = load i32* %ch225                      ; <i32> [#uses=1]
  %inc246 = add nsw i32 %tmp245, 1                ; <i32> [#uses=1]
  store i32 %inc246, i32* %ch225
  br label %for.cond226

for.end247:                                       ; preds = %for.cond226
  %tmp248 = load float* %wD                       ; <float> [#uses=1]
  %tmp249 = load float* %wR                       ; <float> [#uses=1]
  %mul250 = fmul float %tmp248, %tmp249           ; <float> [#uses=1]
  %tmp251 = load float* %scale                    ; <float> [#uses=1]
  %add252 = fadd float %tmp251, %mul250           ; <float> [#uses=1]
  store float %add252, float* %scale
  br label %for.inc253

for.inc253:                                       ; preds = %for.end247
  %tmp254 = load i32* %x                          ; <i32> [#uses=1]
  %inc255 = add nsw i32 %tmp254, 1                ; <i32> [#uses=1]
  store i32 %inc255, i32* %x
  br label %for.cond70

for.end256:                                       ; preds = %for.cond70
  br label %for.inc257

for.inc257:                                       ; preds = %for.end256
  %tmp258 = load i32* %y                          ; <i32> [#uses=1]
  %inc259 = add nsw i32 %tmp258, 1                ; <i32> [#uses=1]
  store i32 %inc259, i32* %y
  br label %for.cond

for.end260:                                       ; preds = %for.cond
  %tmp263 = load float* %scale                    ; <float> [#uses=3]
  %cmp264 = fcmp oeq float 0.000000e+000, %tmp263 ; <i1> [#uses=1]
  %sel = select i1 %cmp264, float 1.000000e+000, float %tmp263 ; <float> [#uses=0]
  %div = fdiv float 1.000000e+000, %tmp263        ; <float> [#uses=1]
  store float %div, float* %tmp_scale
  %arraydecay265 = getelementptr inbounds [4 x float]* %dst, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx266 = getelementptr inbounds float* %arraydecay265, i32 0 ; <float*> [#uses=1]
  %tmp267 = load float* %arrayidx266              ; <float> [#uses=1]
  %tmp268 = load float* %tmp_scale                ; <float> [#uses=1]
  %mul269 = fmul float %tmp267, %tmp268           ; <float> [#uses=1]
  %tmp270 = load <4 x float>* %dst1               ; <<4 x float>> [#uses=1]
  %tmp271 = insertelement <4 x float> %tmp270, float %mul269, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp271, <4 x float>* %dst1
  %arraydecay272 = getelementptr inbounds [4 x float]* %dst, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx273 = getelementptr inbounds float* %arraydecay272, i32 1 ; <float*> [#uses=1]
  %tmp274 = load float* %arrayidx273              ; <float> [#uses=1]
  %tmp275 = load float* %tmp_scale                ; <float> [#uses=1]
  %mul276 = fmul float %tmp274, %tmp275           ; <float> [#uses=1]
  %tmp277 = load <4 x float>* %dst1               ; <<4 x float>> [#uses=1]
  %tmp278 = insertelement <4 x float> %tmp277, float %mul276, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp278, <4 x float>* %dst1
  %arraydecay279 = getelementptr inbounds [4 x float]* %dst, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx280 = getelementptr inbounds float* %arraydecay279, i32 2 ; <float*> [#uses=1]
  %tmp281 = load float* %arrayidx280              ; <float> [#uses=1]
  %tmp282 = load float* %tmp_scale                ; <float> [#uses=1]
  %mul283 = fmul float %tmp281, %tmp282           ; <float> [#uses=1]
  %tmp284 = load <4 x float>* %dst1               ; <<4 x float>> [#uses=1]
  %tmp285 = insertelement <4 x float> %tmp284, float %mul283, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp285, <4 x float>* %dst1
  %arraydecay286 = getelementptr inbounds [4 x float]* %dst, i32 0, i32 0 ; <float*> [#uses=1]
  %arrayidx287 = getelementptr inbounds float* %arraydecay286, i32 3 ; <float*> [#uses=1]
  %tmp288 = load float* %arrayidx287              ; <float> [#uses=1]
  %tmp289 = load float* %tmp_scale                ; <float> [#uses=1]
  %mul290 = fmul float %tmp288, %tmp289           ; <float> [#uses=1]
  %tmp291 = load <4 x float>* %dst1               ; <<4 x float>> [#uses=1]
  %tmp292 = insertelement <4 x float> %tmp291, float %mul290, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp292, <4 x float>* %dst1
  %tmp293 = load <4 x float>* %dst1               ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp293, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare void @llvm.memset.i32(i8* nocapture, i8, i32, i32) nounwind

declare i32 @_Z3minii(i32, i32)

declare float @_Z3expf(float)

declare float @_Z4fabsf(float)

; CHECK: ret
define <4 x float> @evaluatePixel(<4 x float> addrspace(1)* %inputImage, <2 x i32> %outCrd, <2 x i32> %imgSize, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=3]
  %outCrd.addr = alloca <2 x i32>, align 8        ; <<2 x i32>*> [#uses=8]
  %imgSize.addr = alloca <2 x i32>, align 8       ; <<2 x i32>*> [#uses=5]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=16]
  %kZero = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %iZero = alloca <2 x i32>, align 8              ; <<2 x i32>*> [#uses=1]
  %origSrc = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %dst = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=6]
  %scale = alloca float, align 4                  ; <float*> [#uses=4]
  %difference = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=6]
  %negOffsetX = alloca i32, align 4               ; <i32*> [#uses=2]
  %posOffsetX = alloca i32, align 4               ; <i32*> [#uses=2]
  %negOffsetY = alloca i32, align 4               ; <i32*> [#uses=2]
  %posOffsetY = alloca i32, align 4               ; <i32*> [#uses=2]
  %y = alloca i32, align 4                        ; <i32*> [#uses=7]
  %x = alloca i32, align 4                        ; <i32*> [#uses=7]
  %sCrd = alloca <2 x i32>, align 8               ; <<2 x i32>*> [#uses=2]
  %v3 = alloca <2 x i32>, align 8                 ; <<2 x i32>*> [#uses=3]
  %curVal = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=12]
  %wD = alloca float, align 4                     ; <float*> [#uses=3]
  %diffLum = alloca float, align 4                ; <float*> [#uses=3]
  %curLum = alloca float, align 4                 ; <float*> [#uses=2]
  %wR = alloca float, align 4                     ; <float*> [#uses=3]
  store <4 x float> addrspace(1)* %inputImage, <4 x float> addrspace(1)** %inputImage.addr
  store <2 x i32> %outCrd, <2 x i32>* %outCrd.addr
  store <2 x i32> %imgSize, <2 x i32>* %imgSize.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  store <4 x float> zeroinitializer, <4 x float>* %kZero
  store <2 x i32> zeroinitializer, <2 x i32>* %iZero
  %tmp = load <2 x i32>* %imgSize.addr            ; <<2 x i32>> [#uses=1]
  %tmp1 = extractelement <2 x i32> %tmp, i32 0    ; <i32> [#uses=1]
  %tmp2 = load <2 x i32>* %outCrd.addr            ; <<2 x i32>> [#uses=1]
  %tmp3 = extractelement <2 x i32> %tmp2, i32 1   ; <i32> [#uses=1]
  %mul = mul i32 %tmp1, %tmp3                     ; <i32> [#uses=1]
  %tmp4 = load <2 x i32>* %outCrd.addr            ; <<2 x i32>> [#uses=1]
  %tmp5 = extractelement <2 x i32> %tmp4, i32 0   ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %tmp5                  ; <i32> [#uses=1]
  %tmp6 = load <4 x float> addrspace(1)** %inputImage.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp6, i32 %add ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp7 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp7, <4 x float>* %origSrc
  %tmp9 = load <4 x float>* %kZero                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp9, <4 x float>* %dst
  store float 0.000000e+000, float* %scale
  %tmp13 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp14 = getelementptr inbounds %struct.anon addrspace(2)* %tmp13, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp15 = load i32 addrspace(2)* %tmp14          ; <i32> [#uses=1]
  %tmp16 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp17 = getelementptr inbounds %struct.anon addrspace(2)* %tmp16, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp18 = load i32 addrspace(2)* %tmp17          ; <i32> [#uses=1]
  %tmp19 = load <2 x i32>* %outCrd.addr           ; <<2 x i32>> [#uses=1]
  %tmp20 = extractelement <2 x i32> %tmp19, i32 0 ; <i32> [#uses=1]
  %call = call i32 @_Z3minii(i32 %tmp18, i32 %tmp20) ; <i32> [#uses=1]
  %sub = sub i32 %tmp15, %call                    ; <i32> [#uses=1]
  store i32 %sub, i32* %negOffsetX
  %tmp22 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp23 = getelementptr inbounds %struct.anon addrspace(2)* %tmp22, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp24 = load i32 addrspace(2)* %tmp23          ; <i32> [#uses=1]
  %tmp25 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp26 = getelementptr inbounds %struct.anon addrspace(2)* %tmp25, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp27 = load i32 addrspace(2)* %tmp26          ; <i32> [#uses=1]
  %tmp28 = load <2 x i32>* %imgSize.addr          ; <<2 x i32>> [#uses=1]
  %tmp29 = extractelement <2 x i32> %tmp28, i32 0 ; <i32> [#uses=1]
  %tmp30 = load <2 x i32>* %outCrd.addr           ; <<2 x i32>> [#uses=1]
  %tmp31 = extractelement <2 x i32> %tmp30, i32 0 ; <i32> [#uses=1]
  %sub32 = sub i32 %tmp29, %tmp31                 ; <i32> [#uses=1]
  %sub33 = sub i32 %sub32, 1                      ; <i32> [#uses=1]
  %call34 = call i32 @_Z3minii(i32 %tmp27, i32 %sub33) ; <i32> [#uses=1]
  %sub35 = sub i32 %tmp24, %call34                ; <i32> [#uses=1]
  store i32 %sub35, i32* %posOffsetX
  %tmp37 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp38 = getelementptr inbounds %struct.anon addrspace(2)* %tmp37, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp39 = load i32 addrspace(2)* %tmp38          ; <i32> [#uses=1]
  %tmp40 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp41 = getelementptr inbounds %struct.anon addrspace(2)* %tmp40, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp42 = load i32 addrspace(2)* %tmp41          ; <i32> [#uses=1]
  %tmp43 = load <2 x i32>* %outCrd.addr           ; <<2 x i32>> [#uses=1]
  %tmp44 = extractelement <2 x i32> %tmp43, i32 1 ; <i32> [#uses=1]
  %call45 = call i32 @_Z3minii(i32 %tmp42, i32 %tmp44) ; <i32> [#uses=1]
  %sub46 = sub i32 %tmp39, %call45                ; <i32> [#uses=1]
  store i32 %sub46, i32* %negOffsetY
  %tmp48 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp49 = getelementptr inbounds %struct.anon addrspace(2)* %tmp48, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp50 = load i32 addrspace(2)* %tmp49          ; <i32> [#uses=1]
  %tmp51 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp52 = getelementptr inbounds %struct.anon addrspace(2)* %tmp51, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp53 = load i32 addrspace(2)* %tmp52          ; <i32> [#uses=1]
  %tmp54 = load <2 x i32>* %imgSize.addr          ; <<2 x i32>> [#uses=1]
  %tmp55 = extractelement <2 x i32> %tmp54, i32 1 ; <i32> [#uses=1]
  %tmp56 = load <2 x i32>* %outCrd.addr           ; <<2 x i32>> [#uses=1]
  %tmp57 = extractelement <2 x i32> %tmp56, i32 1 ; <i32> [#uses=1]
  %sub58 = sub i32 %tmp55, %tmp57                 ; <i32> [#uses=1]
  %sub59 = sub i32 %sub58, 1                      ; <i32> [#uses=1]
  %call60 = call i32 @_Z3minii(i32 %tmp53, i32 %sub59) ; <i32> [#uses=1]
  %sub61 = sub i32 %tmp50, %call60                ; <i32> [#uses=1]
  store i32 %sub61, i32* %posOffsetY
  %tmp63 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp64 = getelementptr inbounds %struct.anon addrspace(2)* %tmp63, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp65 = load i32 addrspace(2)* %tmp64          ; <i32> [#uses=1]
  %neg = sub i32 0, %tmp65                        ; <i32> [#uses=1]
  %tmp66 = load i32* %negOffsetY                  ; <i32> [#uses=1]
  %add67 = add nsw i32 %neg, %tmp66               ; <i32> [#uses=1]
  store i32 %add67, i32* %y
  br label %for.cond

for.cond:                                         ; preds = %for.inc177, %entry
  %tmp68 = load i32* %y                           ; <i32> [#uses=1]
  %tmp69 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp70 = getelementptr inbounds %struct.anon addrspace(2)* %tmp69, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp71 = load i32 addrspace(2)* %tmp70          ; <i32> [#uses=1]
  %tmp72 = load i32* %posOffsetY                  ; <i32> [#uses=1]
  %sub73 = sub i32 %tmp71, %tmp72                 ; <i32> [#uses=1]
  %cmp = icmp sle i32 %tmp68, %sub73              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end180

for.body:                                         ; preds = %for.cond
  %tmp75 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp76 = getelementptr inbounds %struct.anon addrspace(2)* %tmp75, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp77 = load i32 addrspace(2)* %tmp76          ; <i32> [#uses=1]
  %neg78 = sub i32 0, %tmp77                      ; <i32> [#uses=1]
  %tmp79 = load i32* %negOffsetX                  ; <i32> [#uses=1]
  %add80 = add nsw i32 %neg78, %tmp79             ; <i32> [#uses=1]
  store i32 %add80, i32* %x
  br label %for.cond81

for.cond81:                                       ; preds = %for.inc, %for.body
  %tmp82 = load i32* %x                           ; <i32> [#uses=1]
  %tmp83 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp84 = getelementptr inbounds %struct.anon addrspace(2)* %tmp83, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp85 = load i32 addrspace(2)* %tmp84          ; <i32> [#uses=1]
  %tmp86 = load i32* %posOffsetX                  ; <i32> [#uses=1]
  %sub87 = sub i32 %tmp85, %tmp86                 ; <i32> [#uses=1]
  %cmp88 = icmp sle i32 %tmp82, %sub87            ; <i1> [#uses=1]
  br i1 %cmp88, label %for.body89, label %for.end

for.body89:                                       ; preds = %for.cond81
  %tmp91 = load i32* %x                           ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %tmp91, i32 0 ; <<2 x i32>> [#uses=1]
  %tmp92 = load i32* %y                           ; <i32> [#uses=1]
  %vecinit93 = insertelement <2 x i32> %vecinit, i32 %tmp92, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit93, <2 x i32>* %sCrd
  %tmp95 = load <2 x i32>* %outCrd.addr           ; <<2 x i32>> [#uses=1]
  %tmp96 = load <2 x i32>* %sCrd                  ; <<2 x i32>> [#uses=1]
  %add97 = add nsw <2 x i32> %tmp95, %tmp96       ; <<2 x i32>> [#uses=1]
  store <2 x i32> %add97, <2 x i32>* %v3
  %tmp99 = load <2 x i32>* %imgSize.addr          ; <<2 x i32>> [#uses=1]
  %tmp100 = extractelement <2 x i32> %tmp99, i32 0 ; <i32> [#uses=1]
  %tmp101 = load <2 x i32>* %v3                   ; <<2 x i32>> [#uses=1]
  %tmp102 = extractelement <2 x i32> %tmp101, i32 1 ; <i32> [#uses=1]
  %mul103 = mul i32 %tmp100, %tmp102              ; <i32> [#uses=1]
  %tmp104 = load <2 x i32>* %v3                   ; <<2 x i32>> [#uses=1]
  %tmp105 = extractelement <2 x i32> %tmp104, i32 0 ; <i32> [#uses=1]
  %add106 = add nsw i32 %mul103, %tmp105          ; <i32> [#uses=1]
  %tmp107 = load <4 x float> addrspace(1)** %inputImage.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx108 = getelementptr inbounds <4 x float> addrspace(1)* %tmp107, i32 %add106 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp109 = load <4 x float> addrspace(1)* %arrayidx108 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp109, <4 x float>* %curVal
  %tmp110 = load <4 x float>* %origSrc            ; <<4 x float>> [#uses=1]
  %tmp111 = load <4 x float>* %curVal             ; <<4 x float>> [#uses=1]
  %sub112 = fsub <4 x float> %tmp110, %tmp111     ; <<4 x float>> [#uses=1]
  %call113 = call <4 x float> @_Z4fabsU8__vector4f(<4 x float> %sub112) ; <<4 x float>> [#uses=1]
  store <4 x float> %call113, <4 x float>* %difference
  %tmp115 = load i32* %x                          ; <i32> [#uses=1]
  %tmp116 = load i32* %x                          ; <i32> [#uses=1]
  %mul117 = mul i32 %tmp115, %tmp116              ; <i32> [#uses=1]
  %tmp118 = load i32* %y                          ; <i32> [#uses=1]
  %tmp119 = load i32* %y                          ; <i32> [#uses=1]
  %mul120 = mul i32 %tmp118, %tmp119              ; <i32> [#uses=1]
  %add121 = add nsw i32 %mul117, %mul120          ; <i32> [#uses=1]
  %neg122 = sub i32 0, %add121                    ; <i32> [#uses=1]
  %conv = sitofp i32 %neg122 to float             ; <float> [#uses=1]
  %tmp123 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp124 = getelementptr inbounds %struct.anon addrspace(2)* %tmp123, i32 0, i32 1 ; <float addrspace(2)*> [#uses=1]
  %tmp125 = load float addrspace(2)* %tmp124      ; <float> [#uses=1]
  %mul126 = fmul float %conv, %tmp125             ; <float> [#uses=1]
  %call127 = call float @_Z3expf(float %mul126)   ; <float> [#uses=1]
  store float %call127, float* %wD
  %tmp129 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp130 = getelementptr inbounds %struct.anon addrspace(2)* %tmp129, i32 0, i32 0 ; <i32 addrspace(2)*> [#uses=1]
  %tmp131 = load i32 addrspace(2)* %tmp130        ; <i32> [#uses=1]
  %tobool = icmp ne i32 %tmp131, 0                ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %for.body89
  %tmp132 = load <4 x float>* %difference         ; <<4 x float>> [#uses=1]
  %tmp133 = insertelement <4 x float> %tmp132, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp133, <4 x float>* %difference
  %tmp134 = load <4 x float>* %difference         ; <<4 x float>> [#uses=1]
  %tmp135 = load <4 x float>* %difference         ; <<4 x float>> [#uses=1]
  %call136 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp134, <4 x float> %tmp135) ; <float> [#uses=1]
  store float %call136, float* %diffLum
  br label %if.end

if.else:                                          ; preds = %for.body89
  %tmp137 = load <4 x float>* %difference         ; <<4 x float>> [#uses=1]
  %call138 = call float @rgbToLuminance(<4 x float> %tmp137) ; <float> [#uses=1]
  store float %call138, float* %diffLum
  %tmp140 = load <4 x float>* %curVal             ; <<4 x float>> [#uses=1]
  %call141 = call float @rgbToLuminance(<4 x float> %tmp140) ; <float> [#uses=1]
  store float %call141, float* %curLum
  %tmp142 = load float* %curLum                   ; <float> [#uses=1]
  %tmp143 = load <4 x float>* %curVal             ; <<4 x float>> [#uses=1]
  %tmp144 = insertelement <4 x float> %tmp143, float %tmp142, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp144, <4 x float>* %curVal
  %tmp145 = load <4 x float>* %curVal             ; <<4 x float>> [#uses=1]
  %tmp146 = extractelement <4 x float> %tmp145, i32 2 ; <float> [#uses=1]
  %tmp147 = load <4 x float>* %curVal             ; <<4 x float>> [#uses=1]
  %tmp148 = insertelement <4 x float> %tmp147, float %tmp146, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp148, <4 x float>* %curVal
  %tmp149 = load <4 x float>* %curVal             ; <<4 x float>> [#uses=1]
  %tmp150 = extractelement <4 x float> %tmp149, i32 1 ; <float> [#uses=1]
  %tmp151 = load <4 x float>* %curVal             ; <<4 x float>> [#uses=1]
  %tmp152 = insertelement <4 x float> %tmp151, float %tmp150, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp152, <4 x float>* %curVal
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %tmp154 = load float* %diffLum                  ; <float> [#uses=1]
  %tmp155 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp156 = getelementptr inbounds %struct.anon addrspace(2)* %tmp155, i32 0, i32 2 ; <float addrspace(2)*> [#uses=1]
  %tmp157 = load float addrspace(2)* %tmp156      ; <float> [#uses=1]
  %neg158 = fsub float -0.000000e+000, %tmp157    ; <float> [#uses=1]
  %mul159 = fmul float %tmp154, %neg158           ; <float> [#uses=1]
  %call160 = call float @_Z3expf(float %mul159)   ; <float> [#uses=1]
  store float %call160, float* %wR
  %tmp161 = load <4 x float>* %curVal             ; <<4 x float>> [#uses=1]
  %tmp162 = load float* %wD                       ; <float> [#uses=1]
  %tmp163 = insertelement <4 x float> undef, float %tmp162, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp163, <4 x float> %tmp163, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul164 = fmul <4 x float> %tmp161, %splat      ; <<4 x float>> [#uses=1]
  %tmp165 = load float* %wR                       ; <float> [#uses=1]
  %tmp166 = insertelement <4 x float> undef, float %tmp165, i32 0 ; <<4 x float>> [#uses=2]
  %splat167 = shufflevector <4 x float> %tmp166, <4 x float> %tmp166, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul168 = fmul <4 x float> %mul164, %splat167   ; <<4 x float>> [#uses=1]
  %tmp169 = load <4 x float>* %dst                ; <<4 x float>> [#uses=1]
  %add170 = fadd <4 x float> %tmp169, %mul168     ; <<4 x float>> [#uses=1]
  store <4 x float> %add170, <4 x float>* %dst
  %tmp171 = load float* %wD                       ; <float> [#uses=1]
  %tmp172 = load float* %wR                       ; <float> [#uses=1]
  %mul173 = fmul float %tmp171, %tmp172           ; <float> [#uses=1]
  %tmp174 = load float* %scale                    ; <float> [#uses=1]
  %add175 = fadd float %tmp174, %mul173           ; <float> [#uses=1]
  store float %add175, float* %scale
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %tmp176 = load i32* %x                          ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp176, 1                   ; <i32> [#uses=1]
  store i32 %inc, i32* %x
  br label %for.cond81

for.end:                                          ; preds = %for.cond81
  br label %for.inc177

for.inc177:                                       ; preds = %for.end
  %tmp178 = load i32* %y                          ; <i32> [#uses=1]
  %inc179 = add nsw i32 %tmp178, 1                ; <i32> [#uses=1]
  store i32 %inc179, i32* %y
  br label %for.cond

for.end180:                                       ; preds = %for.cond
  %tmp181 = load float* %scale                    ; <float> [#uses=1]
  %tmp182 = insertelement <4 x float> undef, float %tmp181, i32 0 ; <<4 x float>> [#uses=2]
  %splat183 = shufflevector <4 x float> %tmp182, <4 x float> %tmp182, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %tmp184 = load <4 x float>* %dst                ; <<4 x float>> [#uses=1]
  %cmp185 = fcmp oeq <4 x float> zeroinitializer, %splat183 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp185, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat183 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %tmp184, %splat183      ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %dst
  %tmp186 = load <4 x float>* %dst                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp186, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z4fabsU8__vector4f(<4 x float>)

; CHECK: ret
define void @intel_bilateral2D_2D(<4 x float> addrspace(1)* %inputImage, <4 x float> addrspace(1)* %output, i32 %uiImageWidth, i32 %uiDevImageHeight, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %inputImage.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %uiImageWidth.addr = alloca i32, align 4        ; <i32*> [#uses=2]
  %uiDevImageHeight.addr = alloca i32, align 4    ; <i32*> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=3]
  %curCrd = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=4]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  store <4 x float> addrspace(1)* %inputImage, <4 x float> addrspace(1)** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %uiImageWidth, i32* %uiImageWidth.addr
  store i32 %uiDevImageHeight, i32* %uiDevImageHeight.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %tmp = load i32* %uiImageWidth.addr             ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %tmp, i32 0 ; <<2 x i32>> [#uses=1]
  %tmp1 = load i32* %uiDevImageHeight.addr        ; <i32> [#uses=1]
  %vecinit2 = insertelement <2 x i32> %vecinit, i32 %tmp1, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit2, <2 x i32>* %imgSize
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  %vecinit4 = insertelement <2 x i32> undef, i32 %call, i32 0 ; <<2 x i32>> [#uses=1]
  %call5 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  %vecinit6 = insertelement <2 x i32> %vecinit4, i32 %call5, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit6, <2 x i32>* %curCrd
  %tmp8 = load <2 x i32>* %curCrd                 ; <<2 x i32>> [#uses=1]
  %tmp9 = extractelement <2 x i32> %tmp8, i32 1   ; <i32> [#uses=1]
  %tmp10 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp11 = extractelement <2 x i32> %tmp10, i32 0 ; <i32> [#uses=1]
  %mul = mul i32 %tmp9, %tmp11                    ; <i32> [#uses=1]
  %tmp12 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp13 = extractelement <2 x i32> %tmp12, i32 0 ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %tmp13                 ; <i32> [#uses=1]
  store i32 %add, i32* %index
  %tmp14 = load <4 x float> addrspace(1)** %inputImage.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp15 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp16 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp17 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call18 = call <4 x float> @evaluatePixel(<4 x float> addrspace(1)* %tmp14, <2 x i32> %tmp15, <2 x i32> %tmp16, %struct.anon addrspace(2)* %tmp17) ; <<4 x float>> [#uses=1]
  %tmp19 = load i32* %index                       ; <i32> [#uses=1]
  %tmp20 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp20, i32 %tmp19 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call18, <4 x float> addrspace(1)* %arrayidx
  ret void
}

declare i32 @get_global_id(i32)

; CHECK: ret
define void @intel_bilateral2DScalar_2D(<4 x float> addrspace(1)* %inputImage, <4 x float> addrspace(1)* %output, i32 %uiImageWidth, i32 %uiDevImageHeight, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %inputImage.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %uiImageWidth.addr = alloca i32, align 4        ; <i32*> [#uses=2]
  %uiDevImageHeight.addr = alloca i32, align 4    ; <i32*> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=3]
  %curCrd = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=4]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  store <4 x float> addrspace(1)* %inputImage, <4 x float> addrspace(1)** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %uiImageWidth, i32* %uiImageWidth.addr
  store i32 %uiDevImageHeight, i32* %uiDevImageHeight.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %tmp = load i32* %uiImageWidth.addr             ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %tmp, i32 0 ; <<2 x i32>> [#uses=1]
  %tmp1 = load i32* %uiDevImageHeight.addr        ; <i32> [#uses=1]
  %vecinit2 = insertelement <2 x i32> %vecinit, i32 %tmp1, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit2, <2 x i32>* %imgSize
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  %vecinit4 = insertelement <2 x i32> undef, i32 %call, i32 0 ; <<2 x i32>> [#uses=1]
  %call5 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  %vecinit6 = insertelement <2 x i32> %vecinit4, i32 %call5, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit6, <2 x i32>* %curCrd
  %tmp8 = load <2 x i32>* %curCrd                 ; <<2 x i32>> [#uses=1]
  %tmp9 = extractelement <2 x i32> %tmp8, i32 1   ; <i32> [#uses=1]
  %tmp10 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp11 = extractelement <2 x i32> %tmp10, i32 0 ; <i32> [#uses=1]
  %mul = mul i32 %tmp9, %tmp11                    ; <i32> [#uses=1]
  %tmp12 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp13 = extractelement <2 x i32> %tmp12, i32 0 ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %tmp13                 ; <i32> [#uses=1]
  store i32 %add, i32* %index
  %tmp14 = load <4 x float> addrspace(1)** %inputImage.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %conv = bitcast <4 x float> addrspace(1)* %tmp14 to float addrspace(1)* ; <float addrspace(1)*> [#uses=1]
  %tmp15 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp16 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp17 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call18 = call <4 x float> @evaluatePixelScalar(float addrspace(1)* %conv, <2 x i32> %tmp15, <2 x i32> %tmp16, %struct.anon addrspace(2)* %tmp17) ; <<4 x float>> [#uses=1]
  %tmp19 = load i32* %index                       ; <i32> [#uses=1]
  %tmp20 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp20, i32 %tmp19 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call18, <4 x float> addrspace(1)* %arrayidx
  ret void
}

; CHECK: ret
define void @intel_bilateral2D(<4 x float> addrspace(1)* %inputImage, <4 x float> addrspace(1)* %output, i32 %rowCountPerGlobalID, i32 %uiImageWidth, i32 %uiDevImageHeight, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %inputImage.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %rowCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=4]
  %uiImageWidth.addr = alloca i32, align 4        ; <i32*> [#uses=2]
  %uiDevImageHeight.addr = alloca i32, align 4    ; <i32*> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=8]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=6]
  %lastRow = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %curCrd = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=5]
  %col = alloca i32, align 4                      ; <i32*> [#uses=5]
  store <4 x float> addrspace(1)* %inputImage, <4 x float> addrspace(1)** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %rowCountPerGlobalID, i32* %rowCountPerGlobalID.addr
  store i32 %uiImageWidth, i32* %uiImageWidth.addr
  store i32 %uiDevImageHeight, i32* %uiDevImageHeight.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp = load i32* %rowCountPerGlobalID.addr      ; <i32> [#uses=1]
  %tmp1 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %row
  %tmp3 = load i32* %uiImageWidth.addr            ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %tmp3, i32 0 ; <<2 x i32>> [#uses=1]
  %tmp4 = load i32* %uiDevImageHeight.addr        ; <i32> [#uses=1]
  %vecinit5 = insertelement <2 x i32> %vecinit, i32 %tmp4, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit5, <2 x i32>* %imgSize
  %tmp7 = load i32* %row                          ; <i32> [#uses=1]
  %tmp8 = load i32* %rowCountPerGlobalID.addr     ; <i32> [#uses=1]
  %add = add i32 %tmp7, %tmp8                     ; <i32> [#uses=1]
  %tmp9 = load <2 x i32>* %imgSize                ; <<2 x i32>> [#uses=1]
  %tmp10 = extractelement <2 x i32> %tmp9, i32 1  ; <i32> [#uses=1]
  %cmp = icmp ult i32 %add, %tmp10                ; <i1> [#uses=1]
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %tmp11 = load i32* %row                         ; <i32> [#uses=1]
  %tmp12 = load i32* %rowCountPerGlobalID.addr    ; <i32> [#uses=1]
  %add13 = add i32 %tmp11, %tmp12                 ; <i32> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %entry
  %tmp14 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp15 = extractelement <2 x i32> %tmp14, i32 1 ; <i32> [#uses=1]
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %add13, %cond.true ], [ %tmp15, %cond.false ] ; <i32> [#uses=1]
  store i32 %cond, i32* %lastRow
  %tmp17 = load i32* %row                         ; <i32> [#uses=1]
  %tmp18 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp19 = extractelement <2 x i32> %tmp18, i32 0 ; <i32> [#uses=1]
  %mul20 = mul i32 %tmp17, %tmp19                 ; <i32> [#uses=1]
  store i32 %mul20, i32* %index
  br label %for.cond

for.cond:                                         ; preds = %for.inc47, %cond.end
  %tmp21 = load i32* %row                         ; <i32> [#uses=1]
  %tmp22 = load i32* %lastRow                     ; <i32> [#uses=1]
  %cmp23 = icmp slt i32 %tmp21, %tmp22            ; <i1> [#uses=1]
  br i1 %cmp23, label %for.body, label %for.end50

for.body:                                         ; preds = %for.cond
  %tmp25 = load i32* %row                         ; <i32> [#uses=1]
  %tmp26 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp27 = insertelement <2 x i32> %tmp26, i32 %tmp25, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp27, <2 x i32>* %curCrd
  store i32 0, i32* %col
  br label %for.cond29

for.cond29:                                       ; preds = %for.inc, %for.body
  %tmp30 = load i32* %col                         ; <i32> [#uses=1]
  %tmp31 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp32 = extractelement <2 x i32> %tmp31, i32 0 ; <i32> [#uses=1]
  %cmp33 = icmp slt i32 %tmp30, %tmp32            ; <i1> [#uses=1]
  br i1 %cmp33, label %for.body34, label %for.end

for.body34:                                       ; preds = %for.cond29
  %tmp35 = load i32* %col                         ; <i32> [#uses=1]
  %tmp36 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp37 = insertelement <2 x i32> %tmp36, i32 %tmp35, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp37, <2 x i32>* %curCrd
  %tmp38 = load <4 x float> addrspace(1)** %inputImage.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp39 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp40 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp41 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call42 = call <4 x float> @evaluatePixel(<4 x float> addrspace(1)* %tmp38, <2 x i32> %tmp39, <2 x i32> %tmp40, %struct.anon addrspace(2)* %tmp41) ; <<4 x float>> [#uses=1]
  %tmp43 = load i32* %index                       ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp43, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp44 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp44, i32 %tmp43 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call42, <4 x float> addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body34
  %tmp45 = load i32* %col                         ; <i32> [#uses=1]
  %inc46 = add nsw i32 %tmp45, 1                  ; <i32> [#uses=1]
  store i32 %inc46, i32* %col
  br label %for.cond29

for.end:                                          ; preds = %for.cond29
  br label %for.inc47

for.inc47:                                        ; preds = %for.end
  %tmp48 = load i32* %row                         ; <i32> [#uses=1]
  %inc49 = add nsw i32 %tmp48, 1                  ; <i32> [#uses=1]
  store i32 %inc49, i32* %row
  br label %for.cond

for.end50:                                        ; preds = %for.cond
  ret void
}

; CHECK: ret
define void @intel_bilateral2DScalar(<4 x float> addrspace(1)* %inputImage, <4 x float> addrspace(1)* %output, i32 %rowCountPerGlobalID, i32 %uiImageWidth, i32 %uiDevImageHeight, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %inputImage.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %rowCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=4]
  %uiImageWidth.addr = alloca i32, align 4        ; <i32*> [#uses=2]
  %uiDevImageHeight.addr = alloca i32, align 4    ; <i32*> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=8]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=6]
  %lastRow = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %curCrd = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=5]
  %col = alloca i32, align 4                      ; <i32*> [#uses=5]
  store <4 x float> addrspace(1)* %inputImage, <4 x float> addrspace(1)** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %rowCountPerGlobalID, i32* %rowCountPerGlobalID.addr
  store i32 %uiImageWidth, i32* %uiImageWidth.addr
  store i32 %uiDevImageHeight, i32* %uiDevImageHeight.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp = load i32* %rowCountPerGlobalID.addr      ; <i32> [#uses=1]
  %tmp1 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %row
  %tmp3 = load i32* %uiImageWidth.addr            ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %tmp3, i32 0 ; <<2 x i32>> [#uses=1]
  %tmp4 = load i32* %uiDevImageHeight.addr        ; <i32> [#uses=1]
  %vecinit5 = insertelement <2 x i32> %vecinit, i32 %tmp4, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit5, <2 x i32>* %imgSize
  %tmp7 = load i32* %row                          ; <i32> [#uses=1]
  %tmp8 = load i32* %rowCountPerGlobalID.addr     ; <i32> [#uses=1]
  %add = add i32 %tmp7, %tmp8                     ; <i32> [#uses=1]
  %tmp9 = load <2 x i32>* %imgSize                ; <<2 x i32>> [#uses=1]
  %tmp10 = extractelement <2 x i32> %tmp9, i32 1  ; <i32> [#uses=1]
  %cmp = icmp ult i32 %add, %tmp10                ; <i1> [#uses=1]
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %tmp11 = load i32* %row                         ; <i32> [#uses=1]
  %tmp12 = load i32* %rowCountPerGlobalID.addr    ; <i32> [#uses=1]
  %add13 = add i32 %tmp11, %tmp12                 ; <i32> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %entry
  %tmp14 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp15 = extractelement <2 x i32> %tmp14, i32 1 ; <i32> [#uses=1]
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %add13, %cond.true ], [ %tmp15, %cond.false ] ; <i32> [#uses=1]
  store i32 %cond, i32* %lastRow
  %tmp17 = load i32* %row                         ; <i32> [#uses=1]
  %tmp18 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp19 = extractelement <2 x i32> %tmp18, i32 0 ; <i32> [#uses=1]
  %mul20 = mul i32 %tmp17, %tmp19                 ; <i32> [#uses=1]
  store i32 %mul20, i32* %index
  br label %for.cond

for.cond:                                         ; preds = %for.inc47, %cond.end
  %tmp21 = load i32* %row                         ; <i32> [#uses=1]
  %tmp22 = load i32* %lastRow                     ; <i32> [#uses=1]
  %cmp23 = icmp slt i32 %tmp21, %tmp22            ; <i1> [#uses=1]
  br i1 %cmp23, label %for.body, label %for.end50

for.body:                                         ; preds = %for.cond
  %tmp25 = load i32* %row                         ; <i32> [#uses=1]
  %tmp26 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp27 = insertelement <2 x i32> %tmp26, i32 %tmp25, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp27, <2 x i32>* %curCrd
  store i32 0, i32* %col
  br label %for.cond29

for.cond29:                                       ; preds = %for.inc, %for.body
  %tmp30 = load i32* %col                         ; <i32> [#uses=1]
  %tmp31 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp32 = extractelement <2 x i32> %tmp31, i32 0 ; <i32> [#uses=1]
  %cmp33 = icmp slt i32 %tmp30, %tmp32            ; <i1> [#uses=1]
  br i1 %cmp33, label %for.body34, label %for.end

for.body34:                                       ; preds = %for.cond29
  %tmp35 = load i32* %col                         ; <i32> [#uses=1]
  %tmp36 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp37 = insertelement <2 x i32> %tmp36, i32 %tmp35, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp37, <2 x i32>* %curCrd
  %tmp38 = load <4 x float> addrspace(1)** %inputImage.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %conv = bitcast <4 x float> addrspace(1)* %tmp38 to float addrspace(1)* ; <float addrspace(1)*> [#uses=1]
  %tmp39 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp40 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp41 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call42 = call <4 x float> @evaluatePixelScalar(float addrspace(1)* %conv, <2 x i32> %tmp39, <2 x i32> %tmp40, %struct.anon addrspace(2)* %tmp41) ; <<4 x float>> [#uses=1]
  %tmp43 = load i32* %index                       ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp43, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp44 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp44, i32 %tmp43 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call42, <4 x float> addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body34
  %tmp45 = load i32* %col                         ; <i32> [#uses=1]
  %inc46 = add nsw i32 %tmp45, 1                  ; <i32> [#uses=1]
  store i32 %inc46, i32* %col
  br label %for.cond29

for.end:                                          ; preds = %for.cond29
  br label %for.inc47

for.inc47:                                        ; preds = %for.end
  %tmp48 = load i32* %row                         ; <i32> [#uses=1]
  %inc49 = add nsw i32 %tmp48, 1                  ; <i32> [#uses=1]
  store i32 %inc49, i32* %row
  br label %for.cond

for.end50:                                        ; preds = %for.cond
  ret void
}
