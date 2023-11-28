; IR is dumped before PipeSupportPass when building following kernel:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int c4[4][8];
;
; __kernel void foo() {
;   int val4 = read_channel_intel(c4[0][0]);
; }
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-pipe-support %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-pipe-support %s -S | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@c4.pipe = addrspace(1) global [4 x [8 x ptr addrspace(1)]] zeroinitializer, align 16, !packet_size !0, !packet_align !0
@c4.pipe.bs = addrspace(1) global [14592 x i8] zeroinitializer, align 4

; Function Attrs: norecurse nounwind
define dso_local void @foo() local_unnamed_addr #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_name !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !arg_type_null_val !4 {
entry:
; CHECK: call void @__store_read_pipe_use(
; CHECK: call i32 @__read_pipe_2_fpga(
; CHECK: call void @__flush_pipe_read_array(
; CHECK: call void @__flush_pipe_write_array(
; CHECK: call void @__flush_pipe_read_array(
; CHECK: call void @__flush_pipe_write_array(

  %read.dst = alloca i32, align 4
  %0 = load ptr addrspace(1), ptr addrspace(1) @c4.pipe, align 16, !tbaa !5
  %1 = addrspacecast ptr %read.dst to ptr addrspace(4)
  %call1 = call i32 @__read_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %1, i32 4, i32 4) #3
  ret void
}

; Function Attrs: norecurse nounwind
define void @__pipe_global_ctor() #1 {
entry:
  store ptr addrspace(1) @c4.pipe.bs, ptr addrspace(1) @c4.pipe, align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 456), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 912), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 2), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 1368), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 1824), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 4), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 2280), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 2736), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 6), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 3192), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 0, i64 7), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 3648), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 0), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 4104), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 4560), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 2), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 5016), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 5472), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 4), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 5928), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 6384), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 6), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 6840), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 1, i64 7), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 7296), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 0), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 7752), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 8208), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 2), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 8664), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 9120), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 4), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 9576), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 10032), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 6), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 10488), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 2, i64 7), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 10944), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 0), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 11400), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 11856), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 2), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 12312), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 3), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 12768), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 4), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 13224), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 5), align 8
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 13680), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 6), align 16
  store ptr addrspace(1) getelementptr inbounds ([14592 x i8], ptr addrspace(1) @c4.pipe.bs, i64 0, i64 14136), ptr addrspace(1) getelementptr inbounds ([4 x [8 x ptr addrspace(1)]], ptr addrspace(1) @c4.pipe, i64 0, i64 3, i64 7), align 8
  tail call void @__pipe_init_array_fpga(ptr addrspace(1) @c4.pipe, i32 32, i32 4, i32 0, i32 0)
  ret void
}

; Function Attrs: convergent norecurse nounwind
declare void @__pipe_init_array_fpga(ptr addrspace(1) nocapture noundef readonly, i32 noundef, i32 noundef, i32 noundef, i32 noundef) local_unnamed_addr #2

declare i32 @__read_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32) local_unnamed_addr

attributes #0 = { norecurse nounwind "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" }
attributes #1 = { norecurse nounwind }
attributes #2 = { convergent norecurse nounwind "denormal-fp-math"="dynamic,dynamic" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="skx" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #3 = { nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!llvm.ident = !{!2}
!sycl.kernels = !{!3}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!3 = !{ptr @foo}
!4 = !{}
!5 = !{!6, !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

; DEBUGIFY-NOT: WARNING
