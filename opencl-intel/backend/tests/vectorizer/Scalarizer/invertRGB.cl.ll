; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../Full/runtime.bc -O3 -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -scalarize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\invertRGB.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_invertRGB_GPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_invertRGB_GPU_parameters = appending global [87 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[87 x i8]*> [#uses=1]
@opencl_invertRGB_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_invertRGB_parameters = appending global [99 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, uint const\00", section "llvm.metadata" ; <[99 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*)* @invertRGB_GPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_invertRGB_GPU_locals to i8*), i8* getelementptr inbounds ([87 x i8], [87 x i8]* @opencl_invertRGB_GPU_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32)* @invertRGB to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_invertRGB_locals to i8*), i8* getelementptr inbounds ([99 x i8], [99 x i8]* @opencl_invertRGB_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define <4 x float> @evaluatePixel(<4 x float> %pixel) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %pixel.addr = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=3]
  %one = alloca <4 x float>, align 16             ; <<4 x float>*> [#uses=2]
  %output = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=5]
  store <4 x float> %pixel, <4 x float>* %pixel.addr
  store <4 x float> <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, <4 x float>* %one
  %tmp = load <4 x float>, <4 x float>* %one                   ; <<4 x float>> [#uses=1]
  %tmp1 = shufflevector <4 x float> %tmp, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2> ; <<3 x float>> [#uses=1]
  %tmp2 = load <4 x float>, <4 x float>* %pixel.addr           ; <<4 x float>> [#uses=1]
  %tmp3 = shufflevector <4 x float> %tmp2, <4 x float> undef, <3 x i32> <i32 0, i32 1, i32 2> ; <<3 x float>> [#uses=1]
  %sub = fsub <3 x float> %tmp1, %tmp3            ; <<3 x float>> [#uses=1]
  %tmp4 = load <4 x float>, <4 x float>* %output               ; <<4 x float>> [#uses=1]
  %tmp5 = shufflevector <3 x float> %sub, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef> ; <<4 x float>> [#uses=1]
  %tmp6 = shufflevector <4 x float> %tmp4, <4 x float> %tmp5, <4 x i32> <i32 4, i32 5, i32 6, i32 3> ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp6, <4 x float>* %output
  %tmp7 = load <4 x float>, <4 x float>* %pixel.addr           ; <<4 x float>> [#uses=1]
  %tmp8 = extractelement <4 x float> %tmp7, i32 3 ; <float> [#uses=1]
  %tmp9 = load <4 x float>, <4 x float>* %output               ; <<4 x float>> [#uses=1]
  %tmp10 = insertelement <4 x float> %tmp9, float %tmp8, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp10, <4 x float>* %output
  %tmp11 = load <4 x float>, <4 x float>* %output              ; <<4 x float>> [#uses=1]
  store <4 x float> %tmp11, <4 x float>* %retval
  %0 = load <4 x float>, <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

; CHECK: ret
define void @invertRGB_GPU(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output) nounwind {
entry:
  %input.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %gid0_curPixel = alloca i32, align 4            ; <i32*> [#uses=3]
  store <4 x float> addrspace(1)* %input, <4 x float> addrspace(1)** %input.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  %call = call i32 @_Z13get_global_idj(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid0_curPixel
  %tmp = load i32, i32* %gid0_curPixel                 ; <i32> [#uses=1]
  %tmp1 = load <4 x float> addrspace(1)*, <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float>, <4 x float> addrspace(1)* %tmp1, i32 %tmp ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp2 = load <4 x float>, <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  %call3 = call <4 x float> @evaluatePixel(<4 x float> %tmp2) ; <<4 x float>> [#uses=1]
  %tmp4 = load i32, i32* %gid0_curPixel                ; <i32> [#uses=1]
  %tmp5 = load <4 x float> addrspace(1)*, <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx6 = getelementptr inbounds <4 x float>, <4 x float> addrspace(1)* %tmp5, i32 %tmp4 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call3, <4 x float> addrspace(1)* %arrayidx6
  ret void
}

declare i32 @_Z13get_global_idj(i32)

; CHECK: ret
define void @invertRGB(<4 x float> addrspace(1)* %input, <4 x float> addrspace(1)* %output, i32 %pixelCount) nounwind {
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
  %call1 = call i32 @_Z13get_global_idj(i32 0)         ; <i32> [#uses=1]
  store i32 %call1, i32* %global_id
  %tmp = load i32, i32* %pixelCount.addr               ; <i32> [#uses=1]
  %tmp2 = load i32, i32* %global_size                  ; <i32> [#uses=2]
  %cmp = icmp eq i32 0, %tmp2                     ; <i1> [#uses=1]
  %sel = select i1 %cmp, i32 1, i32 %tmp2         ; <i32> [#uses=1]
  %div = udiv i32 %tmp, %sel                      ; <i32> [#uses=1]
  store i32 %div, i32* %count
  %tmp4 = load i32, i32* %pixelCount.addr              ; <i32> [#uses=1]
  %tmp5 = load i32, i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp4, %tmp5                     ; <i32> [#uses=1]
  %tmp6 = load i32, i32* %global_size                  ; <i32> [#uses=2]
  %cmp7 = icmp eq i32 0, %tmp6                    ; <i1> [#uses=1]
  %sel8 = select i1 %cmp7, i32 1, i32 %tmp6       ; <i32> [#uses=1]
  %div9 = udiv i32 %mul, %sel8                    ; <i32> [#uses=1]
  store i32 %div9, i32* %index
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp11 = load i32, i32* %i                           ; <i32> [#uses=1]
  %tmp12 = load i32, i32* %count                       ; <i32> [#uses=1]
  %cmp13 = icmp ult i32 %tmp11, %tmp12            ; <i1> [#uses=1]
  br i1 %cmp13, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp14 = load i32, i32* %index                       ; <i32> [#uses=1]
  %tmp15 = load <4 x float> addrspace(1)*, <4 x float> addrspace(1)** %input.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float>, <4 x float> addrspace(1)* %tmp15, i32 %tmp14 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp16 = load <4 x float>, <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  %call17 = call <4 x float> @evaluatePixel(<4 x float> %tmp16) ; <<4 x float>> [#uses=1]
  %tmp18 = load i32, i32* %index                       ; <i32> [#uses=1]
  %tmp19 = load <4 x float> addrspace(1)*, <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx20 = getelementptr inbounds <4 x float>, <4 x float> addrspace(1)* %tmp19, i32 %tmp18 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call17, <4 x float> addrspace(1)* %arrayidx20
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp21 = load i32, i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp21, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  %tmp22 = load i32, i32* %index                       ; <i32> [#uses=1]
  %inc23 = add i32 %tmp22, 1                      ; <i32> [#uses=1]
  store i32 %inc23, i32* %index
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}

declare i32 @get_global_size(i32)
