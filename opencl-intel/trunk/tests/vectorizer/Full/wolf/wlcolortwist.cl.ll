; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlcolortwist.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct._image2d_t = type opaque

@opencl_wlcolortwistKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlcolortwistKernel_parameters = appending global [84 x i8] c"__rd image2d_t, __wr image2d_t, float __attribute__((address_space(1))), uint const\00", section "llvm.metadata" ; <[84 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, %struct._image2d_t*, float, i32)* @wlcolortwistKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlcolortwistKernel_locals to i8*), i8* getelementptr inbounds ([84 x i8]* @opencl_wlcolortwistKernel_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define <4 x float> @processPixel(%struct._image2d_t* %inputImage, <2 x i32> %curCord, float %scaling) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %curCord.addr = alloca <2 x i32>, align 8       ; <<2 x i32>*> [#uses=2]
  %scaling.addr = alloca float, align 4           ; <float*> [#uses=4]
  %samplerNearest = alloca i32, align 4           ; <i32*> [#uses=1]
  %outputColor = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=7]
  %processedColor = alloca <4 x float>, align 16  ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <2 x i32> %curCord, <2 x i32>* %curCord.addr
  store float %scaling, float* %scaling.addr
  store i32 1, i32* %samplerNearest
  %tmp = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp1 = load <2 x i32>* %curCord.addr           ; <<2 x i32>> [#uses=1]
  %call = call <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2i(%struct._image2d_t* %tmp, i32 1, <2 x i32> %tmp1) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %outputColor
  %tmp3 = load <4 x float>* %outputColor          ; <<4 x float>> [#uses=1]
  %tmp4 = extractelement <4 x float> %tmp3, i32 0 ; <float> [#uses=1]
  %tmp5 = load float* %scaling.addr               ; <float> [#uses=1]
  %mul = fmul float %tmp4, %tmp5                  ; <float> [#uses=1]
  %tmp6 = load <4 x float>* %outputColor          ; <<4 x float>> [#uses=1]
  %tmp7 = extractelement <4 x float> %tmp6, i32 1 ; <float> [#uses=1]
  %tmp8 = load float* %scaling.addr               ; <float> [#uses=1]
  %mul9 = fmul float %tmp7, %tmp8                 ; <float> [#uses=1]
  %add = fadd float %mul, %mul9                   ; <float> [#uses=1]
  %tmp10 = load <4 x float>* %outputColor         ; <<4 x float>> [#uses=1]
  %tmp11 = extractelement <4 x float> %tmp10, i32 2 ; <float> [#uses=1]
  %tmp12 = load float* %scaling.addr              ; <float> [#uses=1]
  %mul13 = fmul float %tmp11, %tmp12              ; <float> [#uses=1]
  %add14 = fadd float %add, %mul13                ; <float> [#uses=1]
  %conv = fpext float %add14 to double            ; <double> [#uses=1]
  %tmp15 = load <4 x float>* %outputColor         ; <<4 x float>> [#uses=1]
  %tmp16 = extractelement <4 x float> %tmp15, i32 1 ; <float> [#uses=1]
  %conv17 = fpext float %tmp16 to double          ; <double> [#uses=1]
  %tmp18 = load <4 x float>* %outputColor         ; <<4 x float>> [#uses=1]
  %tmp19 = extractelement <4 x float> %tmp18, i32 2 ; <float> [#uses=1]
  %conv20 = fpext float %tmp19 to double          ; <double> [#uses=1]
  %tmp21 = load <4 x float>* %outputColor         ; <<4 x float>> [#uses=1]
  %tmp22 = extractelement <4 x float> %tmp21, i32 3 ; <float> [#uses=1]
  %conv23 = fpext float %tmp22 to double          ; <double> [#uses=1]
  %call24 = call i32 (...)* @make_float4(double %conv, double %conv17, double %conv20, double %conv23) ; <i32> [#uses=1]
  %conv25 = sitofp i32 %call24 to float           ; <float> [#uses=1]
  %tmp26 = insertelement <4 x float> undef, float %conv25, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp26, <4 x float> %tmp26, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat, <4 x float>* %processedColor
  %tmp27 = load <4 x float>* %processedColor      ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp27, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2i(%struct._image2d_t*, i32, <2 x i32>)

declare i32 @make_float4(...)

; CHECK: ret
define void @wlcolortwistKernel(%struct._image2d_t* %inputImage, %struct._image2d_t* %outputImage, float %scaling, i32 %buffer_size) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %outputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %scaling.addr = alloca float, align 4           ; <float*> [#uses=2]
  %buffer_size.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %curCrd = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=3]
  %color = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store %struct._image2d_t* %outputImage, %struct._image2d_t** %outputImage.addr
  store float %scaling, float* %scaling.addr
  store i32 %buffer_size, i32* %buffer_size.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  %call2 = call i32 (...)* @make_int2(i32 %call, i32 %call1) ; <i32> [#uses=1]
  %tmp = insertelement <2 x i32> undef, i32 %call2, i32 0 ; <<2 x i32>> [#uses=2]
  %splat = shufflevector <2 x i32> %tmp, <2 x i32> %tmp, <2 x i32> zeroinitializer ; <<2 x i32>> [#uses=1]
  store <2 x i32> %splat, <2 x i32>* %curCrd
  %tmp4 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp5 = load <2 x i32>* %curCrd                 ; <<2 x i32>> [#uses=1]
  %tmp6 = load float* %scaling.addr               ; <float> [#uses=1]
  %call7 = call <4 x float> @processPixel(%struct._image2d_t* %tmp4, <2 x i32> %tmp5, float %tmp6) ; <<4 x float>> [#uses=1]
  store <4 x float> %call7, <4 x float>* %color
  %tmp8 = load %struct._image2d_t** %outputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp9 = load <2 x i32>* %curCrd                 ; <<2 x i32>> [#uses=1]
  %tmp10 = load <4 x float>* %color               ; <<4 x float>> [#uses=1]
  call void @_Z12write_imagefP10_image2d_tU8__vector2iU8__vector4f(%struct._image2d_t* %tmp8, <2 x i32> %tmp9, <4 x float> %tmp10)
  ret void
}

declare i32 @make_int2(...)

declare i32 @get_global_id(i32)

declare void @_Z12write_imagefP10_image2d_tU8__vector2iU8__vector4f(%struct._image2d_t*, <2 x i32>, <4 x float>)
