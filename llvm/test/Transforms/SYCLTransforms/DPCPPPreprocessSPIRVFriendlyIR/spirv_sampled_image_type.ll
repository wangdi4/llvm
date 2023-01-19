; RUN: opt -passes='dpcpp-kernel-preprocess-spv-ir' -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='dpcpp-kernel-preprocess-spv-ir' -S %s | FileCheck %s

; Extended basing on clang/test/CodeGenOpenCL/sampled_image.cl
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Readonly types
%spirv.SampledImage.image1d_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image1d_ro_t
; CHECK: %spirv.SampledImage._void_1_0_0_0_1_0_0 = type opaque
%spirv.SampledImage.image1d_array_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image1d_array_ro_t
; CHECK: %spirv.SampledImage._void_1_0_1_0_1_0_0 = type opaque
%spirv.SampledImage.image1d_buffer_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image1d_buffer_ro_t
; CHECK: %spirv.SampledImage._void_1_0_0_0_2_0_0 = type opaque
%spirv.SampledImage.image2d_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_ro_t
; CHECK: %spirv.SampledImage._void_2_0_0_0_1_0_0 = type opaque
%spirv.SampledImage.image2d_array_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_ro_t
; CHECK: %spirv.SampledImage._void_2_0_1_0_1_0_0 = type opaque
%spirv.SampledImage.image2d_depth_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_depth_ro_t
; CHECK: %spirv.SampledImage._void_2_1_0_0_1_0_0 = type opaque
%spirv.SampledImage.image2d_array_depth_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_depth_ro_t
; CHECK: %spirv.SampledImage._void_2_1_1_0_1_0_0 = type opaque
%spirv.SampledImage.image2d_msaa_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_msaa_ro_t
; CHECK: %spirv.SampledImage._void_2_0_0_1_1_0_0 = type opaque
%spirv.SampledImage.image2d_array_msaa_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_msaa_ro_t
; CHECK: %spirv.SampledImage._void_2_0_1_1_1_0_0 = type opaque
%spirv.SampledImage.image2d_msaa_depth_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_msaa_depth_ro_t
; CHECK: %spirv.SampledImage._void_2_1_0_1_1_0_0 = type opaque
%spirv.SampledImage.image2d_array_msaa_depth_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_msaa_depth_ro_t
; CHECK: %spirv.SampledImage._void_2_1_1_1_1_0_0 = type opaque
%spirv.SampledImage.image3d_ro_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image3d_ro_t
; CHECK: %spirv.SampledImage._void_3_0_0_0_1_0_0 = type opaque

; Writeonly types
%spirv.SampledImage.image1d_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image1d_wo_t
; CHECK: %spirv.SampledImage._void_1_0_0_0_1_0_1 = type opaque
%spirv.SampledImage.image1d_array_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image1d_array_wo_t
; CHECK: %spirv.SampledImage._void_1_0_1_0_1_0_1 = type opaque
%spirv.SampledImage.image1d_buffer_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image1d_buffer_wo_t
; CHECK: %spirv.SampledImage._void_1_0_0_0_2_0_1 = type opaque
%spirv.SampledImage.image2d_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_wo_t
; CHECK: %spirv.SampledImage._void_2_0_0_0_1_0_1 = type opaque
%spirv.SampledImage.image2d_array_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_wo_t
; CHECK: %spirv.SampledImage._void_2_0_1_0_1_0_1 = type opaque
%spirv.SampledImage.image2d_depth_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_depth_wo_t
; CHECK: %spirv.SampledImage._void_2_1_0_0_1_0_1 = type opaque
%spirv.SampledImage.image2d_array_depth_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_depth_wo_t
; CHECK: %spirv.SampledImage._void_2_1_1_0_1_0_1 = type opaque
%spirv.SampledImage.image2d_msaa_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_msaa_wo_t
; CHECK: %spirv.SampledImage._void_2_0_0_1_1_0_1 = type opaque
%spirv.SampledImage.image2d_array_msaa_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_msaa_wo_t
; CHECK: %spirv.SampledImage._void_2_0_1_1_1_0_1 = type opaque
%spirv.SampledImage.image2d_msaa_depth_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_msaa_depth_wo_t
; CHECK: %spirv.SampledImage._void_2_1_0_1_1_0_1 = type opaque
%spirv.SampledImage.image2d_array_msaa_depth_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_msaa_depth_wo_t
; CHECK: %spirv.SampledImage._void_2_1_1_1_1_0_1 = type opaque
%spirv.SampledImage.image3d_wo_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image3d_wo_t
; CHECK: %spirv.SampledImage._void_3_0_0_0_1_0_1 = type opaque

