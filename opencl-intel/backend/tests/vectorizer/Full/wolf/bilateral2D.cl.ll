; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\bilateral2D.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct._image2d_t = type opaque
%struct.anon = type <{ i32, float, float, i32, i32 }>

@opencl_evaluateDependents_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_evaluateDependents_parameters = appending global [86 x i8] c"float2 const, float const, uint const, kernelArgs __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[86 x i8]*> [#uses=1]
@opencl_bilateral2D_2D_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_bilateral2D_2D_parameters = appending global [107 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[107 x i8]*> [#uses=1]
@opencl_bilateral2D_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_bilateral2D_parameters = appending global [119 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, uint const, kernelArgs __attribute__((address_space(2))) *\00", section "llvm.metadata" ; <[119 x i8]*> [#uses=1]
@opencl_metadata = appending global [3 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<2 x float>, float, i32, %struct.anon addrspace(1)*)* @evaluateDependents to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_evaluateDependents_locals to i8*), i8* getelementptr inbounds ([86 x i8]* @opencl_evaluateDependents_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, %struct.anon addrspace(2)*)* @bilateral2D_2D to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_bilateral2D_2D_locals to i8*), i8* getelementptr inbounds ([107 x i8]* @opencl_bilateral2D_2D_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, i32, %struct.anon addrspace(2)*)* @bilateral2D to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_bilateral2D_locals to i8*), i8* getelementptr inbounds ([119 x i8]* @opencl_bilateral2D_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[3 x %opencl_metadata_type]*> [#uses=0]

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
define <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x i32> %outCrd, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=3]
  %outCrd.addr = alloca <2 x i32>, align 8        ; <<2 x i32>*> [#uses=3]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=8]
  %samplerNearest = alloca i32, align 4           ; <i32*> [#uses=1]
  %kZero = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  %iZero = alloca <2 x i32>, align 8              ; <<2 x i32>*> [#uses=1]
  %origSrc = alloca <4 x float>, align 16         ; <<4 x float>*> [#uses=2]
  %dst = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=6]
  %scale = alloca float, align 4                  ; <float*> [#uses=4]
  %difference = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=6]
  %x = alloca i32, align 4                        ; <i32*> [#uses=7]
  %y = alloca i32, align 4                        ; <i32*> [#uses=7]
  %sCrd = alloca <2 x i32>, align 8               ; <<2 x i32>*> [#uses=2]
  %curVal = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=12]
  %wD = alloca float, align 4                     ; <float*> [#uses=3]
  %diffLum = alloca float, align 4                ; <float*> [#uses=3]
  %curLum = alloca float, align 4                 ; <float*> [#uses=2]
  %wR = alloca float, align 4                     ; <float*> [#uses=3]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <2 x i32> %outCrd, <2 x i32>* %outCrd.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  store i32 1, i32* %samplerNearest
  store <4 x float> zeroinitializer, <4 x float>* %kZero
  store <2 x i32> zeroinitializer, <2 x i32>* %iZero
  %tmp = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp1 = load <2 x i32>* %outCrd.addr            ; <<2 x i32>> [#uses=1]
  %call = call <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2i(%struct._image2d_t* %tmp, i32 1, <2 x i32> %tmp1) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %origSrc
  %tmp3 = load <4 x float>* %kZero                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp3, <4 x float>* %dst
  store float 0.000000e+000, float* %scale
  %tmp7 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp8 = getelementptr inbounds %struct.anon addrspace(2)* %tmp7, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp9 = load i32 addrspace(2)* %tmp8            ; <i32> [#uses=1]
  %neg = sub i32 0, %tmp9                         ; <i32> [#uses=1]
  store i32 %neg, i32* %x
  br label %for.cond

for.cond:                                         ; preds = %for.inc100, %entry
  %tmp10 = load i32* %x                           ; <i32> [#uses=1]
  %tmp11 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp12 = getelementptr inbounds %struct.anon addrspace(2)* %tmp11, i32 0, i32 3 ; <i32 addrspace(2)*> [#uses=1]
  %tmp13 = load i32 addrspace(2)* %tmp12          ; <i32> [#uses=1]
  %cmp = icmp sle i32 %tmp10, %tmp13              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end103

for.body:                                         ; preds = %for.cond
  %tmp15 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp16 = getelementptr inbounds %struct.anon addrspace(2)* %tmp15, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp17 = load i32 addrspace(2)* %tmp16          ; <i32> [#uses=1]
  %neg18 = sub i32 0, %tmp17                      ; <i32> [#uses=1]
  store i32 %neg18, i32* %y
  br label %for.cond19

for.cond19:                                       ; preds = %for.inc, %for.body
  %tmp20 = load i32* %y                           ; <i32> [#uses=1]
  %tmp21 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp22 = getelementptr inbounds %struct.anon addrspace(2)* %tmp21, i32 0, i32 4 ; <i32 addrspace(2)*> [#uses=1]
  %tmp23 = load i32 addrspace(2)* %tmp22          ; <i32> [#uses=1]
  %cmp24 = icmp sle i32 %tmp20, %tmp23            ; <i1> [#uses=1]
  br i1 %cmp24, label %for.body25, label %for.end

for.body25:                                       ; preds = %for.cond19
  %tmp27 = load i32* %x                           ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %tmp27, i32 0 ; <<2 x i32>> [#uses=1]
  %tmp28 = load i32* %y                           ; <i32> [#uses=1]
  %vecinit29 = insertelement <2 x i32> %vecinit, i32 %tmp28, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit29, <2 x i32>* %sCrd
  %tmp31 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp32 = load <2 x i32>* %outCrd.addr           ; <<2 x i32>> [#uses=1]
  %tmp33 = load <2 x i32>* %sCrd                  ; <<2 x i32>> [#uses=1]
  %add = add nsw <2 x i32> %tmp32, %tmp33         ; <<2 x i32>> [#uses=1]
  %call34 = call <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2i(%struct._image2d_t* %tmp31, i32 1, <2 x i32> %add) ; <<4 x float>> [#uses=1]
  store <4 x float> %call34, <4 x float>* %curVal
  %tmp35 = load <4 x float>* %origSrc             ; <<4 x float>> [#uses=1]
  %tmp36 = load <4 x float>* %curVal              ; <<4 x float>> [#uses=1]
  %sub = fsub <4 x float> %tmp35, %tmp36          ; <<4 x float>> [#uses=1]
  %call37 = call <4 x float> @_Z4fabsU8__vector4f(<4 x float> %sub) ; <<4 x float>> [#uses=1]
  store <4 x float> %call37, <4 x float>* %difference
  %tmp39 = load i32* %x                           ; <i32> [#uses=1]
  %tmp40 = load i32* %x                           ; <i32> [#uses=1]
  %mul = mul i32 %tmp39, %tmp40                   ; <i32> [#uses=1]
  %tmp41 = load i32* %y                           ; <i32> [#uses=1]
  %tmp42 = load i32* %y                           ; <i32> [#uses=1]
  %mul43 = mul i32 %tmp41, %tmp42                 ; <i32> [#uses=1]
  %add44 = add nsw i32 %mul, %mul43               ; <i32> [#uses=1]
  %neg45 = sub i32 0, %add44                      ; <i32> [#uses=1]
  %conv = sitofp i32 %neg45 to float              ; <float> [#uses=1]
  %tmp46 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp47 = getelementptr inbounds %struct.anon addrspace(2)* %tmp46, i32 0, i32 1 ; <float addrspace(2)*> [#uses=1]
  %tmp48 = load float addrspace(2)* %tmp47        ; <float> [#uses=1]
  %mul49 = fmul float %conv, %tmp48               ; <float> [#uses=1]
  %call50 = call float @_Z3expf(float %mul49)     ; <float> [#uses=1]
  store float %call50, float* %wD
  %tmp52 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp53 = getelementptr inbounds %struct.anon addrspace(2)* %tmp52, i32 0, i32 0 ; <i32 addrspace(2)*> [#uses=1]
  %tmp54 = load i32 addrspace(2)* %tmp53          ; <i32> [#uses=1]
  %tobool = icmp ne i32 %tmp54, 0                 ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %for.body25
  %tmp55 = load <4 x float>* %difference          ; <<4 x float>> [#uses=1]
  %tmp56 = insertelement <4 x float> %tmp55, float 0.000000e+000, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp56, <4 x float>* %difference
  %tmp57 = load <4 x float>* %difference          ; <<4 x float>> [#uses=1]
  %tmp58 = load <4 x float>* %difference          ; <<4 x float>> [#uses=1]
  %call59 = call float @_Z3dotU8__vector4fS_(<4 x float> %tmp57, <4 x float> %tmp58) ; <float> [#uses=1]
  store float %call59, float* %diffLum
  br label %if.end

if.else:                                          ; preds = %for.body25
  %tmp60 = load <4 x float>* %difference          ; <<4 x float>> [#uses=1]
  %call61 = call float @rgbToLuminance(<4 x float> %tmp60) ; <float> [#uses=1]
  store float %call61, float* %diffLum
  %tmp63 = load <4 x float>* %curVal              ; <<4 x float>> [#uses=1]
  %call64 = call float @rgbToLuminance(<4 x float> %tmp63) ; <float> [#uses=1]
  store float %call64, float* %curLum
  %tmp65 = load float* %curLum                    ; <float> [#uses=1]
  %tmp66 = load <4 x float>* %curVal              ; <<4 x float>> [#uses=1]
  %tmp67 = insertelement <4 x float> %tmp66, float %tmp65, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp67, <4 x float>* %curVal
  %tmp68 = load <4 x float>* %curVal              ; <<4 x float>> [#uses=1]
  %tmp69 = extractelement <4 x float> %tmp68, i32 2 ; <float> [#uses=1]
  %tmp70 = load <4 x float>* %curVal              ; <<4 x float>> [#uses=1]
  %tmp71 = insertelement <4 x float> %tmp70, float %tmp69, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp71, <4 x float>* %curVal
  %tmp72 = load <4 x float>* %curVal              ; <<4 x float>> [#uses=1]
  %tmp73 = extractelement <4 x float> %tmp72, i32 1 ; <float> [#uses=1]
  %tmp74 = load <4 x float>* %curVal              ; <<4 x float>> [#uses=1]
  %tmp75 = insertelement <4 x float> %tmp74, float %tmp73, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp75, <4 x float>* %curVal
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %tmp77 = load float* %diffLum                   ; <float> [#uses=1]
  %tmp78 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %tmp79 = getelementptr inbounds %struct.anon addrspace(2)* %tmp78, i32 0, i32 2 ; <float addrspace(2)*> [#uses=1]
  %tmp80 = load float addrspace(2)* %tmp79        ; <float> [#uses=1]
  %neg81 = fsub float -0.000000e+000, %tmp80      ; <float> [#uses=1]
  %mul82 = fmul float %tmp77, %neg81              ; <float> [#uses=1]
  %call83 = call float @_Z3expf(float %mul82)     ; <float> [#uses=1]
  store float %call83, float* %wR
  %tmp84 = load <4 x float>* %curVal              ; <<4 x float>> [#uses=1]
  %tmp85 = load float* %wD                        ; <float> [#uses=1]
  %tmp86 = insertelement <4 x float> undef, float %tmp85, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp86, <4 x float> %tmp86, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul87 = fmul <4 x float> %tmp84, %splat        ; <<4 x float>> [#uses=1]
  %tmp88 = load float* %wR                        ; <float> [#uses=1]
  %tmp89 = insertelement <4 x float> undef, float %tmp88, i32 0 ; <<4 x float>> [#uses=2]
  %splat90 = shufflevector <4 x float> %tmp89, <4 x float> %tmp89, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %mul91 = fmul <4 x float> %mul87, %splat90      ; <<4 x float>> [#uses=1]
  %tmp92 = load <4 x float>* %dst                 ; <<4 x float>> [#uses=1]
  %add93 = fadd <4 x float> %tmp92, %mul91        ; <<4 x float>> [#uses=1]
  store <4 x float> %add93, <4 x float>* %dst
  %tmp94 = load float* %wD                        ; <float> [#uses=1]
  %tmp95 = load float* %wR                        ; <float> [#uses=1]
  %mul96 = fmul float %tmp94, %tmp95              ; <float> [#uses=1]
  %tmp97 = load float* %scale                     ; <float> [#uses=1]
  %add98 = fadd float %tmp97, %mul96              ; <float> [#uses=1]
  store float %add98, float* %scale
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %tmp99 = load i32* %y                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp99, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %y
  br label %for.cond19

for.end:                                          ; preds = %for.cond19
  br label %for.inc100

for.inc100:                                       ; preds = %for.end
  %tmp101 = load i32* %x                          ; <i32> [#uses=1]
  %inc102 = add nsw i32 %tmp101, 1                ; <i32> [#uses=1]
  store i32 %inc102, i32* %x
  br label %for.cond

for.end103:                                       ; preds = %for.cond
  %tmp104 = load float* %scale                    ; <float> [#uses=1]
  %tmp105 = insertelement <4 x float> undef, float %tmp104, i32 0 ; <<4 x float>> [#uses=2]
  %splat106 = shufflevector <4 x float> %tmp105, <4 x float> %tmp105, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %tmp107 = load <4 x float>* %dst                ; <<4 x float>> [#uses=1]
  %cmp108 = fcmp oeq <4 x float> zeroinitializer, %splat106 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp108, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat106 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %tmp107, %splat106      ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %dst
  %tmp109 = load <4 x float>* %dst                ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp109, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2i(%struct._image2d_t*, i32, <2 x i32>)

declare <4 x float> @_Z4fabsU8__vector4f(<4 x float>)

declare float @_Z3expf(float)

; CHECK: ret
define void @bilateral2D_2D(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=3]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=2]
  %curCrd = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=4]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %tmp = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %call = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %tmp) ; <<2 x i32>> [#uses=1]
  store <2 x i32> %call, <2 x i32>* %imgSize
  %call2 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %call2, i32 0 ; <<2 x i32>> [#uses=1]
  %call3 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  %vecinit4 = insertelement <2 x i32> %vecinit, i32 %call3, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit4, <2 x i32>* %curCrd
  %tmp6 = load <2 x i32>* %curCrd                 ; <<2 x i32>> [#uses=1]
  %tmp7 = extractelement <2 x i32> %tmp6, i32 1   ; <i32> [#uses=1]
  %tmp8 = load <2 x i32>* %imgSize                ; <<2 x i32>> [#uses=1]
  %tmp9 = extractelement <2 x i32> %tmp8, i32 0   ; <i32> [#uses=1]
  %mul = mul i32 %tmp7, %tmp9                     ; <i32> [#uses=1]
  %tmp10 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp11 = extractelement <2 x i32> %tmp10, i32 0 ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %tmp11                 ; <i32> [#uses=1]
  store i32 %add, i32* %index
  %tmp12 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp13 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp14 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call15 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp12, <2 x i32> %tmp13, %struct.anon addrspace(2)* %tmp14) ; <<4 x float>> [#uses=1]
  %tmp16 = load i32* %index                       ; <i32> [#uses=1]
  %tmp17 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp17, i32 %tmp16 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call15, <4 x float> addrspace(1)* %arrayidx
  ret void
}

declare <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t*)

declare i32 @get_global_id(i32)

; CHECK: ret
define void @bilateral2D(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, i32 %rowCountPerGlobalID, %struct.anon addrspace(2)* %pArgs) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=3]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %rowCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=4]
  %pArgs.addr = alloca %struct.anon addrspace(2)*, align 4 ; <%struct.anon addrspace(2)**> [#uses=2]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=8]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=5]
  %lastRow = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %curCrd = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=5]
  %col = alloca i32, align 4                      ; <i32*> [#uses=5]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %rowCountPerGlobalID, i32* %rowCountPerGlobalID.addr
  store %struct.anon addrspace(2)* %pArgs, %struct.anon addrspace(2)** %pArgs.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp = load i32* %rowCountPerGlobalID.addr      ; <i32> [#uses=1]
  %tmp1 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %row
  %tmp3 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %call4 = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %tmp3) ; <<2 x i32>> [#uses=1]
  store <2 x i32> %call4, <2 x i32>* %imgSize
  %tmp6 = load i32* %row                          ; <i32> [#uses=1]
  %tmp7 = load i32* %rowCountPerGlobalID.addr     ; <i32> [#uses=1]
  %add = add i32 %tmp6, %tmp7                     ; <i32> [#uses=1]
  %tmp8 = load <2 x i32>* %imgSize                ; <<2 x i32>> [#uses=1]
  %tmp9 = extractelement <2 x i32> %tmp8, i32 1   ; <i32> [#uses=1]
  %cmp = icmp ult i32 %add, %tmp9                 ; <i1> [#uses=1]
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %tmp10 = load i32* %row                         ; <i32> [#uses=1]
  %tmp11 = load i32* %rowCountPerGlobalID.addr    ; <i32> [#uses=1]
  %add12 = add i32 %tmp10, %tmp11                 ; <i32> [#uses=1]
  br label %cond.end

cond.false:                                       ; preds = %entry
  %tmp13 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp14 = extractelement <2 x i32> %tmp13, i32 1 ; <i32> [#uses=1]
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %add12, %cond.true ], [ %tmp14, %cond.false ] ; <i32> [#uses=1]
  store i32 %cond, i32* %lastRow
  %tmp16 = load i32* %row                         ; <i32> [#uses=1]
  %tmp17 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp18 = extractelement <2 x i32> %tmp17, i32 0 ; <i32> [#uses=1]
  %mul19 = mul i32 %tmp16, %tmp18                 ; <i32> [#uses=1]
  store i32 %mul19, i32* %index
  br label %for.cond

for.cond:                                         ; preds = %for.inc45, %cond.end
  %tmp20 = load i32* %row                         ; <i32> [#uses=1]
  %tmp21 = load i32* %lastRow                     ; <i32> [#uses=1]
  %cmp22 = icmp slt i32 %tmp20, %tmp21            ; <i1> [#uses=1]
  br i1 %cmp22, label %for.body, label %for.end48

for.body:                                         ; preds = %for.cond
  %tmp24 = load i32* %row                         ; <i32> [#uses=1]
  %tmp25 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp26 = insertelement <2 x i32> %tmp25, i32 %tmp24, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp26, <2 x i32>* %curCrd
  store i32 0, i32* %col
  br label %for.cond28

for.cond28:                                       ; preds = %for.inc, %for.body
  %tmp29 = load i32* %col                         ; <i32> [#uses=1]
  %tmp30 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp31 = extractelement <2 x i32> %tmp30, i32 0 ; <i32> [#uses=1]
  %cmp32 = icmp slt i32 %tmp29, %tmp31            ; <i1> [#uses=1]
  br i1 %cmp32, label %for.body33, label %for.end

for.body33:                                       ; preds = %for.cond28
  %tmp34 = load i32* %col                         ; <i32> [#uses=1]
  %tmp35 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp36 = insertelement <2 x i32> %tmp35, i32 %tmp34, i32 0 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %tmp36, <2 x i32>* %curCrd
  %tmp37 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp38 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp39 = load %struct.anon addrspace(2)** %pArgs.addr ; <%struct.anon addrspace(2)*> [#uses=1]
  %call40 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp37, <2 x i32> %tmp38, %struct.anon addrspace(2)* %tmp39) ; <<4 x float>> [#uses=1]
  %tmp41 = load i32* %index                       ; <i32> [#uses=2]
  %inc = add nsw i32 %tmp41, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp42 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp42, i32 %tmp41 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call40, <4 x float> addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body33
  %tmp43 = load i32* %col                         ; <i32> [#uses=1]
  %inc44 = add nsw i32 %tmp43, 1                  ; <i32> [#uses=1]
  store i32 %inc44, i32* %col
  br label %for.cond28

for.end:                                          ; preds = %for.cond28
  br label %for.inc45

for.inc45:                                        ; preds = %for.end
  %tmp46 = load i32* %row                         ; <i32> [#uses=1]
  %inc47 = add nsw i32 %tmp46, 1                  ; <i32> [#uses=1]
  store i32 %inc47, i32* %row
  br label %for.cond

for.end48:                                        ; preds = %for.cond
  ret void
}
