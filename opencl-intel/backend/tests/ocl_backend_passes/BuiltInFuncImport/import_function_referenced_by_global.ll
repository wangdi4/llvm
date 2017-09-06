; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -runtimelib=%t.rtl.bc -builtin-import -verify %s -S | FileCheck %s

;*********************************************************************************
; When importing we need to fully explore call tree for functions referenced
; in global variables.
;*********************************************************************************

; ModuleID = 'main'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32-gnu-elf"

%opencl.image2d_ro_t = type opaque

; CHECK: @coord_translate_i_callback = constant [2 x <4 x i32> (i8*, <4 x i32>)*] [<4 x i32> (i8*, <4 x i32>)* @_Z41trans_coord_int_CLAMPTOEDGE_FALSE_NEARESTPvDv4_i, <4 x i32> (i8*, <4 x i32>)* @_Z25trans_coord_int_UNDEFINEDPvDv4_i], align 16

; Function Attrs: nounwind readonly
declare i8* @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t addrspace(1)*, i32, <2 x i32>)


; Function Attrs: nounwind
define i8* @test_bgra8888(%opencl.image2d_ro_t addrspace(1)* %srcimg, i32 %sampler, <2 x i32> %vecinit4) {
entry:
  %call = tail call i8* @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i(%opencl.image2d_ro_t addrspace(1)* %srcimg, i32 %sampler, <2 x i32> %vecinit4) 
  ret i8*  %call
}

; CHECK: define i8* @test_bgra8888
; CHECK-DAG: define linkonce_odr i8* @_Z11read_imagef14ocl_image2d_ro11ocl_samplerDv2_i
; CHECK-DAG: define linkonce_odr i8* @call_coord_translate_i_callback
; CHECK-DAG: define linkonce_odr <4 x i32> @_Z41trans_coord_int_CLAMPTOEDGE_FALSE_NEARESTPvDv4_i
; CHECK-DAG: define linkonce_odr <4 x i32> @_Z25trans_coord_int_UNDEFINEDPvDv4_i
; CHECK-DAG: define linkonce_odr <4 x i32> @_Z3maxDv4_iS_
; CHECK-DAG: define linkonce_odr <4 x i32> @_Z3minDv4_iS_
