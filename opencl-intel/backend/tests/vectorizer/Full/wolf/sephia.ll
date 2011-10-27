; RUN: llvm-as %s -o %t.bc
; RUN: opt  -runtimelib %p/../runtime.bc -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -scalarize -mergereturn -loopsimplify -phicanon -predicate -mem2reg -dce -packetize -packet-size=4 -resolve -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; ModuleID = 'sepia.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_sepia_OpenCL_C_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_sepia_OpenCL_C_parameters = appending global [143 x i8] c"float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int, int, int\00", section "llvm.metadata" ; <[143 x i8]*> [#uses=1]
@opencl_sepia_OpenCL_float4_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_sepia_OpenCL_float4_parameters = appending global [145 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int, int, int\00", section "llvm.metadata" ; <[145 x i8]*> [#uses=1]
@opencl_sepia_OpenCL_float4_naive_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_sepia_OpenCL_float4_naive_parameters = appending global [145 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *, int, int, int\00", section "llvm.metadata" ; <[145 x i8]*> [#uses=1]
@opencl_sepia_OpenCL_float4_SSE_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_sepia_OpenCL_float4_SSE_parameters = appending global [146 x i8] c"float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, float4 __attribute__((address_space(1))) *, int, int, int\00", section "llvm.metadata" ; <[146 x i8]*> [#uses=1]
@opencl_sepia_OpenCL_float8_AVX_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_sepia_OpenCL_float8_AVX_parameters = appending global [146 x i8] c"float8 __attribute__((address_space(1))) *, float8 __attribute__((address_space(1))) *, float8 __attribute__((address_space(1))) *, int, int, int\00", section "llvm.metadata" ; <[146 x i8]*> [#uses=1]
@opencl_sepia_OpenCL_float16_LRBNI_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_sepia_OpenCL_float16_LRBNI_parameters = appending global [149 x i8] c"float16 __attribute__((address_space(1))) *, float16 __attribute__((address_space(1))) *, float16 __attribute__((address_space(1))) *, int, int, int\00", section "llvm.metadata" ; <[149 x i8]*> [#uses=1]
@opencl_metadata = appending global [6 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, float addrspace(1)*, float addrspace(1)*, i32, i32, i32)* @sepia_OpenCL_C to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_sepia_OpenCL_C_locals to i8*), i8* getelementptr inbounds ([143 x i8]* @opencl_sepia_OpenCL_C_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, float addrspace(1)*, i32, i32, i32)* @sepia_OpenCL_float4 to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_sepia_OpenCL_float4_locals to i8*), i8* getelementptr inbounds ([145 x i8]* @opencl_sepia_OpenCL_float4_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, float addrspace(1)*, i32, i32, i32)* @sepia_OpenCL_float4_naive to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_sepia_OpenCL_float4_naive_locals to i8*), i8* getelementptr inbounds ([145 x i8]* @opencl_sepia_OpenCL_float4_naive_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<4 x float> addrspace(1)*, <4 x float> addrspace(1)*, <4 x float> addrspace(1)*, i32, i32, i32)* @sepia_OpenCL_float4_SSE to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_sepia_OpenCL_float4_SSE_locals to i8*), i8* getelementptr inbounds ([146 x i8]* @opencl_sepia_OpenCL_float4_SSE_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<8 x float> addrspace(1)*, <8 x float> addrspace(1)*, <8 x float> addrspace(1)*, i32, i32, i32)* @sepia_OpenCL_float8_AVX to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_sepia_OpenCL_float8_AVX_locals to i8*), i8* getelementptr inbounds ([146 x i8]* @opencl_sepia_OpenCL_float8_AVX_parameters, i32 0, i32 0) }>, %opencl_metadata_type <{ i8* bitcast (void (<16 x float> addrspace(1)*, <16 x float> addrspace(1)*, <16 x float> addrspace(1)*, i32, i32, i32)* @sepia_OpenCL_float16_LRBNI to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_sepia_OpenCL_float16_LRBNI_locals to i8*), i8* getelementptr inbounds ([149 x i8]* @opencl_sepia_OpenCL_float16_LRBNI_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[6 x %opencl_metadata_type]*> [#uses=0]


; CHECK: ret
define void @sepia_OpenCL_C(float addrspace(1)* nocapture %src, float addrspace(1)* nocapture %dest, float addrspace(1)* nocapture %coef, i32 %size, i32 %image_height, i32 %Iterations) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = icmp eq i32 %Iterations, 0                 ; <i1> [#uses=1]
  br i1 %2, label %._crit_edge7, label %bb.nph6

bb.nph6:                                          ; preds = %0
  %3 = shl i32 %1, 2                              ; <i32> [#uses=2]
  %4 = getelementptr inbounds float addrspace(1)* %coef, i32 1 ; <float addrspace(1)*> [#uses=1]
  %5 = getelementptr inbounds float addrspace(1)* %coef, i32 2 ; <float addrspace(1)*> [#uses=1]
  %6 = getelementptr inbounds float addrspace(1)* %coef, i32 3 ; <float addrspace(1)*> [#uses=1]
  %7 = getelementptr inbounds float addrspace(1)* %coef, i32 16 ; <float addrspace(1)*> [#uses=1]
  %8 = getelementptr inbounds float addrspace(1)* %coef, i32 17 ; <float addrspace(1)*> [#uses=1]
  %9 = getelementptr inbounds float addrspace(1)* %coef, i32 18 ; <float addrspace(1)*> [#uses=1]
  %10 = getelementptr inbounds float addrspace(1)* %coef, i32 19 ; <float addrspace(1)*> [#uses=1]
  %11 = getelementptr inbounds float addrspace(1)* %coef, i32 32 ; <float addrspace(1)*> [#uses=1]
  %12 = getelementptr inbounds float addrspace(1)* %coef, i32 33 ; <float addrspace(1)*> [#uses=1]
  %13 = getelementptr inbounds float addrspace(1)* %coef, i32 34 ; <float addrspace(1)*> [#uses=1]
  %14 = getelementptr inbounds float addrspace(1)* %coef, i32 35 ; <float addrspace(1)*> [#uses=1]
  %15 = getelementptr inbounds float addrspace(1)* %coef, i32 48 ; <float addrspace(1)*> [#uses=1]
  %16 = getelementptr inbounds float addrspace(1)* %coef, i32 49 ; <float addrspace(1)*> [#uses=1]
  %17 = getelementptr inbounds float addrspace(1)* %coef, i32 50 ; <float addrspace(1)*> [#uses=1]
  %18 = getelementptr inbounds float addrspace(1)* %coef, i32 51 ; <float addrspace(1)*> [#uses=1]
  br label %19

; <label>:19                                      ; preds = %._crit_edge, %bb.nph6
  %i.02 = phi i32 [ 0, %bb.nph6 ], [ %100, %._crit_edge ] ; <i32> [#uses=1]
  %20 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %21 = shl i32 %20, 4                            ; <i32> [#uses=1]
  %22 = icmp ult i32 %3, %21                      ; <i1> [#uses=1]
  br i1 %22, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %19, %bb.nph
  %offset.01 = phi i32 [ %96, %bb.nph ], [ %3, %19 ] ; <i32> [#uses=6]
  %23 = load float addrspace(1)* %coef            ; <float> [#uses=1]
  %24 = getelementptr inbounds float addrspace(1)* %src, i32 %offset.01 ; <float addrspace(1)*> [#uses=4]
  %25 = load float addrspace(1)* %24              ; <float> [#uses=1]
  %26 = fmul float %23, %25                       ; <float> [#uses=2]
  %27 = getelementptr inbounds float addrspace(1)* %dest, i32 %offset.01 ; <float addrspace(1)*> [#uses=4]
  store float %26, float addrspace(1)* %27
  %28 = load float addrspace(1)* %4               ; <float> [#uses=1]
  %29 = load float addrspace(1)* %24              ; <float> [#uses=1]
  %30 = fmul float %28, %29                       ; <float> [#uses=2]
  %31 = or i32 %offset.01, 1                      ; <i32> [#uses=2]
  %32 = getelementptr inbounds float addrspace(1)* %dest, i32 %31 ; <float addrspace(1)*> [#uses=4]
  store float %30, float addrspace(1)* %32
  %33 = load float addrspace(1)* %5               ; <float> [#uses=1]
  %34 = load float addrspace(1)* %24              ; <float> [#uses=1]
  %35 = fmul float %33, %34                       ; <float> [#uses=2]
  %36 = or i32 %offset.01, 2                      ; <i32> [#uses=2]
  %37 = getelementptr inbounds float addrspace(1)* %dest, i32 %36 ; <float addrspace(1)*> [#uses=4]
  store float %35, float addrspace(1)* %37
  %38 = load float addrspace(1)* %6               ; <float> [#uses=1]
  %39 = load float addrspace(1)* %24              ; <float> [#uses=1]
  %40 = fmul float %38, %39                       ; <float> [#uses=2]
  %41 = or i32 %offset.01, 3                      ; <i32> [#uses=2]
  %42 = getelementptr inbounds float addrspace(1)* %dest, i32 %41 ; <float addrspace(1)*> [#uses=4]
  store float %40, float addrspace(1)* %42
  %43 = load float addrspace(1)* %7               ; <float> [#uses=1]
  %44 = getelementptr inbounds float addrspace(1)* %src, i32 %31 ; <float addrspace(1)*> [#uses=4]
  %45 = load float addrspace(1)* %44              ; <float> [#uses=1]
  %46 = fmul float %43, %45                       ; <float> [#uses=1]
  %47 = fadd float %26, %46                       ; <float> [#uses=2]
  store float %47, float addrspace(1)* %27
  %48 = load float addrspace(1)* %8               ; <float> [#uses=1]
  %49 = load float addrspace(1)* %44              ; <float> [#uses=1]
  %50 = fmul float %48, %49                       ; <float> [#uses=1]
  %51 = fadd float %30, %50                       ; <float> [#uses=2]
  store float %51, float addrspace(1)* %32
  %52 = load float addrspace(1)* %9               ; <float> [#uses=1]
  %53 = load float addrspace(1)* %44              ; <float> [#uses=1]
  %54 = fmul float %52, %53                       ; <float> [#uses=1]
  %55 = fadd float %35, %54                       ; <float> [#uses=2]
  store float %55, float addrspace(1)* %37
  %56 = load float addrspace(1)* %10              ; <float> [#uses=1]
  %57 = load float addrspace(1)* %44              ; <float> [#uses=1]
  %58 = fmul float %56, %57                       ; <float> [#uses=1]
  %59 = fadd float %40, %58                       ; <float> [#uses=2]
  store float %59, float addrspace(1)* %42
  %60 = load float addrspace(1)* %11              ; <float> [#uses=1]
  %61 = getelementptr inbounds float addrspace(1)* %src, i32 %36 ; <float addrspace(1)*> [#uses=4]
  %62 = load float addrspace(1)* %61              ; <float> [#uses=1]
  %63 = fmul float %60, %62                       ; <float> [#uses=1]
  %64 = fadd float %47, %63                       ; <float> [#uses=2]
  store float %64, float addrspace(1)* %27
  %65 = load float addrspace(1)* %12              ; <float> [#uses=1]
  %66 = load float addrspace(1)* %61              ; <float> [#uses=1]
  %67 = fmul float %65, %66                       ; <float> [#uses=1]
  %68 = fadd float %51, %67                       ; <float> [#uses=2]
  store float %68, float addrspace(1)* %32
  %69 = load float addrspace(1)* %13              ; <float> [#uses=1]
  %70 = load float addrspace(1)* %61              ; <float> [#uses=1]
  %71 = fmul float %69, %70                       ; <float> [#uses=1]
  %72 = fadd float %55, %71                       ; <float> [#uses=2]
  store float %72, float addrspace(1)* %37
  %73 = load float addrspace(1)* %14              ; <float> [#uses=1]
  %74 = load float addrspace(1)* %61              ; <float> [#uses=1]
  %75 = fmul float %73, %74                       ; <float> [#uses=1]
  %76 = fadd float %59, %75                       ; <float> [#uses=2]
  store float %76, float addrspace(1)* %42
  %77 = load float addrspace(1)* %15              ; <float> [#uses=1]
  %78 = getelementptr inbounds float addrspace(1)* %src, i32 %41 ; <float addrspace(1)*> [#uses=4]
  %79 = load float addrspace(1)* %78              ; <float> [#uses=1]
  %80 = fmul float %77, %79                       ; <float> [#uses=1]
  %81 = fadd float %64, %80                       ; <float> [#uses=1]
  store float %81, float addrspace(1)* %27
  %82 = load float addrspace(1)* %16              ; <float> [#uses=1]
  %83 = load float addrspace(1)* %78              ; <float> [#uses=1]
  %84 = fmul float %82, %83                       ; <float> [#uses=1]
  %85 = fadd float %68, %84                       ; <float> [#uses=1]
  store float %85, float addrspace(1)* %32
  %86 = load float addrspace(1)* %17              ; <float> [#uses=1]
  %87 = load float addrspace(1)* %78              ; <float> [#uses=1]
  %88 = fmul float %86, %87                       ; <float> [#uses=1]
  %89 = fadd float %72, %88                       ; <float> [#uses=1]
  store float %89, float addrspace(1)* %37
  %90 = load float addrspace(1)* %18              ; <float> [#uses=1]
  %91 = load float addrspace(1)* %78              ; <float> [#uses=1]
  %92 = fmul float %90, %91                       ; <float> [#uses=1]
  %93 = fadd float %76, %92                       ; <float> [#uses=1]
  store float %93, float addrspace(1)* %42
  %94 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %95 = shl i32 %94, 2                            ; <i32> [#uses=1]
  %96 = add i32 %95, %offset.01                   ; <i32> [#uses=2]
  %97 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %98 = shl i32 %97, 4                            ; <i32> [#uses=1]
  %99 = icmp ult i32 %96, %98                     ; <i1> [#uses=1]
  br i1 %99, label %bb.nph, label %._crit_edge

._crit_edge:                                      ; preds = %bb.nph, %19
  %100 = add i32 %i.02, 1                         ; <i32> [#uses=2]
  %exitcond = icmp eq i32 %100, %Iterations       ; <i1> [#uses=1]
  br i1 %exitcond, label %._crit_edge7, label %19

._crit_edge7:                                     ; preds = %._crit_edge, %0
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_global_size(i32)

define void @sepia_OpenCL_float4(<4 x float> addrspace(1)* nocapture %src, <4 x float> addrspace(1)* nocapture %dest, float addrspace(1)* nocapture %coef, i32 %size, i32 %image_height, i32 %Iterations) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=2]
  %2 = icmp eq i32 %Iterations, 0                 ; <i1> [#uses=1]
  br i1 %2, label %._crit_edge7, label %bb.nph6

bb.nph6:                                          ; preds = %0
  %3 = getelementptr inbounds float addrspace(1)* %coef, i32 1 ; <float addrspace(1)*> [#uses=1]
  %4 = getelementptr inbounds float addrspace(1)* %coef, i32 2 ; <float addrspace(1)*> [#uses=1]
  %5 = getelementptr inbounds float addrspace(1)* %coef, i32 3 ; <float addrspace(1)*> [#uses=1]
  %6 = getelementptr inbounds float addrspace(1)* %coef, i32 16 ; <float addrspace(1)*> [#uses=1]
  %7 = getelementptr inbounds float addrspace(1)* %coef, i32 17 ; <float addrspace(1)*> [#uses=1]
  %8 = getelementptr inbounds float addrspace(1)* %coef, i32 18 ; <float addrspace(1)*> [#uses=1]
  %9 = getelementptr inbounds float addrspace(1)* %coef, i32 19 ; <float addrspace(1)*> [#uses=1]
  %10 = getelementptr inbounds float addrspace(1)* %coef, i32 32 ; <float addrspace(1)*> [#uses=1]
  %11 = getelementptr inbounds float addrspace(1)* %coef, i32 33 ; <float addrspace(1)*> [#uses=1]
  %12 = getelementptr inbounds float addrspace(1)* %coef, i32 34 ; <float addrspace(1)*> [#uses=1]
  %13 = getelementptr inbounds float addrspace(1)* %coef, i32 35 ; <float addrspace(1)*> [#uses=1]
  %14 = getelementptr inbounds float addrspace(1)* %coef, i32 48 ; <float addrspace(1)*> [#uses=1]
  %15 = getelementptr inbounds float addrspace(1)* %coef, i32 49 ; <float addrspace(1)*> [#uses=1]
  %16 = getelementptr inbounds float addrspace(1)* %coef, i32 50 ; <float addrspace(1)*> [#uses=1]
  %17 = getelementptr inbounds float addrspace(1)* %coef, i32 51 ; <float addrspace(1)*> [#uses=1]
  br label %.preheader

.preheader:                                       ; preds = %._crit_edge, %bb.nph6
  %i.02 = phi i32 [ 0, %bb.nph6 ], [ %121, %._crit_edge ] ; <i32> [#uses=1]
  %18 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %19 = shl i32 %18, 2                            ; <i32> [#uses=1]
  %20 = icmp ult i32 %1, %19                      ; <i1> [#uses=1]
  br i1 %20, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %.preheader, %bb.nph
  %offset.01 = phi i32 [ %117, %bb.nph ], [ %1, %.preheader ] ; <i32> [#uses=3]
  %21 = load float addrspace(1)* %coef            ; <float> [#uses=1]
  %22 = getelementptr inbounds <4 x float> addrspace(1)* %src, i32 %offset.01 ; <<4 x float> addrspace(1)*> [#uses=16]
  %23 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %24 = extractelement <4 x float> %23, i32 0     ; <float> [#uses=1]
  %25 = fmul float %21, %24                       ; <float> [#uses=2]
  %26 = getelementptr inbounds <4 x float> addrspace(1)* %dest, i32 %offset.01 ; <<4 x float> addrspace(1)*> [#uses=17]
  %27 = load <4 x float> addrspace(1)* %26        ; <<4 x float>> [#uses=1]
  %28 = insertelement <4 x float> %27, float %25, i32 0 ; <<4 x float>> [#uses=2]
  store <4 x float> %28, <4 x float> addrspace(1)* %26
  %29 = load float addrspace(1)* %3               ; <float> [#uses=1]
  %30 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %31 = extractelement <4 x float> %30, i32 0     ; <float> [#uses=1]
  %32 = fmul float %29, %31                       ; <float> [#uses=2]
  %33 = insertelement <4 x float> %28, float %32, i32 1 ; <<4 x float>> [#uses=2]
  store <4 x float> %33, <4 x float> addrspace(1)* %26
  %34 = load float addrspace(1)* %4               ; <float> [#uses=1]
  %35 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %36 = extractelement <4 x float> %35, i32 0     ; <float> [#uses=1]
  %37 = fmul float %34, %36                       ; <float> [#uses=2]
  %38 = insertelement <4 x float> %33, float %37, i32 2 ; <<4 x float>> [#uses=2]
  store <4 x float> %38, <4 x float> addrspace(1)* %26
  %39 = load float addrspace(1)* %5               ; <float> [#uses=1]
  %40 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %41 = extractelement <4 x float> %40, i32 0     ; <float> [#uses=1]
  %42 = fmul float %39, %41                       ; <float> [#uses=2]
  %43 = insertelement <4 x float> %38, float %42, i32 3 ; <<4 x float>> [#uses=2]
  store <4 x float> %43, <4 x float> addrspace(1)* %26
  %44 = load float addrspace(1)* %6               ; <float> [#uses=1]
  %45 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %46 = extractelement <4 x float> %45, i32 1     ; <float> [#uses=1]
  %47 = fmul float %44, %46                       ; <float> [#uses=1]
  %48 = fadd float %25, %47                       ; <float> [#uses=2]
  %49 = insertelement <4 x float> %43, float %48, i32 0 ; <<4 x float>> [#uses=2]
  store <4 x float> %49, <4 x float> addrspace(1)* %26
  %50 = load float addrspace(1)* %7               ; <float> [#uses=1]
  %51 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %52 = extractelement <4 x float> %51, i32 1     ; <float> [#uses=1]
  %53 = fmul float %50, %52                       ; <float> [#uses=1]
  %54 = fadd float %32, %53                       ; <float> [#uses=2]
  %55 = insertelement <4 x float> %49, float %54, i32 1 ; <<4 x float>> [#uses=2]
  store <4 x float> %55, <4 x float> addrspace(1)* %26
  %56 = load float addrspace(1)* %8               ; <float> [#uses=1]
  %57 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %58 = extractelement <4 x float> %57, i32 1     ; <float> [#uses=1]
  %59 = fmul float %56, %58                       ; <float> [#uses=1]
  %60 = fadd float %37, %59                       ; <float> [#uses=2]
  %61 = insertelement <4 x float> %55, float %60, i32 2 ; <<4 x float>> [#uses=2]
  store <4 x float> %61, <4 x float> addrspace(1)* %26
  %62 = load float addrspace(1)* %9               ; <float> [#uses=1]
  %63 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %64 = extractelement <4 x float> %63, i32 1     ; <float> [#uses=1]
  %65 = fmul float %62, %64                       ; <float> [#uses=1]
  %66 = fadd float %42, %65                       ; <float> [#uses=2]
  %67 = insertelement <4 x float> %61, float %66, i32 3 ; <<4 x float>> [#uses=2]
  store <4 x float> %67, <4 x float> addrspace(1)* %26
  %68 = load float addrspace(1)* %10              ; <float> [#uses=1]
  %69 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %70 = extractelement <4 x float> %69, i32 2     ; <float> [#uses=1]
  %71 = fmul float %68, %70                       ; <float> [#uses=1]
  %72 = fadd float %48, %71                       ; <float> [#uses=2]
  %73 = insertelement <4 x float> %67, float %72, i32 0 ; <<4 x float>> [#uses=2]
  store <4 x float> %73, <4 x float> addrspace(1)* %26
  %74 = load float addrspace(1)* %11              ; <float> [#uses=1]
  %75 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %76 = extractelement <4 x float> %75, i32 2     ; <float> [#uses=1]
  %77 = fmul float %74, %76                       ; <float> [#uses=1]
  %78 = fadd float %54, %77                       ; <float> [#uses=2]
  %79 = insertelement <4 x float> %73, float %78, i32 1 ; <<4 x float>> [#uses=2]
  store <4 x float> %79, <4 x float> addrspace(1)* %26
  %80 = load float addrspace(1)* %12              ; <float> [#uses=1]
  %81 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %82 = extractelement <4 x float> %81, i32 2     ; <float> [#uses=1]
  %83 = fmul float %80, %82                       ; <float> [#uses=1]
  %84 = fadd float %60, %83                       ; <float> [#uses=2]
  %85 = insertelement <4 x float> %79, float %84, i32 2 ; <<4 x float>> [#uses=2]
  store <4 x float> %85, <4 x float> addrspace(1)* %26
  %86 = load float addrspace(1)* %13              ; <float> [#uses=1]
  %87 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %88 = extractelement <4 x float> %87, i32 2     ; <float> [#uses=1]
  %89 = fmul float %86, %88                       ; <float> [#uses=1]
  %90 = fadd float %66, %89                       ; <float> [#uses=2]
  %91 = insertelement <4 x float> %85, float %90, i32 3 ; <<4 x float>> [#uses=2]
  store <4 x float> %91, <4 x float> addrspace(1)* %26
  %92 = load float addrspace(1)* %14              ; <float> [#uses=1]
  %93 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %94 = extractelement <4 x float> %93, i32 3     ; <float> [#uses=1]
  %95 = fmul float %92, %94                       ; <float> [#uses=1]
  %96 = fadd float %72, %95                       ; <float> [#uses=1]
  %97 = insertelement <4 x float> %91, float %96, i32 0 ; <<4 x float>> [#uses=2]
  store <4 x float> %97, <4 x float> addrspace(1)* %26
  %98 = load float addrspace(1)* %15              ; <float> [#uses=1]
  %99 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=1]
  %100 = extractelement <4 x float> %99, i32 3    ; <float> [#uses=1]
  %101 = fmul float %98, %100                     ; <float> [#uses=1]
  %102 = fadd float %78, %101                     ; <float> [#uses=1]
  %103 = insertelement <4 x float> %97, float %102, i32 1 ; <<4 x float>> [#uses=2]
  store <4 x float> %103, <4 x float> addrspace(1)* %26
  %104 = load float addrspace(1)* %16             ; <float> [#uses=1]
  %105 = load <4 x float> addrspace(1)* %22       ; <<4 x float>> [#uses=1]
  %106 = extractelement <4 x float> %105, i32 3   ; <float> [#uses=1]
  %107 = fmul float %104, %106                    ; <float> [#uses=1]
  %108 = fadd float %84, %107                     ; <float> [#uses=1]
  %109 = insertelement <4 x float> %103, float %108, i32 2 ; <<4 x float>> [#uses=2]
  store <4 x float> %109, <4 x float> addrspace(1)* %26
  %110 = load float addrspace(1)* %17             ; <float> [#uses=1]
  %111 = load <4 x float> addrspace(1)* %22       ; <<4 x float>> [#uses=1]
  %112 = extractelement <4 x float> %111, i32 3   ; <float> [#uses=1]
  %113 = fmul float %110, %112                    ; <float> [#uses=1]
  %114 = fadd float %90, %113                     ; <float> [#uses=1]
  %115 = insertelement <4 x float> %109, float %114, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %115, <4 x float> addrspace(1)* %26
  %116 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %117 = add i32 %116, %offset.01                 ; <i32> [#uses=2]
  %118 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %119 = shl i32 %118, 2                          ; <i32> [#uses=1]
  %120 = icmp ult i32 %117, %119                  ; <i1> [#uses=1]
  br i1 %120, label %bb.nph, label %._crit_edge

._crit_edge:                                      ; preds = %bb.nph, %.preheader
  %121 = add i32 %i.02, 1                         ; <i32> [#uses=2]
  %exitcond = icmp eq i32 %121, %Iterations       ; <i1> [#uses=1]
  br i1 %exitcond, label %._crit_edge7, label %.preheader

._crit_edge7:                                     ; preds = %._crit_edge, %0
  ret void
}

define void @sepia_OpenCL_float4_naive(<4 x float> addrspace(1)* nocapture %src, <4 x float> addrspace(1)* nocapture %dest, float addrspace(1)* nocapture %coef, i32 %size, i32 %image_height, i32 %Iterations) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=2]
  %2 = icmp eq i32 %Iterations, 0                 ; <i1> [#uses=1]
  br i1 %2, label %._crit_edge7, label %bb.nph6

bb.nph6:                                          ; preds = %0
  %3 = getelementptr inbounds float addrspace(1)* %coef, i32 16 ; <float addrspace(1)*> [#uses=1]
  %4 = getelementptr inbounds float addrspace(1)* %coef, i32 32 ; <float addrspace(1)*> [#uses=1]
  %5 = getelementptr inbounds float addrspace(1)* %coef, i32 48 ; <float addrspace(1)*> [#uses=1]
  %6 = getelementptr inbounds float addrspace(1)* %coef, i32 1 ; <float addrspace(1)*> [#uses=1]
  %7 = getelementptr inbounds float addrspace(1)* %coef, i32 17 ; <float addrspace(1)*> [#uses=1]
  %8 = getelementptr inbounds float addrspace(1)* %coef, i32 33 ; <float addrspace(1)*> [#uses=1]
  %9 = getelementptr inbounds float addrspace(1)* %coef, i32 49 ; <float addrspace(1)*> [#uses=1]
  %10 = getelementptr inbounds float addrspace(1)* %coef, i32 2 ; <float addrspace(1)*> [#uses=1]
  %11 = getelementptr inbounds float addrspace(1)* %coef, i32 18 ; <float addrspace(1)*> [#uses=1]
  %12 = getelementptr inbounds float addrspace(1)* %coef, i32 34 ; <float addrspace(1)*> [#uses=1]
  %13 = getelementptr inbounds float addrspace(1)* %coef, i32 50 ; <float addrspace(1)*> [#uses=1]
  %14 = getelementptr inbounds float addrspace(1)* %coef, i32 3 ; <float addrspace(1)*> [#uses=1]
  %15 = getelementptr inbounds float addrspace(1)* %coef, i32 19 ; <float addrspace(1)*> [#uses=1]
  %16 = getelementptr inbounds float addrspace(1)* %coef, i32 35 ; <float addrspace(1)*> [#uses=1]
  %17 = getelementptr inbounds float addrspace(1)* %coef, i32 51 ; <float addrspace(1)*> [#uses=1]
  br label %.preheader

.preheader:                                       ; preds = %._crit_edge, %bb.nph6
  %i.02 = phi i32 [ 0, %bb.nph6 ], [ %97, %._crit_edge ] ; <i32> [#uses=1]
  %18 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %19 = shl i32 %18, 2                            ; <i32> [#uses=1]
  %20 = icmp ult i32 %1, %19                      ; <i1> [#uses=1]
  br i1 %20, label %bb.nph, label %._crit_edge

bb.nph:                                           ; preds = %.preheader, %bb.nph
  %offset.01 = phi i32 [ %93, %bb.nph ], [ %1, %.preheader ] ; <i32> [#uses=3]
  %21 = load float addrspace(1)* %coef            ; <float> [#uses=1]
  %22 = getelementptr inbounds <4 x float> addrspace(1)* %src, i32 %offset.01 ; <<4 x float> addrspace(1)*> [#uses=4]
  %23 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=4]
  %24 = extractelement <4 x float> %23, i32 0     ; <float> [#uses=1]
  %25 = fmul float %21, %24                       ; <float> [#uses=1]
  %26 = load float addrspace(1)* %3               ; <float> [#uses=1]
  %27 = extractelement <4 x float> %23, i32 1     ; <float> [#uses=1]
  %28 = fmul float %26, %27                       ; <float> [#uses=1]
  %29 = fadd float %25, %28                       ; <float> [#uses=1]
  %30 = load float addrspace(1)* %4               ; <float> [#uses=1]
  %31 = extractelement <4 x float> %23, i32 2     ; <float> [#uses=1]
  %32 = fmul float %30, %31                       ; <float> [#uses=1]
  %33 = fadd float %29, %32                       ; <float> [#uses=1]
  %34 = load float addrspace(1)* %5               ; <float> [#uses=1]
  %35 = extractelement <4 x float> %23, i32 3     ; <float> [#uses=1]
  %36 = fmul float %34, %35                       ; <float> [#uses=1]
  %37 = fadd float %33, %36                       ; <float> [#uses=1]
  %38 = getelementptr inbounds <4 x float> addrspace(1)* %dest, i32 %offset.01 ; <<4 x float> addrspace(1)*> [#uses=5]
  %39 = load <4 x float> addrspace(1)* %38        ; <<4 x float>> [#uses=1]
  %40 = insertelement <4 x float> %39, float %37, i32 0 ; <<4 x float>> [#uses=2]
  store <4 x float> %40, <4 x float> addrspace(1)* %38
  %41 = load float addrspace(1)* %6               ; <float> [#uses=1]
  %42 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=4]
  %43 = extractelement <4 x float> %42, i32 0     ; <float> [#uses=1]
  %44 = fmul float %41, %43                       ; <float> [#uses=1]
  %45 = load float addrspace(1)* %7               ; <float> [#uses=1]
  %46 = extractelement <4 x float> %42, i32 1     ; <float> [#uses=1]
  %47 = fmul float %45, %46                       ; <float> [#uses=1]
  %48 = fadd float %44, %47                       ; <float> [#uses=1]
  %49 = load float addrspace(1)* %8               ; <float> [#uses=1]
  %50 = extractelement <4 x float> %42, i32 2     ; <float> [#uses=1]
  %51 = fmul float %49, %50                       ; <float> [#uses=1]
  %52 = fadd float %48, %51                       ; <float> [#uses=1]
  %53 = load float addrspace(1)* %9               ; <float> [#uses=1]
  %54 = extractelement <4 x float> %42, i32 3     ; <float> [#uses=1]
  %55 = fmul float %53, %54                       ; <float> [#uses=1]
  %56 = fadd float %52, %55                       ; <float> [#uses=1]
  %57 = insertelement <4 x float> %40, float %56, i32 1 ; <<4 x float>> [#uses=2]
  store <4 x float> %57, <4 x float> addrspace(1)* %38
  %58 = load float addrspace(1)* %10              ; <float> [#uses=1]
  %59 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=4]
  %60 = extractelement <4 x float> %59, i32 0     ; <float> [#uses=1]
  %61 = fmul float %58, %60                       ; <float> [#uses=1]
  %62 = load float addrspace(1)* %11              ; <float> [#uses=1]
  %63 = extractelement <4 x float> %59, i32 1     ; <float> [#uses=1]
  %64 = fmul float %62, %63                       ; <float> [#uses=1]
  %65 = fadd float %61, %64                       ; <float> [#uses=1]
  %66 = load float addrspace(1)* %12              ; <float> [#uses=1]
  %67 = extractelement <4 x float> %59, i32 2     ; <float> [#uses=1]
  %68 = fmul float %66, %67                       ; <float> [#uses=1]
  %69 = fadd float %65, %68                       ; <float> [#uses=1]
  %70 = load float addrspace(1)* %13              ; <float> [#uses=1]
  %71 = extractelement <4 x float> %59, i32 3     ; <float> [#uses=1]
  %72 = fmul float %70, %71                       ; <float> [#uses=1]
  %73 = fadd float %69, %72                       ; <float> [#uses=1]
  %74 = insertelement <4 x float> %57, float %73, i32 2 ; <<4 x float>> [#uses=2]
  store <4 x float> %74, <4 x float> addrspace(1)* %38
  %75 = load float addrspace(1)* %14              ; <float> [#uses=1]
  %76 = load <4 x float> addrspace(1)* %22        ; <<4 x float>> [#uses=4]
  %77 = extractelement <4 x float> %76, i32 0     ; <float> [#uses=1]
  %78 = fmul float %75, %77                       ; <float> [#uses=1]
  %79 = load float addrspace(1)* %15              ; <float> [#uses=1]
  %80 = extractelement <4 x float> %76, i32 1     ; <float> [#uses=1]
  %81 = fmul float %79, %80                       ; <float> [#uses=1]
  %82 = fadd float %78, %81                       ; <float> [#uses=1]
  %83 = load float addrspace(1)* %16              ; <float> [#uses=1]
  %84 = extractelement <4 x float> %76, i32 2     ; <float> [#uses=1]
  %85 = fmul float %83, %84                       ; <float> [#uses=1]
  %86 = fadd float %82, %85                       ; <float> [#uses=1]
  %87 = load float addrspace(1)* %17              ; <float> [#uses=1]
  %88 = extractelement <4 x float> %76, i32 3     ; <float> [#uses=1]
  %89 = fmul float %87, %88                       ; <float> [#uses=1]
  %90 = fadd float %86, %89                       ; <float> [#uses=1]
  %91 = insertelement <4 x float> %74, float %90, i32 3 ; <<4 x float>> [#uses=1]
  store <4 x float> %91, <4 x float> addrspace(1)* %38
  %92 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %93 = add i32 %92, %offset.01                   ; <i32> [#uses=2]
  %94 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %95 = shl i32 %94, 2                            ; <i32> [#uses=1]
  %96 = icmp ult i32 %93, %95                     ; <i1> [#uses=1]
  br i1 %96, label %bb.nph, label %._crit_edge

._crit_edge:                                      ; preds = %bb.nph, %.preheader
  %97 = add i32 %i.02, 1                          ; <i32> [#uses=2]
  %exitcond = icmp eq i32 %97, %Iterations        ; <i1> [#uses=1]
  br i1 %exitcond, label %._crit_edge7, label %.preheader

._crit_edge7:                                     ; preds = %._crit_edge, %0
  ret void
}

define void @sepia_OpenCL_float4_SSE(<4 x float> addrspace(1)* nocapture %src, <4 x float> addrspace(1)* nocapture %dest, <4 x float> addrspace(1)* nocapture %coef, i32 %size, i32 %image_height, i32 %Iterations) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=3]
  %2 = icmp eq i32 %Iterations, 0                 ; <i1> [#uses=1]
  br i1 %2, label %._crit_edge, label %bb.nph

bb.nph:                                           ; preds = %0
  %3 = getelementptr inbounds <4 x float> addrspace(1)* %coef, i32 4 ; <<4 x float> addrspace(1)*> [#uses=1]
  %4 = getelementptr inbounds <4 x float> addrspace(1)* %coef, i32 8 ; <<4 x float> addrspace(1)*> [#uses=1]
  %5 = getelementptr inbounds <4 x float> addrspace(1)* %coef, i32 12 ; <<4 x float> addrspace(1)*> [#uses=1]
  %6 = getelementptr inbounds <4 x float> addrspace(1)* %src, i32 %1 ; <<4 x float> addrspace(1)*> [#uses=1]
  %7 = getelementptr inbounds <4 x float> addrspace(1)* %dest, i32 %1 ; <<4 x float> addrspace(1)*> [#uses=1]
  br label %8

; <label>:8                                       ; preds = %8, %bb.nph
  %i.01 = phi i32 [ 0, %bb.nph ], [ %73, %8 ]     ; <i32> [#uses=1]
  %9 = load <4 x float> addrspace(1)* %coef       ; <<4 x float>> [#uses=4]
  %10 = load <4 x float> addrspace(1)* %3         ; <<4 x float>> [#uses=4]
  %11 = load <4 x float> addrspace(1)* %4         ; <<4 x float>> [#uses=4]
  %12 = load <4 x float> addrspace(1)* %5         ; <<4 x float>> [#uses=4]
  %13 = load <4 x float> addrspace(1)* %6         ; <<4 x float>> [#uses=4]
  %14 = shufflevector <4 x float> %13, <4 x float> undef, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %15 = fmul <4 x float> %9, %14                  ; <<4 x float>> [#uses=1]
  %16 = shufflevector <4 x float> %13, <4 x float> undef, <4 x i32> <i32 1, i32 1, i32 1, i32 1> ; <<4 x float>> [#uses=1]
  %17 = fmul <4 x float> %10, %16                 ; <<4 x float>> [#uses=1]
  %18 = fadd <4 x float> %15, %17                 ; <<4 x float>> [#uses=1]
  %19 = shufflevector <4 x float> %13, <4 x float> undef, <4 x i32> <i32 2, i32 2, i32 2, i32 2> ; <<4 x float>> [#uses=1]
  %20 = fmul <4 x float> %11, %19                 ; <<4 x float>> [#uses=1]
  %21 = fadd <4 x float> %18, %20                 ; <<4 x float>> [#uses=1]
  %22 = shufflevector <4 x float> %13, <4 x float> undef, <4 x i32> <i32 3, i32 3, i32 3, i32 3> ; <<4 x float>> [#uses=1]
  %23 = fmul <4 x float> %12, %22                 ; <<4 x float>> [#uses=1]
  %24 = fadd <4 x float> %21, %23                 ; <<4 x float>> [#uses=1]
  store <4 x float> %24, <4 x float> addrspace(1)* %7
  %25 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %26 = add i32 %25, %1                           ; <i32> [#uses=3]
  %27 = getelementptr inbounds <4 x float> addrspace(1)* %src, i32 %26 ; <<4 x float> addrspace(1)*> [#uses=1]
  %28 = load <4 x float> addrspace(1)* %27        ; <<4 x float>> [#uses=4]
  %29 = shufflevector <4 x float> %28, <4 x float> undef, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %30 = fmul <4 x float> %9, %29                  ; <<4 x float>> [#uses=1]
  %31 = shufflevector <4 x float> %28, <4 x float> undef, <4 x i32> <i32 1, i32 1, i32 1, i32 1> ; <<4 x float>> [#uses=1]
  %32 = fmul <4 x float> %10, %31                 ; <<4 x float>> [#uses=1]
  %33 = fadd <4 x float> %30, %32                 ; <<4 x float>> [#uses=1]
  %34 = shufflevector <4 x float> %28, <4 x float> undef, <4 x i32> <i32 2, i32 2, i32 2, i32 2> ; <<4 x float>> [#uses=1]
  %35 = fmul <4 x float> %11, %34                 ; <<4 x float>> [#uses=1]
  %36 = fadd <4 x float> %33, %35                 ; <<4 x float>> [#uses=1]
  %37 = shufflevector <4 x float> %28, <4 x float> undef, <4 x i32> <i32 3, i32 3, i32 3, i32 3> ; <<4 x float>> [#uses=1]
  %38 = fmul <4 x float> %12, %37                 ; <<4 x float>> [#uses=1]
  %39 = fadd <4 x float> %36, %38                 ; <<4 x float>> [#uses=1]
  %40 = getelementptr inbounds <4 x float> addrspace(1)* %dest, i32 %26 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %39, <4 x float> addrspace(1)* %40
  %41 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %42 = add i32 %41, %26                          ; <i32> [#uses=3]
  %43 = getelementptr inbounds <4 x float> addrspace(1)* %src, i32 %42 ; <<4 x float> addrspace(1)*> [#uses=1]
  %44 = load <4 x float> addrspace(1)* %43        ; <<4 x float>> [#uses=4]
  %45 = shufflevector <4 x float> %44, <4 x float> undef, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %46 = fmul <4 x float> %9, %45                  ; <<4 x float>> [#uses=1]
  %47 = shufflevector <4 x float> %44, <4 x float> undef, <4 x i32> <i32 1, i32 1, i32 1, i32 1> ; <<4 x float>> [#uses=1]
  %48 = fmul <4 x float> %10, %47                 ; <<4 x float>> [#uses=1]
  %49 = fadd <4 x float> %46, %48                 ; <<4 x float>> [#uses=1]
  %50 = shufflevector <4 x float> %44, <4 x float> undef, <4 x i32> <i32 2, i32 2, i32 2, i32 2> ; <<4 x float>> [#uses=1]
  %51 = fmul <4 x float> %11, %50                 ; <<4 x float>> [#uses=1]
  %52 = fadd <4 x float> %49, %51                 ; <<4 x float>> [#uses=1]
  %53 = shufflevector <4 x float> %44, <4 x float> undef, <4 x i32> <i32 3, i32 3, i32 3, i32 3> ; <<4 x float>> [#uses=1]
  %54 = fmul <4 x float> %12, %53                 ; <<4 x float>> [#uses=1]
  %55 = fadd <4 x float> %52, %54                 ; <<4 x float>> [#uses=1]
  %56 = getelementptr inbounds <4 x float> addrspace(1)* %dest, i32 %42 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %55, <4 x float> addrspace(1)* %56
  %57 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %58 = add i32 %57, %42                          ; <i32> [#uses=2]
  %59 = getelementptr inbounds <4 x float> addrspace(1)* %src, i32 %58 ; <<4 x float> addrspace(1)*> [#uses=1]
  %60 = load <4 x float> addrspace(1)* %59        ; <<4 x float>> [#uses=4]
  %61 = shufflevector <4 x float> %60, <4 x float> undef, <4 x i32> zeroinitializer ; <<4 x float>> [#uses=1]
  %62 = fmul <4 x float> %9, %61                  ; <<4 x float>> [#uses=1]
  %63 = shufflevector <4 x float> %60, <4 x float> undef, <4 x i32> <i32 1, i32 1, i32 1, i32 1> ; <<4 x float>> [#uses=1]
  %64 = fmul <4 x float> %10, %63                 ; <<4 x float>> [#uses=1]
  %65 = fadd <4 x float> %62, %64                 ; <<4 x float>> [#uses=1]
  %66 = shufflevector <4 x float> %60, <4 x float> undef, <4 x i32> <i32 2, i32 2, i32 2, i32 2> ; <<4 x float>> [#uses=1]
  %67 = fmul <4 x float> %11, %66                 ; <<4 x float>> [#uses=1]
  %68 = fadd <4 x float> %65, %67                 ; <<4 x float>> [#uses=1]
  %69 = shufflevector <4 x float> %60, <4 x float> undef, <4 x i32> <i32 3, i32 3, i32 3, i32 3> ; <<4 x float>> [#uses=1]
  %70 = fmul <4 x float> %12, %69                 ; <<4 x float>> [#uses=1]
  %71 = fadd <4 x float> %68, %70                 ; <<4 x float>> [#uses=1]
  %72 = getelementptr inbounds <4 x float> addrspace(1)* %dest, i32 %58 ; <<4 x float> addrspace(1)*> [#uses=1]
  store <4 x float> %71, <4 x float> addrspace(1)* %72
  %73 = add i32 %i.01, 1                          ; <i32> [#uses=2]
  %exitcond = icmp eq i32 %73, %Iterations        ; <i1> [#uses=1]
  br i1 %exitcond, label %._crit_edge, label %8

._crit_edge:                                      ; preds = %8, %0
  ret void
}

define void @sepia_OpenCL_float8_AVX(<8 x float> addrspace(1)* nocapture %src, <8 x float> addrspace(1)* nocapture %dest, <8 x float> addrspace(1)* nocapture %coef, i32 %size, i32 %image_height, i32 %Iterations) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=3]
  %2 = icmp eq i32 %Iterations, 0                 ; <i1> [#uses=1]
  br i1 %2, label %._crit_edge, label %bb.nph

bb.nph:                                           ; preds = %0
  %3 = getelementptr inbounds <8 x float> addrspace(1)* %coef, i32 2 ; <<8 x float> addrspace(1)*> [#uses=1]
  %4 = getelementptr inbounds <8 x float> addrspace(1)* %coef, i32 4 ; <<8 x float> addrspace(1)*> [#uses=1]
  %5 = getelementptr inbounds <8 x float> addrspace(1)* %coef, i32 6 ; <<8 x float> addrspace(1)*> [#uses=1]
  %6 = getelementptr inbounds <8 x float> addrspace(1)* %src, i32 %1 ; <<8 x float> addrspace(1)*> [#uses=1]
  %7 = getelementptr inbounds <8 x float> addrspace(1)* %dest, i32 %1 ; <<8 x float> addrspace(1)*> [#uses=1]
  br label %8

; <label>:8                                       ; preds = %8, %bb.nph
  %i.01 = phi i32 [ 0, %bb.nph ], [ %41, %8 ]     ; <i32> [#uses=1]
  %9 = load <8 x float> addrspace(1)* %coef       ; <<8 x float>> [#uses=2]
  %10 = load <8 x float> addrspace(1)* %3         ; <<8 x float>> [#uses=2]
  %11 = load <8 x float> addrspace(1)* %4         ; <<8 x float>> [#uses=2]
  %12 = load <8 x float> addrspace(1)* %5         ; <<8 x float>> [#uses=2]
  %13 = load <8 x float> addrspace(1)* %6         ; <<8 x float>> [#uses=4]
  %14 = shufflevector <8 x float> %13, <8 x float> undef, <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 4, i32 4, i32 4, i32 4> ; <<8 x float>> [#uses=1]
  %15 = fmul <8 x float> %9, %14                  ; <<8 x float>> [#uses=1]
  %16 = shufflevector <8 x float> %13, <8 x float> undef, <8 x i32> <i32 1, i32 1, i32 1, i32 1, i32 5, i32 5, i32 5, i32 5> ; <<8 x float>> [#uses=1]
  %17 = fmul <8 x float> %10, %16                 ; <<8 x float>> [#uses=1]
  %18 = fadd <8 x float> %15, %17                 ; <<8 x float>> [#uses=1]
  %19 = shufflevector <8 x float> %13, <8 x float> undef, <8 x i32> <i32 2, i32 2, i32 2, i32 2, i32 6, i32 6, i32 6, i32 6> ; <<8 x float>> [#uses=1]
  %20 = fmul <8 x float> %11, %19                 ; <<8 x float>> [#uses=1]
  %21 = fadd <8 x float> %18, %20                 ; <<8 x float>> [#uses=1]
  %22 = shufflevector <8 x float> %13, <8 x float> undef, <8 x i32> <i32 3, i32 3, i32 3, i32 3, i32 7, i32 7, i32 7, i32 7> ; <<8 x float>> [#uses=1]
  %23 = fmul <8 x float> %12, %22                 ; <<8 x float>> [#uses=1]
  %24 = fadd <8 x float> %21, %23                 ; <<8 x float>> [#uses=1]
  store <8 x float> %24, <8 x float> addrspace(1)* %7  ; pants
  %25 = tail call i32 @get_global_size(i32 0) nounwind ; <i32> [#uses=1]
  %26 = add i32 %25, %1                           ; <i32> [#uses=2]
  %27 = getelementptr inbounds <8 x float> addrspace(1)* %src, i32 %26 ; <<8 x float> addrspace(1)*> [#uses=1]
  %28 = load <8 x float> addrspace(1)* %27        ; <<8 x float>> [#uses=4]
  %29 = shufflevector <8 x float> %28, <8 x float> undef, <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 4, i32 4, i32 4, i32 4> ; <<8 x float>> [#uses=1]
  %30 = fmul <8 x float> %9, %29                  ; <<8 x float>> [#uses=1]
  %31 = shufflevector <8 x float> %28, <8 x float> undef, <8 x i32> <i32 1, i32 1, i32 1, i32 1, i32 5, i32 5, i32 5, i32 5> ; <<8 x float>> [#uses=1]
  %32 = fmul <8 x float> %10, %31                 ; <<8 x float>> [#uses=1]
  %33 = fadd <8 x float> %30, %32                 ; <<8 x float>> [#uses=1]
  %34 = shufflevector <8 x float> %28, <8 x float> undef, <8 x i32> <i32 2, i32 2, i32 2, i32 2, i32 6, i32 6, i32 6, i32 6> ; <<8 x float>> [#uses=1]
  %35 = fmul <8 x float> %11, %34                 ; <<8 x float>> [#uses=1]
  %36 = fadd <8 x float> %33, %35                 ; <<8 x float>> [#uses=1]
  %37 = shufflevector <8 x float> %28, <8 x float> undef, <8 x i32> <i32 3, i32 3, i32 3, i32 3, i32 7, i32 7, i32 7, i32 7> ; <<8 x float>> [#uses=1]
  %38 = fmul <8 x float> %12, %37                 ; <<8 x float>> [#uses=1]
  %39 = fadd <8 x float> %36, %38                 ; <<8 x float>> [#uses=1]
  %40 = getelementptr inbounds <8 x float> addrspace(1)* %dest, i32 %26 ; <<8 x float> addrspace(1)*> [#uses=1]
  store <8 x float> %39, <8 x float> addrspace(1)* %40
  %41 = add i32 %i.01, 1                          ; <i32> [#uses=2]
  %exitcond = icmp eq i32 %41, %Iterations        ; <i1> [#uses=1]
  br i1 %exitcond, label %._crit_edge, label %8

._crit_edge:                                      ; preds = %8, %0
  ret void
}

define void @sepia_OpenCL_float16_LRBNI(<16 x float> addrspace(1)* nocapture %src, <16 x float> addrspace(1)* nocapture %dest, <16 x float> addrspace(1)* nocapture %coef, i32 %size, i32 %image_height, i32 %Iterations) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=2]
  %2 = icmp eq i32 %Iterations, 0                 ; <i1> [#uses=1]
  br i1 %2, label %._crit_edge, label %bb.nph

bb.nph:                                           ; preds = %0
  %3 = getelementptr inbounds <16 x float> addrspace(1)* %coef, i32 1 ; <<16 x float> addrspace(1)*> [#uses=1]
  %4 = getelementptr inbounds <16 x float> addrspace(1)* %coef, i32 2 ; <<16 x float> addrspace(1)*> [#uses=1]
  %5 = getelementptr inbounds <16 x float> addrspace(1)* %coef, i32 3 ; <<16 x float> addrspace(1)*> [#uses=1]
  %6 = getelementptr inbounds <16 x float> addrspace(1)* %src, i32 %1 ; <<16 x float> addrspace(1)*> [#uses=1]
  %7 = getelementptr inbounds <16 x float> addrspace(1)* %dest, i32 %1 ; <<16 x float> addrspace(1)*> [#uses=1]
  br label %8

; <label>:8                                       ; preds = %8, %bb.nph
  %i.01 = phi i32 [ 0, %bb.nph ], [ %25, %8 ]     ; <i32> [#uses=1]
  %9 = load <16 x float> addrspace(1)* %coef      ; <<16 x float>> [#uses=1]
  %10 = load <16 x float> addrspace(1)* %3        ; <<16 x float>> [#uses=1]
  %11 = load <16 x float> addrspace(1)* %4        ; <<16 x float>> [#uses=1]
  %12 = load <16 x float> addrspace(1)* %5        ; <<16 x float>> [#uses=1]
  %13 = load <16 x float> addrspace(1)* %6        ; <<16 x float>> [#uses=4]
  %14 = shufflevector <16 x float> %13, <16 x float> undef, <16 x i32> <i32 0, i32 0, i32 0, i32 0, i32 4, i32 4, i32 4, i32 4, i32 8, i32 8, i32 8, i32 8, i32 12, i32 12, i32 12, i32 12> ; <<16 x float>> [#uses=1]
  %15 = fmul <16 x float> %9, %14                 ; <<16 x float>> [#uses=1]
  %16 = shufflevector <16 x float> %13, <16 x float> undef, <16 x i32> <i32 1, i32 1, i32 1, i32 1, i32 5, i32 5, i32 5, i32 5, i32 9, i32 9, i32 9, i32 9, i32 13, i32 13, i32 13, i32 13> ; <<16 x float>> [#uses=1]
  %17 = fmul <16 x float> %10, %16                ; <<16 x float>> [#uses=1]
  %18 = fadd <16 x float> %15, %17                ; <<16 x float>> [#uses=1]
  %19 = shufflevector <16 x float> %13, <16 x float> undef, <16 x i32> <i32 2, i32 2, i32 2, i32 2, i32 6, i32 6, i32 6, i32 6, i32 10, i32 10, i32 10, i32 10, i32 14, i32 14, i32 14, i32 14> ; <<16 x float>> [#uses=1]
  %20 = fmul <16 x float> %11, %19                ; <<16 x float>> [#uses=1]
  %21 = fadd <16 x float> %18, %20                ; <<16 x float>> [#uses=1]
  %22 = shufflevector <16 x float> %13, <16 x float> undef, <16 x i32> <i32 3, i32 3, i32 3, i32 3, i32 7, i32 7, i32 7, i32 7, i32 11, i32 11, i32 11, i32 11, i32 15, i32 15, i32 15, i32 15> ; <<16 x float>> [#uses=1]
  %23 = fmul <16 x float> %12, %22                ; <<16 x float>> [#uses=1]
  %24 = fadd <16 x float> %21, %23                ; <<16 x float>> [#uses=1]
  store <16 x float> %24, <16 x float> addrspace(1)* %7
  %25 = add i32 %i.01, 1                          ; <i32> [#uses=2]
  %exitcond = icmp eq i32 %25, %Iterations        ; <i1> [#uses=1]
  br i1 %exitcond, label %._crit_edge, label %8

._crit_edge:                                      ; preds = %8, %0
  ret void
}

