;*******************************************
;scalarazer test
;check  OpenCL isfinite function when it recieves global uchar constants vectors
;
;*******************************************


; RUN: opt  -runtimelib %p/../../Full/runtime.bc -std-compile-opts -inline-threshold=4096 -inline  -scalarize -verify %s -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'f_isfinite_dontvec.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

%opencl_metadata_type = type <{ i8*, i8*, [4 x i32], [4 x i32], i8*, i8* }>

@opencl_func_isfinite_locals = appending global [1 x i8*] zeroinitializer, section "llvm.metadata" ; <[1 x i8*]*> [#uses=1]
@opencl_func_isfinite_parameters = appending global [93 x i8] c"float const __attribute__((address_space(1))) *, float16 __attribute__((address_space(1))) *\00", section "llvm.metadata" ; <[93 x i8]*> [#uses=1]
@opencl_metadata = appending global [1 x %opencl_metadata_type] [%opencl_metadata_type <{ i8* bitcast (void (float addrspace(1)*, <16 x float> addrspace(1)*)* @func_isfinite to i8*), i8* null, [4 x i32] zeroinitializer, [4 x i32] zeroinitializer, i8* bitcast ([1 x i8*]* @opencl_func_isfinite_locals to i8*), i8* getelementptr inbounds ([93 x i8]* @opencl_func_isfinite_parameters, i32 0, i32 0) }>], section "llvm.metadata" ; <[1 x %opencl_metadata_type]*> [#uses=0]

define void @func_isfinite(float addrspace(1)* nocapture %in, <16 x float> addrspace(1)* nocapture %out) nounwind {
  %1 = tail call i32 @get_global_id(i32 0) nounwind ; <i32> [#uses=1]
  %2 = tail call <16 x i32> @_Z8isfiniteU8__vector16f(<16 x float> <float 0xC0D6DD6960000000, float 0x40810AB860000000, float 0x40080624E0000000, float 0x407A763680000000, float 0.000000e+000, float 0xC021CCCCC0000000, float -8.000000e+000, float 0x40A17CDE80000000, float 0x40E5047F60000000, float 0xBFD99999A0000000, float 0.000000e+000, float 4.000000e+000, float 0x410C8D6FE0000000, float 0x4074063D80000000, float 0x3F80624DE0000000, float 0x40622FAE20000000>) nounwind ; <<16 x i32>> [#uses=1]
  %3 = tail call <16 x float> @_Z15convert_float16U8__vector16i(<16 x i32> %2) nounwind ; <<16 x float>> [#uses=1]
  %4 = getelementptr inbounds <16 x float> addrspace(1)* %out, i32 %1 ; <<16 x float> addrspace(1)*> [#uses=1]
  store <16 x float> %3, <16 x float> addrspace(1)* %4
  ret void
}

declare i32 @get_global_id(i32)

declare <16 x float> @_Z15convert_float16U8__vector16i(<16 x i32>)

declare <16 x i32> @_Z8isfiniteU8__vector16f(<16 x float>)

;CHECK: @_Z8isfiniteU8__vector16f
;CHECK: ret