; Readwrite types
%spirv.SampledImage.image1d_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image1d_rw_t
; CHECK: %spirv.SampledImage._void_1_0_0_0_1_0_2 = type opaque
%spirv.SampledImage.image1d_array_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image1d_array_rw_t
; CHECK: %spirv.SampledImage._void_1_0_1_0_1_0_2 = type opaque
%spirv.SampledImage.image1d_buffer_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image1d_buffer_rw_t
; CHECK: %spirv.SampledImage._void_1_0_0_0_2_0_2 = type opaque
%spirv.SampledImage.image2d_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_rw_t
; CHECK: %spirv.SampledImage._void_2_0_0_0_1_0_2 = type opaque
%spirv.SampledImage.image2d_array_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_rw_t
; CHECK: %spirv.SampledImage._void_2_0_1_0_1_0_2 = type opaque
%spirv.SampledImage.image2d_depth_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_depth_rw_t
; CHECK: %spirv.SampledImage._void_2_1_0_0_1_0_2 = type opaque
%spirv.SampledImage.image2d_array_depth_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_depth_rw_t
; CHECK: %spirv.SampledImage._void_2_1_1_0_1_0_2 = type opaque
%spirv.SampledImage.image2d_msaa_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_msaa_rw_t
; CHECK: %spirv.SampledImage._void_2_0_0_1_1_0_2 = type opaque
%spirv.SampledImage.image2d_array_msaa_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_msaa_rw_t
; CHECK: %spirv.SampledImage._void_2_0_1_1_1_0_2 = type opaque
%spirv.SampledImage.image2d_msaa_depth_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_msaa_depth_rw_t
; CHECK: %spirv.SampledImage._void_2_1_0_1_1_0_2 = type opaque
%spirv.SampledImage.image2d_array_msaa_depth_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image2d_array_msaa_depth_rw_t
; CHECK: %spirv.SampledImage._void_2_1_1_1_1_0_2 = type opaque
%spirv.SampledImage.image3d_rw_t = type opaque
; CHECK-NOT: %spirv.SampledImage.image3d_rw_t
; CHECK: %spirv.SampledImage._void_3_0_0_0_1_0_2 = type opaque

