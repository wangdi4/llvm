; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\wlAdobe_PB_Pixelate.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>
%struct._image2d_t = type opaque

@opencl_wlAdobe_PB_PixelateKernel_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_wlAdobe_PB_PixelateKernel_parameters = appending global [56 x i8] c"__rd image2d_t, __wr image2d_t, float const, uint const\00", section "llvm.metadata" ; <[56 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (%struct._image2d_t*, %struct._image2d_t*, float, i32)* @wlAdobe_PB_PixelateKernel to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_wlAdobe_PB_PixelateKernel_locals to i8*), i8* getelementptr inbounds ([56 x i8]* @opencl_wlAdobe_PB_PixelateKernel_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define <4 x float> @processPixel(%struct._image2d_t* %inputImage, <2 x float> %curCord) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %curCord.addr = alloca <2 x float>, align 8     ; <<2 x float>*> [#uses=2]
  %samplerNearest = alloca i32, align 4           ; <i32*> [#uses=1]
  %outputColor = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store <2 x float> %curCord, <2 x float>* %curCord.addr
  store i32 1, i32* %samplerNearest
  %tmp = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp1 = load <2 x float>* %curCord.addr         ; <<2 x float>> [#uses=1]
  %call = call <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2f(%struct._image2d_t* %tmp, i32 1, <2 x float> %tmp1) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %outputColor
  %tmp2 = load <4 x float>* %outputColor          ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp2, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z11read_imagefP10_image2d_tjU8__vector2f(%struct._image2d_t*, i32, <2 x float>)

; CHECK: ret
define void @wlAdobe_PB_PixelateKernel(%struct._image2d_t* %inputImage, %struct._image2d_t* %outputImage, float %passed_dimension, i32 %buffer_size) nounwind {
entry:
  %inputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %outputImage.addr = alloca %struct._image2d_t*, align 4 ; <%struct._image2d_t**> [#uses=2]
  %passed_dimension.addr = alloca float, align 4  ; <float*> [#uses=2]
  %buffer_size.addr = alloca i32, align 4         ; <i32*> [#uses=1]
  %curCrd = alloca <2 x i32>, align 8             ; <<2 x i32>*> [#uses=3]
  %dimension = alloca <2 x float>, align 8        ; <<2 x float>*> [#uses=3]
  %coord = alloca <2 x float>, align 8            ; <<2 x float>*> [#uses=2]
  %sc = alloca <2 x float>, align 8               ; <<2 x float>*> [#uses=4]
  %color = alloca <4 x float>, align 16           ; <<4 x float>*> [#uses=2]
  store %struct._image2d_t* %inputImage, %struct._image2d_t** %inputImage.addr
  store %struct._image2d_t* %outputImage, %struct._image2d_t** %outputImage.addr
  store float %passed_dimension, float* %passed_dimension.addr
  store i32 %buffer_size, i32* %buffer_size.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  %vecinit = insertelement <2 x i32> undef, i32 %call, i32 0 ; <<2 x i32>> [#uses=1]
  %call1 = call i32 @get_global_id(i32 1)         ; <i32> [#uses=1]
  %vecinit2 = insertelement <2 x i32> %vecinit, i32 %call1, i32 1 ; <<2 x i32>> [#uses=1]
  store <2 x i32> %vecinit2, <2 x i32>* %curCrd
  %tmp = load float* %passed_dimension.addr       ; <float> [#uses=1]
  %tmp3 = insertelement <2 x float> undef, float %tmp, i32 0 ; <<2 x float>> [#uses=2]
  %splat = shufflevector <2 x float> %tmp3, <2 x float> %tmp3, <2 x i32> zeroinitializer ; <<2 x float>> [#uses=1]
  store <2 x float> %splat, <2 x float>* %dimension
  %tmp5 = load <2 x i32>* %curCrd                 ; <<2 x i32>> [#uses=1]
  %call6 = call <2 x float> @_Z14convert_float2U8__vector2i(<2 x i32> %tmp5) ; <<2 x float>> [#uses=1]
  store <2 x float> %call6, <2 x float>* %coord
  %tmp8 = load <2 x float>* %coord                ; <<2 x float>> [#uses=1]
  %tmp9 = load <2 x float>* %dimension            ; <<2 x float>> [#uses=3]
  %cmp = fcmp oeq <2 x float> zeroinitializer, %tmp9 ; <<2 x i1>> [#uses=1]
  %sel = select <2 x i1> %cmp, <2 x float> <float 1.000000e+000, float 1.000000e+000>, <2 x float> %tmp9 ; <<2 x float>> [#uses=0]
  %div = fdiv <2 x float> %tmp8, %tmp9            ; <<2 x float>> [#uses=1]
  %call10 = call <2 x float> @_Z5floorU8__vector2f(<2 x float> %div) ; <<2 x float>> [#uses=1]
  store <2 x float> %call10, <2 x float>* %sc
  %tmp11 = load <2 x float>* %dimension           ; <<2 x float>> [#uses=1]
  %tmp12 = load <2 x float>* %sc                  ; <<2 x float>> [#uses=1]
  %mul = fmul <2 x float> %tmp12, %tmp11          ; <<2 x float>> [#uses=1]
  store <2 x float> %mul, <2 x float>* %sc
  %tmp14 = load %struct._image2d_t** %inputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp15 = load <2 x float>* %sc                  ; <<2 x float>> [#uses=1]
  %call16 = call <4 x float> @processPixel(%struct._image2d_t* %tmp14, <2 x float> %tmp15) ; <<4 x float>> [#uses=1]
  store <4 x float> %call16, <4 x float>* %color
  %tmp17 = load %struct._image2d_t** %outputImage.addr ; <%struct._image2d_t*> [#uses=1]
  %tmp18 = load <2 x i32>* %curCrd                ; <<2 x i32>> [#uses=1]
  %tmp19 = load <4 x float>* %color               ; <<4 x float>> [#uses=1]
  call void @_Z12write_imagefP10_image2d_tU8__vector2iU8__vector4f(%struct._image2d_t* %tmp17, <2 x i32> %tmp18, <4 x float> %tmp19)
  ret void
}

declare i32 @get_global_id(i32)

declare <2 x float> @_Z14convert_float2U8__vector2i(<2 x i32>)

declare <2 x float> @_Z5floorU8__vector2f(<2 x float>)

declare void @_Z12write_imagefP10_image2d_tU8__vector2iU8__vector4f(%struct._image2d_t*, <2 x i32>, <4 x float>)
