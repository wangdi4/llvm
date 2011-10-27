; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\twirl.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct._image2d_t = type opaque

@opencl_twirl2D_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_twirl2D_parameters = appending global [123 x i8] c"__rd image2d_t, float4 __attribute__((address_space(1))) *, int2 const, float const, float2 const, uint const, float const\00", section "llvm.metadata" ; <[123 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, <4 x float> addrspace(1)*, <2 x i32>, float, <2 x float>, i32, float)* @twirl2D to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_twirl2D_locals to i8*), i8* getelementptr inbounds ([123 x i8]* @opencl_twirl2D_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define <4 x float> @evaluatePixel(%struct._image2d_t* %inputImage, <2 x float> %outCrd, float %radius, <2 x float> %center, i32 %gaussOrSinc, float %twirlAngle) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %outCrd.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=2]
  %radius.addr = alloca float, align 4            ; <float*> [#uses=2]
  %center.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=3]
  %gaussOrSinc.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %twirlAngle.addr = alloca float, align 4        ; <float*> [#uses=2]
  %pixelSize = alloca <2 x float>, align 8        ; <<2 x float>*> [#uses=2]
  %.compoundliteral = alloca <2 x float>, align 8 ; <<2 x float>*> [#uses=2]
  %twirlAngleRadians = alloca float, align 4      ; <float*> [#uses=4]
  %adjustedRadius = alloca float, align 4         ; <float*> [#uses=2]
  %relativePos = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=11]
  %distFromCenter = alloca float, align 4         ; <float*> [#uses=8]
  %adjustedRadians = alloca float, align 4        ; <float*> [#uses=5]
  %sincWeight = alloca float, align 4             ; <float*> [#uses=2]
  %gaussWeight = alloca float, align 4            ; <float*> [#uses=2]
  %cosAngle = alloca float, align 4               ; <float*> [#uses=3]
  %sinAngle = alloca float, align 4               ; <float*> [#uses=3]
  %cosx_minus_siny = alloca float, align 4        ; <float*> [#uses=2]
  %sinx_plus_cosy = alloca float, align 4         ; <float*> [#uses=2]
  %samplerLinear = alloca i32, align 4            ; <i32*> [#uses=1]
  %outputColor = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <2 x float> %outCrd, <2 x float>* %outCrd.addr
  store float %radius, float* %radius.addr
  store <2 x float> %center, <2 x float>* %center.addr
  store i32 %gaussOrSinc, i32* %gaussOrSinc.addr
  store float %twirlAngle, float* %twirlAngle.addr
  store <2 x float> <float 1.000000e+000, float 1.000000e+000>, <2 x float>* %.compoundliteral
  %tmp = load <2 x float>* %.compoundliteral      ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp, <2 x float>* %pixelSize
  %tmp2 = load float* %twirlAngle.addr            ; <float> [#uses=1]
  %call = call float @_Z7radiansf(float %tmp2)    ; <float> [#uses=1]
  store float %call, float* %twirlAngleRadians
  %tmp4 = load float* %radius.addr                ; <float> [#uses=1]
  %tmp5 = insertelement <2 x float> undef, float %tmp4, i32 0 ; <<2 x float>> [#uses=2]
  %splat = shufflevector <2 x float> %tmp5, <2 x float> %tmp5, <2 x i32> zeroinitializer ; <<2 x float>> [#uses=1]
  %tmp6 = load <2 x float>* %pixelSize            ; <<2 x float>> [#uses=1]
  %mul = fmul <2 x float> %splat, %tmp6           ; <<2 x float>> [#uses=1]
  %call7 = call float @_Z6lengthU8__vector2f(<2 x float> %mul) ; <float> [#uses=1]
  store float %call7, float* %adjustedRadius
  %tmp9 = load <2 x float>* %outCrd.addr          ; <<2 x float>> [#uses=1]
  %tmp10 = load <2 x float>* %center.addr         ; <<2 x float>> [#uses=1]
  %sub = fsub <2 x float> %tmp9, %tmp10           ; <<2 x float>> [#uses=1]
  store <2 x float> %sub, <2 x float>* %relativePos
  %tmp12 = load <2 x float>* %relativePos         ; <<2 x float>> [#uses=1]
  %call13 = call float @_Z6lengthU8__vector2f(<2 x float> %tmp12) ; <float> [#uses=1]
  store float %call13, float* %distFromCenter
  %tmp14 = load float* %adjustedRadius            ; <float> [#uses=3]
  %tmp15 = load float* %distFromCenter            ; <float> [#uses=1]
  %cmp = fcmp oeq float 0.000000e+000, %tmp14     ; <i1> [#uses=1]
  %sel = select i1 %cmp, float 1.000000e+000, float %tmp14 ; <float> [#uses=0]
  %div = fdiv float %tmp15, %tmp14                ; <float> [#uses=1]
  store float %div, float* %distFromCenter
  %tmp18 = load float* %distFromCenter            ; <float> [#uses=1]
  %call19 = call float @_Z3sinf(float %tmp18)     ; <float> [#uses=1]
  %tmp20 = load float* %twirlAngleRadians         ; <float> [#uses=1]
  %mul21 = fmul float %call19, %tmp20             ; <float> [#uses=1]
  %tmp22 = load float* %distFromCenter            ; <float> [#uses=3]
  %cmp23 = fcmp oeq float 0.000000e+000, %tmp22   ; <i1> [#uses=1]
  %sel24 = select i1 %cmp23, float 1.000000e+000, float %tmp22 ; <float> [#uses=0]
  %div25 = fdiv float %mul21, %tmp22              ; <float> [#uses=1]
  store float %div25, float* %sincWeight
  %tmp27 = load float* %distFromCenter            ; <float> [#uses=1]
  %mul28 = fmul float -1.000000e+000, %tmp27      ; <float> [#uses=1]
  %tmp29 = load float* %distFromCenter            ; <float> [#uses=1]
  %mul30 = fmul float %mul28, %tmp29              ; <float> [#uses=1]
  %call31 = call float @_Z3expf(float %mul30)     ; <float> [#uses=1]
  %tmp32 = load float* %twirlAngleRadians         ; <float> [#uses=1]
  %mul33 = fmul float %call31, %tmp32             ; <float> [#uses=1]
  store float %mul33, float* %gaussWeight
  %tmp34 = load float* %distFromCenter            ; <float> [#uses=1]
  %cmp35 = fcmp oeq float %tmp34, 0.000000e+000   ; <i1> [#uses=1]
  %tmp36 = load float* %twirlAngleRadians         ; <float> [#uses=1]
  %tmp37 = load float* %sincWeight                ; <float> [#uses=1]
  %cond = select i1 %cmp35, float %tmp36, float %tmp37 ; <float> [#uses=1]
  store float %cond, float* %adjustedRadians
  %tmp38 = load i32* %gaussOrSinc.addr            ; <i32> [#uses=1]
  %cmp39 = icmp eq i32 %tmp38, 1                  ; <i1> [#uses=1]
  %tmp40 = load float* %adjustedRadians           ; <float> [#uses=1]
  %tmp41 = load float* %gaussWeight               ; <float> [#uses=1]
  %cond42 = select i1 %cmp39, float %tmp40, float %tmp41 ; <float> [#uses=1]
  store float %cond42, float* %adjustedRadians
  %tmp44 = load float* %adjustedRadians           ; <float> [#uses=1]
  %call45 = call float @_Z3cosf(float %tmp44)     ; <float> [#uses=1]
  store float %call45, float* %cosAngle
  %tmp47 = load float* %adjustedRadians           ; <float> [#uses=1]
  %call48 = call float @_Z3sinf(float %tmp47)     ; <float> [#uses=1]
  store float %call48, float* %sinAngle
  %tmp50 = load float* %cosAngle                  ; <float> [#uses=1]
  %tmp51 = load <2 x float>* %relativePos         ; <<2 x float>> [#uses=1]
  %tmp52 = extractelement <2 x float> %tmp51, i32 0 ; <float> [#uses=1]
  %mul53 = fmul float %tmp50, %tmp52              ; <float> [#uses=1]
  %tmp54 = load float* %sinAngle                  ; <float> [#uses=1]
  %tmp55 = load <2 x float>* %relativePos         ; <<2 x float>> [#uses=1]
  %tmp56 = extractelement <2 x float> %tmp55, i32 1 ; <float> [#uses=1]
  %mul57 = fmul float %tmp54, %tmp56              ; <float> [#uses=1]
  %sub58 = fsub float %mul53, %mul57              ; <float> [#uses=1]
  store float %sub58, float* %cosx_minus_siny
  %tmp60 = load float* %sinAngle                  ; <float> [#uses=1]
  %tmp61 = load <2 x float>* %relativePos         ; <<2 x float>> [#uses=1]
  %tmp62 = extractelement <2 x float> %tmp61, i32 0 ; <float> [#uses=1]
  %mul63 = fmul float %tmp60, %tmp62              ; <float> [#uses=1]
  %tmp64 = load float* %cosAngle                  ; <float> [#uses=1]
  %tmp65 = load <2 x float>* %relativePos         ; <<2 x float>> [#uses=1]
  %tmp66 = extractelement <2 x float> %tmp65, i32 1 ; <float> [#uses=1]
  %mul67 = fmul float %tmp64, %tmp66              ; <float> [#uses=1]
  %add = fadd float %mul63, %mul67                ; <float> [#uses=1]
  store float %add, float* %sinx_plus_cosy
  %tmp68 = load float* %cosx_minus_siny           ; <float> [#uses=1]
  %tmp69 = load <2 x float>* %relativePos         ; <<2 x float>> [#uses=1]
  %tmp70 = insertelement <2 x float> %tmp69, float %tmp68, i32 0 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp70, <2 x float>* %relativePos
  %tmp71 = load float* %sinx_plus_cosy            ; <float> [#uses=1]
  %tmp72 = load <2 x float>* %relativePos         ; <<2 x float>> [#uses=1]
  %tmp73 = insertelement <2 x float> %tmp72, float %tmp71, i32 1 ; <<2 x float>> [#uses=1]
  store <2 x float> %tmp73, <2 x float>* %relativePos
  store i32 17, i32* %samplerLinear
  %tmp76 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp77 = load <2 x float>* %relativePos         ; <<2 x float>> [#uses=1]
  %tmp78 = load <2 x float>* %center.addr         ; <<2 x float>> [#uses=1]
  %add79 = fadd <2 x float> %tmp77, %tmp78        ; <<2 x float>> [#uses=1]
  %call80 = call <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2f(%struct._image2d_t* %tmp76, i32 17, <2 x float> %add79) ; <<4 x float>> [#uses=1]
  store <4 x float> %call80, <4 x float>* %outputColor
  %tmp81 = load <4 x float>* %outputColor         ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp81, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare float @_Z7radiansf(float)

declare float @_Z6lengthU8__vector2f(<2 x float>)

declare float @_Z3sinf(float)

declare float @_Z3expf(float)

declare float @_Z3cosf(float)

declare <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2f(%struct._image2d_t*, i32, <2 x float>)
; CHECK: ret
define void @twirl2D(%struct._image2d_t* %inputImage, <4 x float> addrspace(1)* %output, <2 x i32> %dstSize, float %radius, <2 x float> %center, i32 %gaussOrSinc, float %twirlAngle) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=3]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %dstSize.addr = alloca <2 x i32>, align 8       ; <<2 x i32>*> [#uses=5]
  %radius.addr = alloca float, align 4            ; <float*> [#uses=2]
  %center.addr = alloca <2 x float>, align 8      ; <<2 x float>*> [#uses=2]
  %gaussOrSinc.addr = alloca i32, align 4         ; <i32*> [#uses=2]
  %twirlAngle.addr = alloca float, align 4        ; <float*> [#uses=2]
  %gid0_col = alloca i32, align 4                 ; <i32*> [#uses=4]
  %gid1_row = alloca i32, align 4                 ; <i32*> [#uses=4]
  %imgSize = alloca <2 x i32>, align 8            ; <<2 x i32>*> [#uses=2]
  %offset = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=2]
  %curCrd = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store <2 x i32> %dstSize, <2 x i32>* %dstSize.addr
  store float %radius, float* %radius.addr
  store <2 x float> %center, <2 x float>* %center.addr
  store i32 %gaussOrSinc, i32* %gaussOrSinc.addr
  store float %twirlAngle, float* %twirlAngle.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid0_col
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  store i32 %call1, i32* %gid1_row
  %tmp = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %call2 = call <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t* %tmp) ; <<2 x i32>> [#uses=1]
  store <2 x i32> %call2, <2 x i32>* %imgSize
  %tmp4 = load <2 x i32>* %dstSize.addr           ; <<2 x i32>> [#uses=1]
  %tmp5 = load <2 x i32>* %imgSize                ; <<2 x i32>> [#uses=1]
  %sub = sub <2 x i32> %tmp4, %tmp5               ; <<2 x i32>> [#uses=1]
  %div = sdiv <2 x i32> %sub, <i32 1, i32 1> ; <<2 x i32>> [#uses=1]
  store <2 x i32> %div, <2 x i32>* %offset
  %tmp6 = load i32* %gid0_col                     ; <i32> [#uses=1]
  %tmp7 = load <2 x i32>* %dstSize.addr           ; <<2 x i32>> [#uses=1]
  %tmp8 = extractelement <2 x i32> %tmp7, i32 0   ; <i32> [#uses=1]
  %cmp = icmp slt i32 %tmp6, %tmp8                ; <i1> [#uses=1]
  br i1 %cmp, label %land.lhs.true, label %if.end

land.lhs.true:                                    ; preds = %entry
  %tmp9 = load i32* %gid1_row                     ; <i32> [#uses=1]
  %tmp10 = load <2 x i32>* %dstSize.addr          ; <<2 x i32>> [#uses=1]
  %tmp11 = extractelement <2 x i32> %tmp10, i32 1 ; <i32> [#uses=1]
  %cmp12 = icmp slt i32 %tmp9, %tmp11             ; <i1> [#uses=1]
  br i1 %cmp12, label %if.then, label %if.end

if.then:                                          ; preds = %land.lhs.true
  %tmp14 = load i32* %gid1_row                    ; <i32> [#uses=1]
  %tmp15 = load <2 x i32>* %dstSize.addr          ; <<2 x i32>> [#uses=1]
  %tmp16 = extractelement <2 x i32> %tmp15, i32 0 ; <i32> [#uses=1]
  %mul = mul i32 %tmp14, %tmp16                   ; <i32> [#uses=1]
  %tmp17 = load i32* %gid0_col                    ; <i32> [#uses=1]
  %add = add nsw i32 %mul, %tmp17                 ; <i32> [#uses=1]
  store i32 %add, i32* %index
  %tmp19 = load i32* %gid0_col                    ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %tmp19, i32 0 ; <<2 x i32>> [#uses=1]
  %tmp20 = load i32* %gid1_row                    ; <i32> [#uses=1]
  %vecinit21 = insertelement <2 x i32> %vecinit, i32 %tmp20, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit21, <2 x i32>* %curCrd
  %tmp22 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp23 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp24 = load <2 x i32>* %offset                ; <<2 x i32>> [#uses=1]
  %sub25 = sub <2 x i32> %tmp23, %tmp24           ; <<2 x i32>> [#uses=1]
  %call26 = call <2 x float> @_Z14convert_float2U8__vector2i(<2 x i32> %sub25) ; <<2 x float>> [#uses=1]
  %tmp27 = load float* %radius.addr               ; <float> [#uses=1]
  %tmp28 = load <2 x float>* %center.addr         ; <<2 x float>> [#uses=1]
  %tmp29 = load i32* %gaussOrSinc.addr            ; <i32> [#uses=1]
  %tmp30 = load float* %twirlAngle.addr           ; <float> [#uses=1]
  %call31 = call <4 x float> @evaluatePixel(%struct._image2d_t* %tmp22, <2 x float> %call26, float %tmp27, <2 x float> %tmp28, i32 %tmp29, float %tmp30) ; <<4 x float>> [#uses=1]
  %tmp32 = load i32* %index                       ; <i32> [#uses=1]
  %tmp33 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp33, i32 %tmp32 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call31, <4 x float> addrspace(1)* %arrayidx
  br label %if.end

if.end:                                           ; preds = %if.then, %land.lhs.true, %entry
  ret void
}

declare i32 @get_global_id(i32)

declare <2 x i32> @_Z13get_image_dimP10_image2d_t(%struct._image2d_t*)

declare <2 x float> @_Z14convert_float2U8__vector2i(<2 x i32>)