; The function names are also remangled to conform with SPV-IR SPEC.
define dso_local void @test_read_image() {
entry:
  call void @_Z13my_read_image32__spirv_SampledImage__image1d_ro(%spirv.SampledImage.image1d_ro_t* undef)
; CHECK-NOT: _Z13my_read_image32__spirv_SampledImage__image1d_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_1_0_0_0_1_0_0(%spirv.SampledImage._void_1_0_0_0_1_0_0
  call void @_Z13my_read_image38__spirv_SampledImage__image1d_array_ro(%spirv.SampledImage.image1d_array_ro_t* undef)
; CHECK-NOT: _Z13my_read_image38__spirv_SampledImage__image1d_array_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_1_0_1_0_1_0_0(%spirv.SampledImage._void_1_0_1_0_1_0_0
  call void @_Z13my_read_image39__spirv_SampledImage__image1d_buffer_ro(%spirv.SampledImage.image1d_buffer_ro_t* undef)
; CHECK-NOT: _Z13my_read_image39__spirv_SampledImage__image1d_buffer_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_1_0_0_0_2_0_0(%spirv.SampledImage._void_1_0_0_0_2_0_0
  call void @_Z13my_read_image32__spirv_SampledImage__image2d_ro(%spirv.SampledImage.image2d_ro_t* undef)
; CHECK-NOT: _Z13my_read_image32__spirv_SampledImage__image2d_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_2_0_0_0_1_0_0(%spirv.SampledImage._void_2_0_0_0_1_0_0
  call void @_Z13my_read_image38__spirv_SampledImage__image2d_array_ro(%spirv.SampledImage.image2d_array_ro_t* undef)
; CHECK-NOT: _Z13my_read_image38__spirv_SampledImage__image2d_array_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_2_0_1_0_1_0_0(%spirv.SampledImage._void_2_0_1_0_1_0_0
  call void @_Z13my_read_image38__spirv_SampledImage__image2d_depth_ro(%spirv.SampledImage.image2d_depth_ro_t* undef)
; CHECK-NOT: _Z13my_read_image38__spirv_SampledImage__image2d_depth_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_2_1_0_0_1_0_0(%spirv.SampledImage._void_2_1_0_0_1_0_0
  call void @_Z13my_read_image44__spirv_SampledImage__image2d_array_depth_ro(%spirv.SampledImage.image2d_array_depth_ro_t* undef)
; CHECK-NOT: _Z13my_read_image44__spirv_SampledImage__image2d_array_depth_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_2_1_1_0_1_0_0(%spirv.SampledImage._void_2_1_1_0_1_0_0
  call void @_Z13my_read_image37__spirv_SampledImage__image2d_msaa_ro(%spirv.SampledImage.image2d_msaa_ro_t* undef)
; CHECK-NOT: _Z13my_read_image37__spirv_SampledImage__image2d_msaa_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_2_0_0_1_1_0_0(%spirv.SampledImage._void_2_0_0_1_1_0_0
  call void @_Z13my_read_image43__spirv_SampledImage__image2d_array_msaa_ro(%spirv.SampledImage.image2d_array_msaa_ro_t* undef)
; CHECK-NOT: _Z13my_read_image43__spirv_SampledImage__image2d_array_msaa_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_2_0_1_1_1_0_0(%spirv.SampledImage._void_2_0_1_1_1_0_0
  call void @_Z13my_read_image43__spirv_SampledImage__image2d_msaa_depth_ro(%spirv.SampledImage.image2d_msaa_depth_ro_t* undef)
; CHECK-NOT: _Z13my_read_image43__spirv_SampledImage__image2d_msaa_depth_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_2_1_0_1_1_0_0(%spirv.SampledImage._void_2_1_0_1_1_0_0
  call void @_Z13my_read_image49__spirv_SampledImage__image2d_array_msaa_depth_ro(%spirv.SampledImage.image2d_array_msaa_depth_ro_t* undef)
; CHECK-NOT: _Z13my_read_image49__spirv_SampledImage__image2d_array_msaa_depth_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_2_1_1_1_1_0_0(%spirv.SampledImage._void_2_1_1_1_1_0_0
  call void @_Z13my_read_image32__spirv_SampledImage__image3d_ro(%spirv.SampledImage.image3d_ro_t* undef)
; CHECK-NOT: _Z13my_read_image32__spirv_SampledImage__image3d_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_3_0_0_0_1_0_0(%spirv.SampledImage._void_3_0_0_0_1_0_0
  ret void
}

declare void @_Z13my_read_image32__spirv_SampledImage__image1d_ro(%spirv.SampledImage.image1d_ro_t*)
declare void @_Z13my_read_image38__spirv_SampledImage__image1d_array_ro(%spirv.SampledImage.image1d_array_ro_t*)
declare void @_Z13my_read_image39__spirv_SampledImage__image1d_buffer_ro(%spirv.SampledImage.image1d_buffer_ro_t*)
declare void @_Z13my_read_image32__spirv_SampledImage__image2d_ro(%spirv.SampledImage.image2d_ro_t*)
declare void @_Z13my_read_image38__spirv_SampledImage__image2d_array_ro(%spirv.SampledImage.image2d_array_ro_t*)
declare void @_Z13my_read_image38__spirv_SampledImage__image2d_depth_ro(%spirv.SampledImage.image2d_depth_ro_t*)
declare void @_Z13my_read_image44__spirv_SampledImage__image2d_array_depth_ro(%spirv.SampledImage.image2d_array_depth_ro_t*)
declare void @_Z13my_read_image37__spirv_SampledImage__image2d_msaa_ro(%spirv.SampledImage.image2d_msaa_ro_t*)
declare void @_Z13my_read_image43__spirv_SampledImage__image2d_array_msaa_ro(%spirv.SampledImage.image2d_array_msaa_ro_t*)
declare void @_Z13my_read_image43__spirv_SampledImage__image2d_msaa_depth_ro(%spirv.SampledImage.image2d_msaa_depth_ro_t*)
declare void @_Z13my_read_image49__spirv_SampledImage__image2d_array_msaa_depth_ro(%spirv.SampledImage.image2d_array_msaa_depth_ro_t*)
declare void @_Z13my_read_image32__spirv_SampledImage__image3d_ro(%spirv.SampledImage.image3d_ro_t*)

define dso_local void @test_write_image() {
entry:
  call void @_Z14my_write_image32__spirv_SampledImage__image1d_wo(%spirv.SampledImage.image1d_wo_t* undef)
; CHECK-NOT: _Z14my_write_image32__spirv_SampledImage__image1d_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_1_0_0_0_1_0_1(%spirv.SampledImage._void_1_0_0_0_1_0_1
  call void @_Z14my_write_image38__spirv_SampledImage__image1d_array_wo(%spirv.SampledImage.image1d_array_wo_t* undef)
; CHECK-NOT: _Z14my_write_image38__spirv_SampledImage__image1d_array_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_1_0_1_0_1_0_1(%spirv.SampledImage._void_1_0_1_0_1_0_1
  call void @_Z14my_write_image39__spirv_SampledImage__image1d_buffer_wo(%spirv.SampledImage.image1d_buffer_wo_t* undef)
; CHECK-NOT: _Z14my_write_image39__spirv_SampledImage__image1d_buffer_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_1_0_0_0_2_0_1(%spirv.SampledImage._void_1_0_0_0_2_0_1
  call void @_Z14my_write_image32__spirv_SampledImage__image2d_wo(%spirv.SampledImage.image2d_wo_t* undef)
; CHECK-NOT: _Z14my_write_image32__spirv_SampledImage__image2d_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_2_0_0_0_1_0_1(%spirv.SampledImage._void_2_0_0_0_1_0_1
  call void @_Z14my_write_image38__spirv_SampledImage__image2d_array_wo(%spirv.SampledImage.image2d_array_wo_t* undef)
; CHECK-NOT: _Z14my_write_image38__spirv_SampledImage__image2d_array_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_2_0_1_0_1_0_1(%spirv.SampledImage._void_2_0_1_0_1_0_1
  call void @_Z14my_write_image38__spirv_SampledImage__image2d_depth_wo(%spirv.SampledImage.image2d_depth_wo_t* undef)
; CHECK-NOT: _Z14my_write_image38__spirv_SampledImage__image2d_depth_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_2_1_0_0_1_0_1(%spirv.SampledImage._void_2_1_0_0_1_0_1
  call void @_Z14my_write_image44__spirv_SampledImage__image2d_array_depth_wo(%spirv.SampledImage.image2d_array_depth_wo_t* undef)
; CHECK-NOT: _Z14my_write_image44__spirv_SampledImage__image2d_array_depth_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_2_1_1_0_1_0_1(%spirv.SampledImage._void_2_1_1_0_1_0_1
  call void @_Z14my_write_image37__spirv_SampledImage__image2d_msaa_wo(%spirv.SampledImage.image2d_msaa_wo_t* undef)
; CHECK-NOT: _Z14my_write_image37__spirv_SampledImage__image2d_msaa_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_2_0_0_1_1_0_1(%spirv.SampledImage._void_2_0_0_1_1_0_1
  call void @_Z14my_write_image43__spirv_SampledImage__image2d_array_msaa_wo(%spirv.SampledImage.image2d_array_msaa_wo_t* undef)
; CHECK-NOT: _Z14my_write_image43__spirv_SampledImage__image2d_array_msaa_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_2_0_1_1_1_0_1(%spirv.SampledImage._void_2_0_1_1_1_0_1
  call void @_Z14my_write_image43__spirv_SampledImage__image2d_msaa_depth_wo(%spirv.SampledImage.image2d_msaa_depth_wo_t* undef)
; CHECK-NOT: _Z14my_write_image43__spirv_SampledImage__image2d_msaa_depth_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_2_1_0_1_1_0_1(%spirv.SampledImage._void_2_1_0_1_1_0_1
  call void @_Z14my_write_image49__spirv_SampledImage__image2d_array_msaa_depth_wo(%spirv.SampledImage.image2d_array_msaa_depth_wo_t* undef)
; CHECK-NOT: _Z14my_write_image49__spirv_SampledImage__image2d_array_msaa_depth_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_2_1_1_1_1_0_1(%spirv.SampledImage._void_2_1_1_1_1_0_1
  call void @_Z14my_write_image32__spirv_SampledImage__image3d_wo(%spirv.SampledImage.image3d_wo_t* undef)
; CHECK-NOT: _Z14my_write_image32__spirv_SampledImage__image3d_wo
; CHECK: _Z14my_write_image40__spirv_SampledImage__void_3_0_0_0_1_0_1(%spirv.SampledImage._void_3_0_0_0_1_0_1
  ret void
}

declare void @_Z14my_write_image32__spirv_SampledImage__image1d_wo(%spirv.SampledImage.image1d_wo_t*)
declare void @_Z14my_write_image38__spirv_SampledImage__image1d_array_wo(%spirv.SampledImage.image1d_array_wo_t*)
declare void @_Z14my_write_image39__spirv_SampledImage__image1d_buffer_wo(%spirv.SampledImage.image1d_buffer_wo_t*)
declare void @_Z14my_write_image32__spirv_SampledImage__image2d_wo(%spirv.SampledImage.image2d_wo_t*)
declare void @_Z14my_write_image38__spirv_SampledImage__image2d_array_wo(%spirv.SampledImage.image2d_array_wo_t*)
declare void @_Z14my_write_image38__spirv_SampledImage__image2d_depth_wo(%spirv.SampledImage.image2d_depth_wo_t*)
declare void @_Z14my_write_image44__spirv_SampledImage__image2d_array_depth_wo(%spirv.SampledImage.image2d_array_depth_wo_t*)
declare void @_Z14my_write_image37__spirv_SampledImage__image2d_msaa_wo(%spirv.SampledImage.image2d_msaa_wo_t*)
declare void @_Z14my_write_image43__spirv_SampledImage__image2d_array_msaa_wo(%spirv.SampledImage.image2d_array_msaa_wo_t*)
declare void @_Z14my_write_image43__spirv_SampledImage__image2d_msaa_depth_wo(%spirv.SampledImage.image2d_msaa_depth_wo_t*)
declare void @_Z14my_write_image49__spirv_SampledImage__image2d_array_msaa_depth_wo(%spirv.SampledImage.image2d_array_msaa_depth_wo_t*)
declare void @_Z14my_write_image32__spirv_SampledImage__image3d_wo(%spirv.SampledImage.image3d_wo_t*)

define dso_local void @test_readwrite_image() {
entry:
  call void @_Z18my_readwrite_image32__spirv_SampledImage__image1d_rw(%spirv.SampledImage.image1d_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image32__spirv_SampledImage__image1d_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_1_0_0_0_1_0_2(%spirv.SampledImage._void_1_0_0_0_1_0_2
  call void @_Z18my_readwrite_image38__spirv_SampledImage__image1d_array_rw(%spirv.SampledImage.image1d_array_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image38__spirv_SampledImage__image1d_array_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_1_0_1_0_1_0_2(%spirv.SampledImage._void_1_0_1_0_1_0_2
  call void @_Z18my_readwrite_image39__spirv_SampledImage__image1d_buffer_rw(%spirv.SampledImage.image1d_buffer_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image39__spirv_SampledImage__image1d_buffer_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_1_0_0_0_2_0_2(%spirv.SampledImage._void_1_0_0_0_2_0_2
  call void @_Z18my_readwrite_image32__spirv_SampledImage__image2d_rw(%spirv.SampledImage.image2d_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image32__spirv_SampledImage__image2d_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_2_0_0_0_1_0_2(%spirv.SampledImage._void_2_0_0_0_1_0_2
  call void @_Z18my_readwrite_image38__spirv_SampledImage__image2d_array_rw(%spirv.SampledImage.image2d_array_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image38__spirv_SampledImage__image2d_array_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_2_0_1_0_1_0_2(%spirv.SampledImage._void_2_0_1_0_1_0_2
  call void @_Z18my_readwrite_image38__spirv_SampledImage__image2d_depth_rw(%spirv.SampledImage.image2d_depth_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image38__spirv_SampledImage__image2d_depth_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_2_1_0_0_1_0_2(%spirv.SampledImage._void_2_1_0_0_1_0_2
  call void @_Z18my_readwrite_image44__spirv_SampledImage__image2d_array_depth_rw(%spirv.SampledImage.image2d_array_depth_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image44__spirv_SampledImage__image2d_array_depth_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_2_1_1_0_1_0_2(%spirv.SampledImage._void_2_1_1_0_1_0_2
  call void @_Z18my_readwrite_image37__spirv_SampledImage__image2d_msaa_rw(%spirv.SampledImage.image2d_msaa_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image37__spirv_SampledImage__image2d_msaa_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_2_0_0_1_1_0_2(%spirv.SampledImage._void_2_0_0_1_1_0_2
  call void @_Z18my_readwrite_image43__spirv_SampledImage__image2d_array_msaa_rw(%spirv.SampledImage.image2d_array_msaa_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image43__spirv_SampledImage__image2d_array_msaa_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_2_0_1_1_1_0_2(%spirv.SampledImage._void_2_0_1_1_1_0_2
  call void @_Z18my_readwrite_image43__spirv_SampledImage__image2d_msaa_depth_rw(%spirv.SampledImage.image2d_msaa_depth_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image43__spirv_SampledImage__image2d_msaa_depth_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_2_1_0_1_1_0_2(%spirv.SampledImage._void_2_1_0_1_1_0_2
  call void @_Z18my_readwrite_image49__spirv_SampledImage__image2d_array_msaa_depth_rw(%spirv.SampledImage.image2d_array_msaa_depth_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image49__spirv_SampledImage__image2d_array_msaa_depth_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_2_1_1_1_1_0_2(%spirv.SampledImage._void_2_1_1_1_1_0_2
  call void @_Z18my_readwrite_image32__spirv_SampledImage__image3d_rw(%spirv.SampledImage.image3d_rw_t* undef)
; CHECK-NOT: _Z18my_readwrite_image32__spirv_SampledImage__image3d_rw
; CHECK: _Z18my_readwrite_image40__spirv_SampledImage__void_3_0_0_0_1_0_2(%spirv.SampledImage._void_3_0_0_0_1_0_2
  ret void
}

declare void @_Z18my_readwrite_image32__spirv_SampledImage__image1d_rw(%spirv.SampledImage.image1d_rw_t*)
declare void @_Z18my_readwrite_image38__spirv_SampledImage__image1d_array_rw(%spirv.SampledImage.image1d_array_rw_t*)
declare void @_Z18my_readwrite_image39__spirv_SampledImage__image1d_buffer_rw(%spirv.SampledImage.image1d_buffer_rw_t*)
declare void @_Z18my_readwrite_image32__spirv_SampledImage__image2d_rw(%spirv.SampledImage.image2d_rw_t*)
declare void @_Z18my_readwrite_image38__spirv_SampledImage__image2d_array_rw(%spirv.SampledImage.image2d_array_rw_t*)
declare void @_Z18my_readwrite_image38__spirv_SampledImage__image2d_depth_rw(%spirv.SampledImage.image2d_depth_rw_t*)
declare void @_Z18my_readwrite_image44__spirv_SampledImage__image2d_array_depth_rw(%spirv.SampledImage.image2d_array_depth_rw_t*)
declare void @_Z18my_readwrite_image37__spirv_SampledImage__image2d_msaa_rw(%spirv.SampledImage.image2d_msaa_rw_t*)
declare void @_Z18my_readwrite_image43__spirv_SampledImage__image2d_array_msaa_rw(%spirv.SampledImage.image2d_array_msaa_rw_t*)
declare void @_Z18my_readwrite_image43__spirv_SampledImage__image2d_msaa_depth_rw(%spirv.SampledImage.image2d_msaa_depth_rw_t*)
declare void @_Z18my_readwrite_image49__spirv_SampledImage__image2d_array_msaa_depth_rw(%spirv.SampledImage.image2d_array_msaa_depth_rw_t*)
declare void @_Z18my_readwrite_image32__spirv_SampledImage__image3d_rw(%spirv.SampledImage.image3d_rw_t*)

; Check all occurrences of original mangled type names are replaced.
define void @test_multiple_args() {
  call void @_Z13my_read_image32__spirv_SampledImage__image1d_ro38__spirv_SampledImage__image2d_array_ro(%spirv.SampledImage.image1d_ro_t* undef, %spirv.SampledImage.image2d_array_ro_t* undef)
; CHECK-NOT: _Z13my_read_image32__spirv_SampledImage__image1d_ro38__spirv_SampledImage__image2d_array_ro
; CHECK: _Z13my_read_image40__spirv_SampledImage__void_1_0_0_0_1_0_040__spirv_SampledImage__void_2_0_1_0_1_0_0(%spirv.SampledImage._void_1_0_0_0_1_0_0* undef, %spirv.SampledImage._void_2_0_1_0_1_0_0* undef)
  ret void
}

declare void @_Z13my_read_image32__spirv_SampledImage__image1d_ro38__spirv_SampledImage__image2d_array_ro(%spirv.SampledImage.image1d_ro_t*, %spirv.SampledImage.image2d_array_ro_t*)

!opencl.ocl.version = !{!0}

!0 = !{i32 2, i32 0}

; DEBUGIFY-NOT: WARNING
