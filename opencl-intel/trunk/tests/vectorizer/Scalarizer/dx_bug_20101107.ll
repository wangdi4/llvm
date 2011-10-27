; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/dxruntime.bc -scalarize -verify %t.bc -S -o %t1.ll -runtime=dx
; RUN: FileCheck %s --input-file=%t1.ll

;CHECK: dx_soa_store
;CHECK-NOT: undef
;CHECK: void

define fastcc void @aos_shader(i8* noalias nocapture %gc, i8* noalias nocapture %lc) nounwind {
entry:
  %load_call = call <4 x float> @dx_soa_load_constant_uniform_imm_1_float4_vs(i8* %gc, i32 2, i32 8) nounwind
  %load_call1 = call <4 x float> @dx_soa_load_constant_uniform_imm_1_float4_vs(i8* %gc, i32 2, i32 7) nounwind
  %0 = fmul <4 x float> %load_call, %load_call1
  %1 = extractelement <4 x float> %0, i32 0
  %2 = fadd float %1, 0.000000e+000
  %3 = extractelement <4 x float> %0, i32 1
  %4 = fadd float %2, %3
  %5 = insertelement <4 x float> undef, float %4, i32 0
  %6 = insertelement <4 x float> %5, float %4, i32 1
  %7 = insertelement <4 x float> %6, float %4, i32 2
  %8 = insertelement <4 x float> %7, float %4, i32 3
  call void @dx_soa_store_output_uniform_indirect_1_float4_vs(i8* %gc, i32 0, i32 0, <4 x float> %8) nounwind
  ret void
}



  declare <4 x float> @dx_soa_load_constant_uniform_imm_1_float4_vs(i8*, i32 , i32 ) nounwind;
  declare  <4 x float> @dx_soa_load_constant_uniform_imm_1_float4_vs(i8*, i32 , i32 ) nounwind;
  declare void @dx_soa_store_output_uniform_indirect_1_float4_vs(i8* , i32 , i32 , <4 x float> ) nounwind;
