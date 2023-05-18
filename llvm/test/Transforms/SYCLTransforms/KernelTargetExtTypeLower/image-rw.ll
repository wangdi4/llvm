; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

; Compiled from OpenCL kernel:
; kernel void test_rw_images(read_write image1d_t src1, read_write image2d_t src2, __write_only image3d_t src3, sampler_t s, local queue_t *q, int2 coords) {
;   uint4 src_val = read_imageui(src2, coords);
;   write_imageui(src2, coords, src_val);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define dso_local spir_kernel void @test_rw_images(target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2) %src1, target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2) %src2, target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 1) %src3, target("spirv.Sampler") %s, ptr addrspace(3) noundef align 8 %q, <2 x i32> noundef %coords) #0 !kernel_arg_addr_space !0 !kernel_arg_access_qual !1 !kernel_arg_type !2 !kernel_arg_base_type !3 !kernel_arg_type_qual !4 !kernel_arg_name !5 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 {
entry:
; CHECK: define dso_local spir_kernel void @test_rw_images(ptr addrspace(1) %src1, ptr addrspace(1) %src2, ptr addrspace(1) %src3, ptr %s, ptr addrspace(3) noundef align 8 %q, <2 x i32> noundef %coords) {{.*}} !arg_type_null_val [[MD_ARG_TY_NULL:![0-9]+]]
; CHECK: %src1.addr = alloca ptr addrspace(1), align 8
; CHECK: %src2.addr = alloca ptr addrspace(1), align 8
; CHECK: %src3.addr = alloca ptr addrspace(1), align 8
; CHECK: %s.addr = alloca ptr, align 8
; CHECK: store ptr addrspace(1) %src1, ptr %src1.addr, align 8
; CHECK: store ptr addrspace(1) %src2, ptr %src2.addr, align 8
; CHECK: store ptr addrspace(1) %src3, ptr %src3.addr, align 8
; CHECK: store ptr addrspace(3) %q, ptr %q.addr, align 8
; CHECK: load ptr addrspace(1), ptr %src2.addr, align 8
; CHECK: call spir_func <4 x i32> @_Z12read_imageui14ocl_image2d_rwDv2_i(ptr addrspace(1)
; CHECK: load ptr addrspace(1), ptr %src2.addr, align 8
; CHECK: call spir_func void @_Z13write_imageui14ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1)

  %src1.addr = alloca target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2), align 8
  %src2.addr = alloca target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2), align 8
  %src3.addr = alloca target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 1), align 8
  %s.addr = alloca target("spirv.Sampler"), align 8
  %q.addr = alloca ptr addrspace(3), align 8
  %coords.addr = alloca <2 x i32>, align 8
  %src_val = alloca <4 x i32>, align 16
  store target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2) %src1, ptr %src1.addr, align 8, !tbaa !8
  store target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2) %src2, ptr %src2.addr, align 8, !tbaa !12
  store target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 1) %src3, ptr %src3.addr, align 8, !tbaa !14
  store target("spirv.Sampler") %s, ptr %s.addr, align 8, !tbaa !16
  store ptr addrspace(3) %q, ptr %q.addr, align 8, !tbaa !18
  store <2 x i32> %coords, ptr %coords.addr, align 8, !tbaa !20
  %0 = load target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2), ptr %src2.addr, align 8, !tbaa !12
  %1 = load <2 x i32>, ptr %coords.addr, align 8, !tbaa !20
  %call = call spir_func <4 x i32> @_Z12read_imageui14ocl_image2d_rwDv2_i(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2) %0, <2 x i32> noundef %1) #2
  store <4 x i32> %call, ptr %src_val, align 16, !tbaa !20
  %2 = load target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2), ptr %src2.addr, align 8, !tbaa !12
  %3 = load <2 x i32>, ptr %coords.addr, align 8, !tbaa !20
  %4 = load <4 x i32>, ptr %src_val, align 16, !tbaa !20
  call spir_func void @_Z13write_imageui14ocl_image2d_rwDv2_iDv4_j(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2) %2, <2 x i32> noundef %3, <4 x i32> noundef %4) #3
  ret void
}

; CHECK-DAG: declare spir_func <4 x i32> @_Z12read_imageui14ocl_image2d_rwDv2_i(ptr addrspace(1), <2 x i32> noundef)

declare spir_func <4 x i32> @_Z12read_imageui14ocl_image2d_rwDv2_i(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2), <2 x i32> noundef) #2

; CHECK-DAG: declare spir_func void @_Z13write_imageui14ocl_image2d_rwDv2_iDv4_j(ptr addrspace(1), <2 x i32> noundef, <4 x i32> noundef)

declare spir_func void @_Z13write_imageui14ocl_image2d_rwDv2_iDv4_j(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2), <2 x i32> noundef, <4 x i32> noundef) #3

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { convergent nounwind willreturn memory(read) }
attributes #3 = { convergent nounwind }

!0 = !{i32 1, i32 1, i32 1, i32 0, i32 3, i32 0}
!1 = !{!"read_write", !"read_write", !"write_only", !"none", !"none", !"none"}
!2 = !{!"image1d_t", !"image2d_t", !"image3d_t", !"sampler_t", !"queue_t*", !"int2"}
!3 = !{!"image1d_t", !"image2d_t", !"image3d_t", !"sampler_t", !"queue_t*", !"int __attribute__((ext_vector_type(2)))"}
!4 = !{!"", !"", !"", !"", !"", !""}
!5 = !{!"src1", !"src2", !"src3", !"s", !"q", !"coords"}
!6 = !{i1 false, i1 false, i1 false, i1 false, i1 false, i1 false}
!7 = !{i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"__read_write image1d_t", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"__read_write image2d_t", !10, i64 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"__write_only image3d_t", !10, i64 0}
!16 = !{!17, !17, i64 0}
!17 = !{!"sampler_t", !10, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"any pointer", !10, i64 0}
!20 = !{!10, !10, i64 0}

; DEBUGIFY-NOT: WARNING
