; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = '.\cl_files\crossfade.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_crossfade_GPU_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_crossfade_GPU_parameters = appending global [144 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float const\00", section "llvm.metadata" ; <[144 x i8]*> [#uses=1]
@opencl_crossfade_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_crossfade_parameters = appending global [156 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float const, uint const\00", section "llvm.metadata" ; <[156 x i8]*> [#uses=1]
@opencl_metadata = appending global [2 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, float)* @crossfade_GPU to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_crossfade_GPU_locals to i8*), i8* getelementptr inbounds ([144 x i8]* @opencl_crossfade_GPU_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, float, i32)* @crossfade to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_crossfade_locals to i8*), i8* getelementptr inbounds ([156 x i8]* @opencl_crossfade_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[2 x %opencl_metadata_type]*> [#uses=0]

; CHECK: ret
define <4 x float> @evaluatePixel(<4 x float> %fColor, <4 x float> %bColor, float %intensity) nounwind {
entry:
  %retval = alloca <4 x float>, align 16          ; <<4 x float>*> [#uses=2]
  %fColor.addr = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %bColor.addr = alloca <4 x float>, align 16     ; <<4 x float>*> [#uses=2]
  %intensity.addr = alloca float, align 4         ; <float*> [#uses=2]
  %vIntensity = alloca <4 x float>, align 16      ; <<4 x float>*> [#uses=2]
  store <4 x float> %fColor, <4 x float>* %fColor.addr
  store <4 x float> %bColor, <4 x float>* %bColor.addr
  store float %intensity, float* %intensity.addr
  %tmp = load float* %intensity.addr              ; <float> [#uses=1]
  %tmp1 = insertelement <4 x float> undef, float %tmp, i32 0 ; <<4 x float>> [#uses=2]
  %splat = shufflevector <4 x float> %tmp1, <4 x float> %tmp1, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  store <4 x float> %splat, <4 x float>* %vIntensity
  %tmp2 = load <4 x float>* %fColor.addr          ; <<4 x float>> [#uses=1]
  %tmp3 = load <4 x float>* %bColor.addr          ; <<4 x float>> [#uses=1]
  %tmp4 = load <4 x float>* %vIntensity           ; <<4 x float>> [#uses=1]
  %call = call <4 x float> @_Z3mixU8__vector4fS_S_(<4 x float> %tmp2, <4 x float> %tmp3, <4 x float> %tmp4) ; <<4 x float>> [#uses=1]
  store <4 x float> %call, <4 x float>* %retval
  %0 = load <4 x float>* %retval                  ; <<4 x float>> [#uses=1]
  ret <4 x float> %0
}

declare <4 x float> @_Z3mixU8__vector4fS_S_(<4 x float>, <4 x float>, <4 x float>)

; CHECK: ret
define void @crossfade_GPU(<4 x float> addrspace(1)* %front, <4 x float> addrspace(1)* %back, <4 x float> addrspace(1)* %output, float %intensity) nounwind {
entry:
  %front.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %back.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %intensity.addr = alloca float, align 4         ; <float*> [#uses=2]
  %gid0_curPix = alloca i32, align 4              ; <i32*> [#uses=4]
  store <4 x float> addrspace(1)* %front, <4 x float> addrspace(1)** %front.addr
  store <4 x float> addrspace(1)* %back, <4 x float> addrspace(1)** %back.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store float %intensity, float* %intensity.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %gid0_curPix
  %tmp = load i32* %gid0_curPix                   ; <i32> [#uses=1]
  %tmp1 = load <4 x float> addrspace(1)** %front.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp1, i32 %tmp ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp2 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  %tmp3 = load i32* %gid0_curPix                  ; <i32> [#uses=1]
  %tmp4 = load <4 x float> addrspace(1)** %back.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx5 = getelementptr inbounds <4 x float> addrspace(1)* %tmp4, i32 %tmp3 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp6 = load <4 x float> addrspace(1)* %arrayidx5 ; <<4 x float>> [#uses=1]
  %tmp7 = load float* %intensity.addr             ; <float> [#uses=1]
  %call8 = call <4 x float> @evaluatePixel(<4 x float> %tmp2, <4 x float> %tmp6, float %tmp7) ; <<4 x float>> [#uses=1]
  %tmp9 = load i32* %gid0_curPix                  ; <i32> [#uses=1]
  %tmp10 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx11 = getelementptr inbounds <4 x float> addrspace(1)* %tmp10, i32 %tmp9 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call8, <4 x float> addrspace(1)* %arrayidx11
  ret void
}

declare i32 @get_global_id(i32)

; CHECK: ret
define void @crossfade(<4 x float> addrspace(1)* %front, <4 x float> addrspace(1)* %back, <4 x float> addrspace(1)* %output, float %intensity, i32 %pixelCountPerGlobalID) nounwind {
entry:
  %front.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %back.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %output.addr = alloca <4 x float> addrspace(1)*, align 4 ; <<4 x float> addrspace(1)**> [#uses=2]
  %intensity.addr = alloca float, align 4         ; <float*> [#uses=2]
  %pixelCountPerGlobalID.addr = alloca i32, align 4 ; <i32*> [#uses=3]
  %global_id = alloca i32, align 4                ; <i32*> [#uses=2]
  %index = alloca i32, align 4                    ; <i32*> [#uses=8]
  %lastIndex = alloca i32, align 4                ; <i32*> [#uses=2]
  store <4 x float> addrspace(1)* %front, <4 x float> addrspace(1)** %front.addr
  store <4 x float> addrspace(1)* %back, <4 x float> addrspace(1)** %back.addr
  store <4 x float> addrspace(1)* %output, <4 x float> addrspace(1)** %output.addr
  store float %intensity, float* %intensity.addr
  store i32 %pixelCountPerGlobalID, i32* %pixelCountPerGlobalID.addr
  %call = call i32 @get_global_id(i32 0)          ; <i32> [#uses=1]
  store i32 %call, i32* %global_id
  %tmp = load i32* %pixelCountPerGlobalID.addr    ; <i32> [#uses=1]
  %tmp1 = load i32* %global_id                    ; <i32> [#uses=1]
  %mul = mul i32 %tmp, %tmp1                      ; <i32> [#uses=1]
  store i32 %mul, i32* %index
  %tmp3 = load i32* %index                        ; <i32> [#uses=1]
  %tmp4 = load i32* %pixelCountPerGlobalID.addr   ; <i32> [#uses=1]
  %add = add i32 %tmp3, %tmp4                     ; <i32> [#uses=1]
  store i32 %add, i32* %lastIndex
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp5 = load i32* %index                        ; <i32> [#uses=1]
  %tmp6 = load i32* %lastIndex                    ; <i32> [#uses=1]
  %cmp = icmp ult i32 %tmp5, %tmp6                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp7 = load i32* %index                        ; <i32> [#uses=1]
  %tmp8 = load <4 x float> addrspace(1)** %front.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx = getelementptr inbounds <4 x float> addrspace(1)* %tmp8, i32 %tmp7 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp9 = load <4 x float> addrspace(1)* %arrayidx ; <<4 x float>> [#uses=1]
  %tmp10 = load i32* %index                       ; <i32> [#uses=1]
  %tmp11 = load <4 x float> addrspace(1)** %back.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx12 = getelementptr inbounds <4 x float> addrspace(1)* %tmp11, i32 %tmp10 ; <<4 x float> addrspace(1)*> [#uses=1]
  %tmp13 = load <4 x float> addrspace(1)* %arrayidx12 ; <<4 x float>> [#uses=1]
  %tmp14 = load float* %intensity.addr            ; <float> [#uses=1]
  %call15 = call <4 x float> @evaluatePixel(<4 x float> %tmp9, <4 x float> %tmp13, float %tmp14) ; <<4 x float>> [#uses=1]
  %tmp16 = load i32* %index                       ; <i32> [#uses=1]
  %tmp17 = load <4 x float> addrspace(1)** %output.addr ; <<4 x float> addrspace(1)*> [#uses=1]
  %arrayidx18 = getelementptr inbounds <4 x float> addrspace(1)* %tmp17, i32 %tmp16 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %call15, <4 x float> addrspace(1)* %arrayidx18
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp19 = load i32* %index                       ; <i32> [#uses=1]
  %inc = add i32 %tmp19, 1                        ; <i32> [#uses=1]
  store i32 %inc, i32* %index
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}
