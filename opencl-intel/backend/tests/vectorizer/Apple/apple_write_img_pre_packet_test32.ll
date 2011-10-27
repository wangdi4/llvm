
; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/apple_only_dcls32.ll -runtime=apple -AppleWIDepPrePack %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'c:\work\write_img_pre_packet_test.ll'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

%0 = type opaque
%struct._image2d_t = type opaque

declare i32 @get_global_id(i32) nounwind


; CHECK: @__write_imagef_2d_x_cons_y_uni_test
; CHECK-NOT: @__write_imagef_2d
; CHECK: @_f_v.__write_imagef_2d
; CHECK-NOT: @__write_imagef_2d
; CHECK: ret void
define void @__write_imagef_2d_x_cons_y_uni_test(%struct._image2d_t addrspace(1)* %img, <4 x float>* nocapture %colors, i32 %ycrd) nounwind {
entry:
  %ind = tail call i32 @get_global_id(i32 0)
  %0 = sext i32 %ind to i64
  %colors_ptr = getelementptr <4 x float>* %colors, i64 %0
  %colors_val = load <4 x float>* %colors_ptr, align 16
  %scalar = extractelement <4 x float> %colors_val, i32 0
  %scalar1 = extractelement <4 x float> %colors_val, i32 1
  %scalar2 = extractelement <4 x float> %colors_val, i32 2
  %scalar3 = extractelement <4 x float> %colors_val, i32 3
  %bitcast.opaque.ptr = bitcast %struct._image2d_t addrspace(1)* %img to %0 addrspace(1)*
  %scalar_arg_vector = insertelement <4 x float> undef, float %scalar, i32 0
  %scalar_arg_vector4 = insertelement <4 x float> %scalar_arg_vector, float %scalar1, i32 1
  %scalar_arg_vector5 = insertelement <4 x float> %scalar_arg_vector4, float %scalar2, i32 2
  %scalar_arg_vector6 = insertelement <4 x float> %scalar_arg_vector5, float %scalar3, i32 3
  tail call void @_f_v.__write_imagef_2d(%0 addrspace(1)* %bitcast.opaque.ptr, i32 %ind, i32 %ycrd, <4 x float> %scalar_arg_vector6)
  ret void
}


; CHECK: @__write_imagef_2d_x_uni_y_cons_test
; CHECK-NOT: @_f_v.__write_imagef_2d
; CHECK: @__write_imagef_2d
; CHECK-NOT: @_f_v.__write_imagef_2d
; CHECK: ret void
define void @__write_imagef_2d_x_uni_y_cons_test(%struct._image2d_t addrspace(1)* %img, <4 x float>* nocapture %colors, i32 %ycrd) nounwind {
entry:
  %ind = tail call i32 @get_global_id(i32 0)
  %0 = sext i32 %ind to i64
  %colors_ptr = getelementptr <4 x float>* %colors, i64 %0
  %colors_val = load <4 x float>* %colors_ptr, align 16
  %scalar = extractelement <4 x float> %colors_val, i32 0
  %scalar1 = extractelement <4 x float> %colors_val, i32 1
  %scalar2 = extractelement <4 x float> %colors_val, i32 2
  %scalar3 = extractelement <4 x float> %colors_val, i32 3
  %bitcast.opaque.ptr = bitcast %struct._image2d_t addrspace(1)* %img to %0 addrspace(1)*
  %scalar_arg_vector = insertelement <4 x float> undef, float %scalar, i32 0
  %scalar_arg_vector4 = insertelement <4 x float> %scalar_arg_vector, float %scalar1, i32 1
  %scalar_arg_vector5 = insertelement <4 x float> %scalar_arg_vector4, float %scalar2, i32 2
  %scalar_arg_vector6 = insertelement <4 x float> %scalar_arg_vector5, float %scalar3, i32 3
  tail call void @_f_v.__write_imagef_2d(%0 addrspace(1)* %bitcast.opaque.ptr, i32 %ycrd, i32 %ind, <4 x float> %scalar_arg_vector6)
  ret void
}

declare void @__write_imagef_2d(%struct._image2d_t addrspace(1)* %image, <4 x i32>, <4 x float>) nounwind 

declare void @_f_v.__write_imagef_2d(%0 addrspace(1)*, i32, i32, <4 x float>) nounwind

declare i1 @allOne(i1)

declare i1 @allZero(i1)
