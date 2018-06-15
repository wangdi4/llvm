; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int c4[4][8];
;
; __kernel void foo() {
;   int val4 = read_channel_intel(c4[0][0]);
; }
; ----------------------------------------------------
; Compilation command:
;   clang -cc1 -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -emit-llvm -cl-std=CL1.2
;   oclopt -runtimelib=%p/../../vectrorizer/Full/runtime.bc -demangle-fpga-pipes -spir-materializer -channel-pipe-transformation -verify %s -S
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -pipe-support -verify %s -S
; REQUIRES: fpga-emulator

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque
%opencl.pipe_rw_t = type opaque
%opencl.pipe_ro_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@c4 = common addrspace(1) global [4 x [8 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]
@c4.pipe = addrspace(1) global [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] zeroinitializer, align 16, !packet_size !0, !packet_align !0
@c4.pipe.bs = addrspace(1) global [10496 x i8] zeroinitializer, align 4

; Function Attrs: convergent nounwind
define void @foo() #0 !kernel_arg_addr_space !3 !kernel_arg_access_qual !3 !kernel_arg_type !3 !kernel_arg_base_type !3 !kernel_arg_type_qual !3 !kernel_arg_host_accessible !3 !kernel_arg_pipe_depth !3 !kernel_arg_pipe_io !3 !kernel_arg_buffer_location !3 {
entry:
  %read.dst = alloca i32
  %val4 = alloca i32, align 4
  %0 = bitcast i32* %val4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  %1 = load %opencl.pipe_rw_t addrspace(1)*, %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i64 0, i64 0, i64 0)
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.channel_t addrspace(1)*]], [4 x [8 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @c4, i64 0, i64 0, i64 0), align 4, !tbaa !6
  %3 = bitcast %opencl.pipe_rw_t addrspace(1)* %1 to %opencl.pipe_ro_t addrspace(1)*
  %4 = addrspacecast i32* %read.dst to i8 addrspace(4)*
  %call1 = call i32 @__read_pipe_2_bl_intel(%opencl.pipe_ro_t addrspace(1)* %3, i8 addrspace(4)* %4, i32 4, i32 4)
  %5 = load i32, i32* %read.dst
  store i32 %5, i32* %val4, align 4, !tbaa !9
  %6 = bitcast i32* %val4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %6) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

define void @__pipe_global_ctor() {
entry:
  store %opencl.pipe_rw_t addrspace(1)* bitcast ([10496 x i8] addrspace(1)* @c4.pipe.bs to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 328) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 1)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 656) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 2)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 984) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 3)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 1312) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 4)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 1640) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 5)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 1968) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 6)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 2296) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 0, i32 7)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 2624) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 2952) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 1)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 3280) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 2)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 3608) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 3)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 3936) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 4)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 4264) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 5)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 4592) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 6)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 4920) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 1, i32 7)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 5248) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 5576) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 1)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 5904) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 2)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 6232) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 3)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 6560) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 4)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 6888) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 5)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 7216) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 6)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 7544) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 2, i32 7)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 7872) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 0)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 8200) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 1)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 8528) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 2)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 8856) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 3)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 9184) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 4)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 9512) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 5)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 9840) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 6)
  store %opencl.pipe_rw_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([10496 x i8], [10496 x i8] addrspace(1)* @c4.pipe.bs, i64 0, i64 10168) to %opencl.pipe_rw_t addrspace(1)*), %opencl.pipe_rw_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]], [4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe, i32 0, i32 3, i32 7)
  call void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)* bitcast ([4 x [8 x %opencl.pipe_rw_t addrspace(1)*]] addrspace(1)* @c4.pipe to %struct.__pipe_t addrspace(1)* addrspace(1)*), i32 32, i32 4, i32 0, i32 0)
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)*, i32, i32, i32, i32) #2

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_intel(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)* nocapture, i32, i32) #2

declare i32 @__read_pipe_2_bl_intel(%opencl.pipe_ro_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind }

!llvm.module.flags = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!opencl.kernels = !{!5}

!0 = !{i32 4}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"clang version 7.0.0 "}
!5 = !{void ()* @foo}
!6 = !{!7, !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !7, i64 0}
