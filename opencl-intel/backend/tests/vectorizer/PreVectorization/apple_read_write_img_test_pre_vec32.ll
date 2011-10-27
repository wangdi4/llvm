
; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -runtime=apple -CLBltnPreVec %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%struct._image2d_t = type opaque
%struct._image3d_t = type opaque


; CHECK: @read_2d_ff_test
; CHECK-NOT: read_2d_ff
; CHECK: _f_v.read_2d_ff
; CHECK-NOT: read_2d_ff
; CHECK: ret void
define void @read_2d_ff_test(%struct._image2d_t addrspace(1)* %img, i32 %smp, <2 x float>* %crd, <4 x float>* %dst) nounwind {
entry:
  %ind = call i32 @get_global_id(i32 0)
  %src_ptr = getelementptr  <2 x float>* %crd, i32 %ind
  %src_val = load <2 x float>* %src_ptr 
  %src_extend = shufflevector <2 x float> %src_val, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %x = call <4 x float> @read_2d_ff(%struct._image2d_t addrspace(1)* %img, i32 %smp, <4 x float> %src_extend) 
  %dst_ptr = getelementptr  <4 x float>* %dst, i32 %ind
  store <4 x float> %x, <4 x float>* %dst_ptr 
  ret void
}


; CHECK: @read_3d_ff_test
; CHECK-NOT: read_3d_ff
; CHECK: _f_v.read_3d_ff
; CHECK-NOT: read_3d_ff
; CHECK: ret void
define void @read_3d_ff_test(%struct._image3d_t addrspace(1)* %img, i32 %smp, <3 x float>* %crd, <4 x float>* %dst) nounwind {
entry:
  %ind = call i32 @get_global_id(i32 0)
  %src_ptr = getelementptr  <3 x float>* %crd, i32 %ind
  %src_val = load <3 x float>* %src_ptr 
  %src_extend = shufflevector <3 x float> %src_val, <3 x float> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 undef>
  %x = call <4 x float> @read_3d_ff(%struct._image3d_t addrspace(1)* %img, i32 %smp, <4 x float> %src_extend) 
  %dst_ptr = getelementptr  <4 x float>* %dst, i32 %ind
  store <4 x float> %x, <4 x float>* %dst_ptr 
  ret void
}


; CHECK: @__write_imagef_2d_test
; CHECK-NOT: __write_imagef_2d
; CHECK: _f_v.__write_imagef_2d
; CHECK-NOT: __write_imagef_2d
; CHECK: ret void
define void @__write_imagef_2d_test(%struct._image2d_t addrspace(1)* %img, <4 x float>* %colors, i32 %ycrd) nounwind {
entry:
  %ind = call i32 @get_global_id(i32 0)
  %crdvec_x = insertelement <2 x i32> undef, i32 %ind, i32 0
  %crdvec_xy = insertelement <2 x i32> %crdvec_x, i32 %ycrd, i32 1
  %crdvec_xy_extend = shufflevector <2 x i32> %crdvec_xy, <2 x i32> undef, <4 x i32> <i32 0, i32 1, i32 undef, i32 undef>
  %colors_ptr = getelementptr  <4 x float>* %colors, i32 %ind
  %colors_val = load <4 x float>* %colors_ptr 
  call void @__write_imagef_2d(%struct._image2d_t addrspace(1)* %img, <4 x i32> %crdvec_xy_extend, <4 x float> %colors_val)
  ret void
}




declare i32 @get_global_id(i32 %dimindx) nounwind 
declare <4 x float> @read_2d_ff(%struct._image2d_t addrspace(1)* %image, i32 %sampler, <4 x float>) nounwind 
declare <4 x float> @read_3d_ff(%struct._image3d_t addrspace(1)* %image, i32 %sampler, <4 x float>) nounwind 
declare void @__write_imagef_2d(%struct._image2d_t addrspace(1)* %image, <4 x i32>, <4 x float>) nounwind 
