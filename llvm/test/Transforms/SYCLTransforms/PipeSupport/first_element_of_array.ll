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
;   opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl.bc -sycl-demangle-fpga-pipes -spir-materializer -sycl-kernel-channel-pipe-transformation -verify %s -S
; ----------------------------------------------------
; RUN: llvm-as %p/../Inputs/fpga-pipes.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-support %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-pipe-support %s -S

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@c4 = internal addrspace(1) global [4 x [8 x target("spirv.Channel")]] zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@c4.pipe = addrspace(1) global [4 x [8 x ptr addrspace(1)]] zeroinitializer, align 16, !packet_size !0, !packet_align !0
@c4.pipe.bs = addrspace(1) global [10496 x i8] zeroinitializer, align 4

; Function Attrs: convergent noinline norecurse nounwind optnone
define dso_local spir_kernel void @foo() #0 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 !kernel_arg_host_accessible !2 !kernel_arg_pipe_depth !2 !kernel_arg_pipe_io !2 !kernel_arg_buffer_location !2 {
entry:
  %read.dst = alloca i32, align 4
  %val4 = alloca i32, align 4
  %0 = load ptr addrspace(1), ptr addrspace(1) @c4.pipe, align 8
  %1 = load target("spirv.Channel"), ptr addrspace(1) @c4, align 4
  %2 = addrspacecast ptr %read.dst to ptr addrspace(4)
  %call1 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  %3 = load i32, ptr %read.dst, align 4
  store i32 %3, ptr %val4, align 4
  ret void
}

define void @__pipe_global_ctor() {
entry:
  store ptr addrspace(1) @c4.pipe.bs, ptr addrspace(1) @c4.pipe, align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 328), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 0, i32 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 656), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 0, i32 2), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 984), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 0, i32 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 1312), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 0, i32 4), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 1640), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 0, i32 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 1968), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 0, i32 6), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 2296), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 0, i32 7), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 2624), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 1, i32 0), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 2952), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 1, i32 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 3280), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 1, i32 2), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 3608), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 1, i32 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 3936), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 1, i32 4), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 4264), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 1, i32 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 4592), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 1, i32 6), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 4920), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 1, i32 7), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 5248), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 2, i32 0), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 5576), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 2, i32 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 5904), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 2, i32 2), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 6232), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 2, i32 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 6560), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 2, i32 4), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 6888), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 2, i32 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 7216), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 2, i32 6), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 7544), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 2, i32 7), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 7872), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 3, i32 0), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 8200), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 3, i32 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 8528), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 3, i32 2), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 8856), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 3, i32 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 9184), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 3, i32 4), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 9512), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 3, i32 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 9840), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 3, i32 6), align 8
  store ptr addrspace(1) getelementptr inbounds ([10496 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 10168), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i32 0, i32 3, i32 7), align 8
  call void @__pipe_init_array_fpga(ptr addrspace(1) @c4.pipe, i32 32, i32 4, i32 0, i32 0)
  ret void
}

; Function Attrs: nounwind memory(none)
declare void @__pipe_init_array_fpga(ptr addrspace(1), i32, i32, i32, i32) #1

; Function Attrs: nounwind memory(none)
declare i32 @__read_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture, i32, i32) #1

declare i32 @__read_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { convergent noinline norecurse nounwind optnone "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #1 = { nounwind memory(none) }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.compiler.options = !{!2}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}

; DEBUGIFY-NOT: WARNING
