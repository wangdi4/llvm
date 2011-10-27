; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\gauss.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct._image2d_t = type opaque

@centerIndex = constant float 5.050000e+001, align 4 ; <float*> [#uses=2]
@blurDirection = common addrspace(1) global <2 x float> zeroinitializer, align 8 ; <<2 x float> addrspace(1)*> [#uses=3]
@weights = common addrspace(1) global [101 x float] zeroinitializer, align 4 ; <[101 x float] addrspace(1)*> [#uses=1]
@direction = common addrspace(1) global i32 0, align 4 ; <i32 addrspace(1)*> [#uses=2]
@radius = common addrspace(1) global float 0.000000e+000, align 4 ; <float addrspace(1)*> [#uses=2]
@opencl_getWeightsSize_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_getWeightsSize_parameters = appending global [55 x i8] c"float const, float __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[55 x i8]*> [#uses=1]
@opencl_evaluateDependents_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_evaluateDependents_parameters = appending global [24 x i8] c"uint const, float const\00", section "llvm.metadata" ; <[24 x i8]*> [#uses=1]
@opencl_gauss_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_gauss_parameters = appending global [71 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[71 x i8]*> [#uses=1]
@opencl_metadata = appending global [3 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float, float addrspace(1)*)* @getWeightsSize to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_getWeightsSize_locals to i8*), i8* getelementptr inbounds ([55 x i8]* @opencl_getWeightsSize_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (i32, float)* @evaluateDependents to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_evaluateDependents_locals to i8*), i8* getelementptr inbounds ([24 x i8]* @opencl_evaluateDependents_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, i32)* @gauss to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_gauss_locals to i8*), i8* getelementptr inbounds ([71 x i8]* @opencl_gauss_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[3 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x float> %outCrd) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=3]
  %outCrd.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=2]
  %colAccum = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=6]
  %weightAccum = alloca float, align 4            ; <float*> [#uses=4]
  %floatIndex = alloca float, align 4             ; <float*> [#uses=4]
  %currentPixelLoc = alloca <2 x float>, align 8  ; <<2 x float>*> [#uses=2]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=1]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %sourceLoc = alloca <2 x float>, align 8        ; <<2 x float>*> [#uses=2]
  %sampleNearest = alloca i32, align 4            ; <i32*> [#uses=1]
  %ReadPixelLoc = alloca <2 x float>, align 8     ; <<2 x float>*> [#uses=2]
  %temp = alloca <4 x float>, align 16            ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <2 x float> %outCrd, <2 x float>* %outCrd.addr
  store <4 x float> zeroinitializer, <4 x float>* %colAccum
  store float 0.000000e+000, float* %weightAccum
  %tmp = load float* @centerIndex                 ; <float> [#uses=1]
  %mul = fmul float -1.000000e+000, %tmp          ; <float> [#uses=1]
  store float %mul, float* %floatIndex
  %tmp2 = load <2 x float>* %outCrd.addr          ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp2, <2 x float>* %currentPixelLoc
  %tmp4 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %call = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %tmp4) ; <<2 x i32>> [#uses=1]
  store <2 x i32> %call, <2 x i32>* %imgSize
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp6 = load i32* %i                            ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp6, 101                  ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp8 = load <2 x float> addrspace(1)* @blurDirection ; <<2 x float>> [#uses=1]
  %tmp9 = load float* %floatIndex                 ; <float> [#uses=1]
  %tmp10 = insertelement <2 x float> undef, float %tmp9, i32 0 ; <<2 x float>> [#uses=2]
  %splat = shufflevector <2 x float> %tmp10, <2 x float> %tmp10, <2 x i32> zeroinitializer ; <<2 x float>> [#uses=1]
  %mul11 = fmul <2 x float> %tmp8, %splat         ; <<2 x float>> [#uses=1]
  store <2 x float> %mul11, <2 x float>* %sourceLoc
  store i32 1, i32* %sampleNearest
  %tmp14 = load <2 x float>* %currentPixelLoc     ; <<2 x float>> [#uses=1]
  %tmp15 = load <2 x float>* %sourceLoc           ; <<2 x float>> [#uses=1]
  %add = fadd <2 x float> %tmp14, %tmp15          ; <<2 x float>> [#uses=1]
  store <2 x float> %add, <2 x float>* %ReadPixelLoc
  %tmp17 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp18 = load <2 x float>* %ReadPixelLoc        ; <<2 x float>> [#uses=1]
  %call19 = call <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2f(%struct._image2d_t* %tmp17, i32 1, <2 x float> %tmp18) ; <<4 x float>> [#uses=1]
  store <4 x float> %call19, <4 x float>* %temp
  %tmp20 = load i32* %i                           ; <i32> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* getelementptr inbounds ([101 x float] addrspace(1)* @weights, i32 0, i32 0), i32 %tmp20 ; <float addrspace(1)*> [#uses=1]
  %tmp21 = load float addrspace(1)* %arrayidx     ; <float> [#uses=1]
  %tmp22 = insertelement <4 x float> undef, float %tmp21, i32 0 ; <<4 x float>> [#uses=2]
  %splat23 = shufflevector <4 x float> %tmp22, <4 x float> %tmp22, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %tmp24 = load <4 x float>* %temp                ; <<4 x float>> [#uses=1]
  %mul25 = fmul <4 x float> %splat23, %tmp24      ; <<4 x float>> [#uses=1]
  %tmp26 = load <4 x float>* %colAccum            ; <<4 x float>> [#uses=1]
  %add27 = fadd <4 x float> %tmp26, %mul25        ; <<4 x float>> [#uses=1]
  store <4 x float> %add27, <4 x float>* %colAccum
  %tmp28 = load i32* %i                           ; <i32> [#uses=1]
  %arrayidx29 = getelementptr inbounds float addrspace(1)* getelementptr inbounds ([101 x float] addrspace(1)* @weights, i32 0, i32 0), i32 %tmp28 ; <float addrspace(1)*> [#uses=1]
  %tmp30 = load float addrspace(1)* %arrayidx29   ; <float> [#uses=1]
  %tmp31 = load float* %weightAccum               ; <float> [#uses=1]
  %add32 = fadd float %tmp31, %tmp30              ; <float> [#uses=1]
  store float %add32, float* %weightAccum
  %tmp33 = load float* %floatIndex                ; <float> [#uses=1]
  %inc = fadd float %tmp33, 1.000000e+000         ; <float> [#uses=1]
  store float %inc, float* %floatIndex
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp34 = load i32* %i                           ; <i32> [#uses=1]
  %inc35 = add nsw i32 %tmp34, 1                  ; <i32> [#uses=1]
  store i32 %inc35, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp36 = load float* %weightAccum               ; <float> [#uses=1]
  %tmp37 = insertelement <4 x float> undef, float %tmp36, i32 0 ; <<4 x float>> [#uses=2]
  %splat38 = shufflevector <4 x float> %tmp37, <4 x float> %tmp37, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=3]
  %tmp39 = load <4 x float>* %colAccum            ; <<4 x float>> [#uses=1]
  %cmp40 = fcmp oeq <4 x float> zeroinitializer, %splat38 ; <<4 x i1>> [#uses=1]
  %sel = select <4 x i1> %cmp40, <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float> %splat38 ; <<4 x float>> [#uses=0]
  %div = fdiv <4 x float> %tmp39, %splat38        ; <<4 x float>> [#uses=1]
  store <4 x float> %div, <4 x float>* %colAccum
  %tmp41 = load <4 x float>* %colAccum            ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp41, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t*)

declare <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2f(%struct._image2d_t*, i32, <2 x float>)

; CHECK: ret
define void @getWeightsSize(float %radius, float addrspace(1)* %wsize) nounwind {
entry:
  %radius.addr = alloca float, align 4            ; <float*> [#uses=2]
  %wsize.addr = alloca float addrspace(1)*, align 4 ; <float addrspace(1)**> [#uses=2]
  %tmp = alloca float, align 4                    ; <float*> [#uses=2]
  %location = alloca float, align 4               ; <float*> [#uses=2]
  %size = alloca float, align 4                   ; <float*> [#uses=2]
  store float %radius, float* %radius.addr
  store float addrspace(1)* %wsize, float addrspace(1)** %wsize.addr
  %call = call float @_Z5log1pf(float 0xBFED09C1C0000000) ; <float> [#uses=1]
  store float %call, float* %tmp
  %tmp2 = load float* %tmp                        ; <float> [#uses=1]
  %div = fdiv float %tmp2, 0xC0030624E0000000     ; <float> [#uses=1]
  %call3 = call float @_Z4sqrtf(float %div)       ; <float> [#uses=1]
  store float %call3, float* %location
  %tmp5 = load float* %location                   ; <float> [#uses=1]
  %tmp6 = load float* %radius.addr                ; <float> [#uses=1]
  %add = fadd float %tmp6, 1.000000e+000          ; <float> [#uses=1]
  %mul = fmul float %tmp5, %add                   ; <float> [#uses=1]
  store float %mul, float* %size
  %tmp7 = load float* %size                       ; <float> [#uses=1]
  %tmp8 = load float addrspace(1)** %wsize.addr   ; <float addrspace(1)*> [#uses=1]
  store float %tmp7, float addrspace(1)* %tmp8
  ret void
}

declare float @_Z5log1pf(float)

declare float @_Z4sqrtf(float)

; CHECK: ret
define void @evaluateDependents(i32 %direction_, float %radius_) nounwind {
entry:
  %direction_.addr = alloca i32, align 4          ; <i32*> [#uses=2]
  %radius_.addr = alloca float, align 4           ; <float*> [#uses=2]
  %i = alloca i32, align 4                        ; <i32*> [#uses=8]
  %val = alloca float, align 4                    ; <float*> [#uses=7]
  %location = alloca float, align 4               ; <float*> [#uses=3]
  %tmp17 = alloca float, align 4                  ; <float*> [#uses=2]
  %.compoundliteral = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %.compoundliteral40 = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  store i32 %direction_, i32* %direction_.addr
  store float %radius_, float* %radius_.addr
  %tmp = load i32* %direction_.addr               ; <i32> [#uses=1]
  store i32 %tmp, i32 addrspace(1)* @direction
  %tmp1 = load float* %radius_.addr               ; <float> [#uses=1]
  store float %tmp1, float addrspace(1)* @radius
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp3 = load i32* %i                            ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp3, 101                  ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp5 = load i32* %i                            ; <i32> [#uses=1]
  %conv = sitofp i32 %tmp5 to float               ; <float> [#uses=1]
  %tmp6 = load float* @centerIndex                ; <float> [#uses=1]
  %sub = fsub float %conv, %tmp6                  ; <float> [#uses=1]
  store float %sub, float* %val
  %tmp7 = load float* %val                        ; <float> [#uses=1]
  %cmp8 = fcmp olt float %tmp7, 0.000000e+000     ; <i1> [#uses=1]
  br i1 %cmp8, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %tmp10 = load float* %val                       ; <float> [#uses=1]
  %sub11 = fsub float 0.000000e+000, %tmp10       ; <float> [#uses=1]
  store float %sub11, float* %val
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %tmp13 = load float* %val                       ; <float> [#uses=1]
  %tmp14 = load float addrspace(1)* @radius       ; <float> [#uses=1]
  %add = fadd float %tmp14, 1.000000e+000         ; <float> [#uses=3]
  %cmp15 = fcmp oeq float 0.000000e+000, %add     ; <i1> [#uses=1]
  %sel = select i1 %cmp15, float 1.000000e+000, float %add ; <float> [#uses=0]
  %div = fdiv float %tmp13, %add                  ; <float> [#uses=1]
  store float %div, float* %location
  %tmp18 = load float* %location                  ; <float> [#uses=1]
  %mul = fmul float 0xC0030624E0000000, %tmp18    ; <float> [#uses=1]
  %tmp19 = load float* %location                  ; <float> [#uses=1]
  %mul20 = fmul float %mul, %tmp19                ; <float> [#uses=1]
  %call = call float @_Z3expf(float %mul20)       ; <float> [#uses=1]
  store float %call, float* %tmp17
  %tmp21 = load float* %tmp17                     ; <float> [#uses=1]
  %sub22 = fsub float 1.000000e+000, %tmp21       ; <float> [#uses=1]
  %mul23 = fmul float 0x3FF1A1CAC0000000, %sub22  ; <float> [#uses=1]
  %sub24 = fsub float 1.000000e+000, %mul23       ; <float> [#uses=1]
  store float %sub24, float* %val
  %tmp25 = load float* %val                       ; <float> [#uses=1]
  %call26 = call float @_Z3maxff(float %tmp25, float 0.000000e+000) ; <float> [#uses=1]
  %tmp27 = load i32* %i                           ; <i32> [#uses=1]
  %arrayidx = getelementptr inbounds float addrspace(1)* getelementptr inbounds ([101 x float] addrspace(1)* @weights, i32 0, i32 0), i32 %tmp27 ; <float addrspace(1)*> [#uses=1]
  store float %call26, float addrspace(1)* %arrayidx
  %tmp28 = load i32* %i                           ; <i32> [#uses=1]
  %arrayidx29 = getelementptr inbounds float addrspace(1)* getelementptr inbounds ([101 x float] addrspace(1)* @weights, i32 0, i32 0), i32 %tmp28 ; <float addrspace(1)*> [#uses=1]
  %tmp30 = load float addrspace(1)* %arrayidx29   ; <float> [#uses=1]
  %call31 = call float @_Z3minff(float %tmp30, float 1.000000e+000) ; <float> [#uses=1]
  %tmp32 = load i32* %i                           ; <i32> [#uses=1]
  %arrayidx33 = getelementptr inbounds float addrspace(1)* getelementptr inbounds ([101 x float] addrspace(1)* @weights, i32 0, i32 0), i32 %tmp32 ; <float addrspace(1)*> [#uses=1]
  store float %call31, float addrspace(1)* %arrayidx33
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %tmp34 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp34, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp35 = load i32 addrspace(1)* @direction      ; <i32> [#uses=1]
  %cmp36 = icmp eq i32 %tmp35, 0                  ; <i1> [#uses=1]
  br i1 %cmp36, label %if.then38, label %if.else

if.then38:                                        ; preds = %for.end
  store <2 x float> <float 1.000000e+000, float 0.000000e+000>, <2 x float>* %.compoundliteral
  %tmp39 = load <2 x float>* %.compoundliteral    ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp39, <2 x float> addrspace(1)* @blurDirection
  br label %if.end42

if.else:                                          ; preds = %for.end
  store <2 x float> <float 0.000000e+000, float 1.000000e+000>, <2 x float>* %.compoundliteral40
  %tmp41 = load <2 x float>* %.compoundliteral40  ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp41, <2 x float> addrspace(1)* @blurDirection
  br label %if.end42

if.end42:                                         ; preds = %if.else, %if.then38
  ret void
}

declare float @_Z3expf(float)

declare float @_Z3maxff(float, float)

declare float @_Z3minff(float, float)

; CHECK: ret
define void @gauss(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, i32 %rowCountPerGlobalID) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=3]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %rowCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %row = alloca i32, align 4                      ; <i32*> [#uses=7]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=4]
  %lastRow = alloca i32, align 4                  ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=3]
  %curCrd = alloca <2 x float>, align 8           ; <<2 x float>*> [#uses=5]
  %col = alloca i32, align 4                      ; <i32*> [#uses=5]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %rowCountPerGlobalID, i32* %rowCountPerGlobalID.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp = load i32* %rowCountPerGlobalID.addr      ; <i32> [#uses=1]
  %tmp1 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %row
  %tmp3 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %call4 = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %tmp3) ; <<2 x i32>> [#uses=1]
  %call5 = call <2 x i32> @_Z13convert_uint2U8__vector2i(<2 x i32> %call4) ; <<2 x i32>> [#uses=1]
  store <2 x i32> %call5, <2 x i32>* %imgSize
  %tmp7 = load i32* %row                          ; <i32> [#uses=1]
  %tmp8 = load i32* %rowCountPerGlobalID.addr     ; <i32> [#uses=1]
  %add = add i32 %tmp7, %tmp8                     ; <i32> [#uses=1]
  %tmp9 = load <2 x i32>* %imgSize                ; <<2 x i32>> [#uses=1]
  %tmp10 = extractelement <2 x i32> %tmp9, i32 1  ; <i32> [#uses=1]
  %call11 = call i32 @_Z3minjj(i32 %add, i32 %tmp10) ; <i32> [#uses=1]
  store i32 %call11, i32* %lastRow
  %tmp13 = load i32* %row                         ; <i32> [#uses=1]
  %tmp14 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp15 = extractelement <2 x i32> %tmp14, i32 0 ; <i32> [#uses=1]
  %mul16 = mul i32 %tmp13, %tmp15                 ; <i32> [#uses=1]
  store i32 %mul16, i32* %index
  br label %for.cond

for.cond:                                         ; preds = %for.inc42, %entry
  %tmp17 = load i32* %row                         ; <i32> [#uses=1]
  %tmp18 = load i32* %lastRow                     ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp17, %tmp18              ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end45

for.body:                                         ; preds = %for.cond
  %tmp20 = load i32* %row                         ; <i32> [#uses=1]
  %conv = uitofp i32 %tmp20 to float              ; <float> [#uses=1]
  %tmp21 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp22 = insertelement <2 x float> %tmp21, float %conv, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp22, <2 x float>* %curCrd
  store i32 0, i32* %col
  br label %for.cond24

for.cond24:                                       ; preds = %for.inc, %for.body
  %tmp25 = load i32* %col                         ; <i32> [#uses=1]
  %tmp26 = load <2 x i32>* %imgSize               ; <<2 x i32>> [#uses=1]
  %tmp27 = extractelement <2 x i32> %tmp26, i32 0 ; <i32> [#uses=1]
  %cmp28 = icmp ult i32 %tmp25, %tmp27            ; <i1> [#uses=1]
  br i1 %cmp28, label %for.body30, label %for.end

for.body30:                                       ; preds = %for.cond24
  %tmp31 = load i32* %col                         ; <i32> [#uses=1]
  %conv32 = uitofp i32 %tmp31 to float            ; <float> [#uses=1]
  %tmp33 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %tmp34 = insertelement <2 x float> %tmp33, float %conv32, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp34, <2 x float>* %curCrd
  %tmp35 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp36 = load <2 x float>* %curCrd              ; <<2 x float>> [#uses=1]
  %call37 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp35, <2 x float> %tmp36) ; <<4 x float>> [#uses=1]
  %tmp38 = load i32* %index                       ; <i32> [#uses=2]
  %inc = add i32 %tmp38, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  %tmp39 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp39, i32 %tmp38 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call37, <4 x float> addrspace(1)* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body30
  %tmp40 = load i32* %col                         ; <i32> [#uses=1]
  %inc41 = add i32 %tmp40, 1                      ; <i32> [#uses=1]
  store i32 %inc41, i32* %col
  br label %for.cond24

for.end:                                          ; preds = %for.cond24
  br label %for.inc42

for.inc42:                                        ; preds = %for.end
  %tmp43 = load i32* %row                         ; <i32> [#uses=1]
  %inc44 = add i32 %tmp43, 1                      ; <i32> [#uses=1]
  store i32 %inc44, i32* %row
  br label %for.cond

for.end45:                                        ; preds = %for.cond
  ret void
}

declare i32 @get_global_id(i32)

declare <2 x i32> @_Z13convert_uint2U8__vector2i(<2 x i32>)

declare i32 @_Z3minjj(i32, i32)
