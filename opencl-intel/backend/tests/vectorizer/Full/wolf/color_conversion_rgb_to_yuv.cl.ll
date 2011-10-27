; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\color_conversion_rgb_to_yuv.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_color_conversion_rgb_to_yuv_GPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_color_conversion_rgb_to_yuv_GPU_parameters = appending global [87 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[87 x i8]*> [#uses=1]
@opencl_color_conversion_rgb_to_yuv_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_color_conversion_rgb_to_yuv_parameters = appending global [99 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[99 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*)* @color_conversion_rgb_to_yuv_GPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_color_conversion_rgb_to_yuv_GPU_locals to i8*), i8* getelementptr inbounds ([87 x i8]* @opencl_color_conversion_rgb_to_yuv_GPU_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32)* @color_conversion_rgb_to_yuv to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_color_conversion_rgb_to_yuv_locals to i8*), i8* getelementptr inbounds ([99 x i8]* @opencl_color_conversion_rgb_to_yuv_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define <4 x float> @evaluatePixel(<4 x float> %pix) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %pix.addr = alloca <4 x float>, align 16        ; <<4 x float>*> [#uses=8]
  %output = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=10]
  store <4 x float> %pix, <4 x float>* %pix.addr
  %tmp = load <4 x float>* %pix.addr              ; <<4 x float>> [#uses=1]
  %tmp1 = extractelement <4 x float> %tmp, i32 0  ; <float> [#uses=1]
  %mul = fmul float 0x3FD322D0E0000000, %tmp1     ; <float> [#uses=1]
  %tmp2 = load <4 x float>* %pix.addr             ; <<4 x float>> [#uses=1]
  %tmp3 = extractelement <4 x float> %tmp2, i32 1 ; <float> [#uses=1]
  %mul4 = fmul float 0x3FE2C8B440000000, %tmp3    ; <float> [#uses=1]
  %add = fadd float %mul, %mul4                   ; <float> [#uses=1]
  %tmp5 = load <4 x float>* %pix.addr             ; <<4 x float>> [#uses=1]
  %tmp6 = extractelement <4 x float> %tmp5, i32 2 ; <float> [#uses=1]
  %mul7 = fmul float 0x3FBD2F1AA0000000, %tmp6    ; <float> [#uses=1]
  %add8 = fadd float %add, %mul7                  ; <float> [#uses=1]
  %tmp9 = load <4 x float>* %output               ; <<4 x float>> [#uses=1]
  %tmp10 = insertelement <4 x float> %tmp9, float %add8, i32 0 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp10, <4 x float>* %output
  %tmp11 = load <4 x float>* %pix.addr            ; <<4 x float>> [#uses=1]
  %tmp12 = extractelement <4 x float> %tmp11, i32 2 ; <float> [#uses=1]
  %tmp13 = load <4 x float>* %output              ; <<4 x float>> [#uses=1]
  %tmp14 = extractelement <4 x float> %tmp13, i32 0 ; <float> [#uses=1]
  %sub = fsub float %tmp12, %tmp14                ; <float> [#uses=1]
  %mul15 = fmul float 0x3FDF7CEDA0000000, %sub    ; <float> [#uses=1]
  %tmp16 = load <4 x float>* %output              ; <<4 x float>> [#uses=1]
  %tmp17 = insertelement <4 x float> %tmp16, float %mul15, i32 1 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp17, <4 x float>* %output
  %tmp18 = load <4 x float>* %pix.addr            ; <<4 x float>> [#uses=1]
  %tmp19 = extractelement <4 x float> %tmp18, i32 0 ; <float> [#uses=1]
  %tmp20 = load <4 x float>* %output              ; <<4 x float>> [#uses=1]
  %tmp21 = extractelement <4 x float> %tmp20, i32 0 ; <float> [#uses=1]
  %sub22 = fsub float %tmp19, %tmp21              ; <float> [#uses=1]
  %mul23 = fmul float 0x3FEC106240000000, %sub22  ; <float> [#uses=1]
  %tmp24 = load <4 x float>* %output              ; <<4 x float>> [#uses=1]
  %tmp25 = insertelement <4 x float> %tmp24, float %mul23, i32 2 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp25, <4 x float>* %output
  %tmp26 = load <4 x float>* %pix.addr            ; <<4 x float>> [#uses=1]
  %tmp27 = extractelement <4 x float> %tmp26, i32 3 ; <float> [#uses=1]
  %tmp28 = load <4 x float>* %output              ; <<4 x float>> [#uses=1]
  %tmp29 = insertelement <4 x float> %tmp28, float %tmp27, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp29, <4 x float>* %output
  %tmp30 = load <4 x float>* %pix.addr            ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp30, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

; CHECK: ret
define void @color_conversion_rgb_to_yuv_GPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %gid0_curPix = alloca i32, align 4              ; <i32*> [#uses=3]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid0_curPix
  %tmp = load i32* %gid0_curPix                   ; <i32> [#uses=1]
  %tmp1 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp1, i32 %tmp ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp2 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  %call3 = call <4 x float> @evaluatePixel(<4 x float> %tmp2) ; <<4 x float>> [#uses=1]
  %tmp4 = load i32* %gid0_curPix                  ; <i32> [#uses=1]
  %tmp5 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx6 = getelementptr inbounds <4 x float> addrspace(1)* %tmp5, i32 %tmp4 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call3, <4 x float> addrspace(1)* %arrayidx6
  ret void
}

declare i32 @get_global_id(i32)

; CHECK: ret
define void @color_conversion_rgb_to_yuv(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %pixelCount) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %pixelCount.addr = alloca i32, align 4          ; <i32*> [#uses=3]
  %global_size = alloca i32, align 4              ; <i32*> [#uses=3]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %count = alloca i32, align 4                    ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=5]
  %i = alloca i32, align 4                        ; <i32*> [#uses=4]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store i32 %pixelCount, i32* %pixelCount.addr
  %call = call i32 @get_global_size(i32 0)        ; <i32> [#uses=1]
  store i32 %call, i32* %global_size
  %call1 = call i32 @get_global_id(i32 0)         ; <i32> [#uses=1]
  store i32 %call1, i32* %global_id
  %tmp = load i32* %pixelCount.addr               ; <i32> [#uses=1]
  %tmp2 = load i32* %global_size                  ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp2                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp2         ; <i32> [#uses=1]
  %div = udiv i32 %tmp, %sel                      ; <i32> [#uses=1]
  store i32 %div, i32* %count
  %tmp4 = load i32* %pixelCount.addr              ; <i32> [#uses=1]
  %tmp5 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp4, %tmp5                     ; <i32> [#uses=1]
  %tmp6 = load i32* %global_size                  ; <i32> [#uses=2]
  %cmp7 = icmp eq i32 0, %tmp6                    ; <i1> [#uses=1]
  %sel8 = select i1 %cmp7, i32 1, i32 %tmp6       ; <i32> [#uses=1]
  %div9 = udiv i32 %mul, %sel8                    ; <i32> [#uses=1]
  store i32 %div9, i32* %index
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp11 = load i32* %i                           ; <i32> [#uses=1]
  %tmp12 = load i32* %count                       ; <i32> [#uses=1]
  %cmp13 = icmp ult i32 %tmp11, %tmp12            ; <i1> [#uses=1]
  br i1 %cmp13, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp14 = load i32* %index                       ; <i32> [#uses=1]
  %tmp15 = load <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp15, i32 %tmp14 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp16 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  %call17 = call <4 x float> @evaluatePixel(<4 x float> %tmp16) ; <<4 x float>> [#uses=1]
  %tmp18 = load i32* %index                       ; <i32> [#uses=1]
  %tmp19 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx20 = getelementptr inbounds <4 x float> addrspace(1)* %tmp19, i32 %tmp18 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call17, <4 x float> addrspace(1)* %arrayidx20
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp21 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp21, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  %tmp22 = load i32* %index                       ; <i32> [#uses=1]
  %inc23 = add i32 %tmp22, 1                      ; <i32> [#uses=1]
  store i32 %inc23, i32* %index
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare i32 @get_global_size(i32)
