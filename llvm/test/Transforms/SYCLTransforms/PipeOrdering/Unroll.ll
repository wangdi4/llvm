; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S | FileCheck %s

; IR is dumped before PipeOrderingPass when building following kernel:
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; 
; channel int ch1;
; channel int ch_arr[4];
; 
; kernel void foo(global int* iters) {
;   for (int i = 0; i < *iters; ++i) {
;     write_channel_intel(ch1, 42);
;     // implicit work-group barrier here
;   }
; }
; 
; kernel void boo() {
;   #pragma unroll
;   for (int i = 0; i < 4; ++i)
;     write_channel_intel(ch_arr[i], 42);
; }

; CHECK:   define dso_local void @foo(
; CHECK:     call void @_Z18work_group_barrierj(i32 1)
; CHECK:   define dso_local void @boo(
; CHECK-NOT: call void @_Z18work_group_barrierj(i32 1)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@ch1 = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@ch_arr = internal addrspace(1) global [4 x target("spirv.Channel")] zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@ch1.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch1.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4
@ch_arr.pipe = addrspace(1) global [4 x ptr addrspace(1)] zeroinitializer, align 16, !packet_size !0, !packet_align !0
@ch_arr.pipe.bs = addrspace(1) global [1824 x i8] zeroinitializer, align 4

define dso_local void @foo(ptr addrspace(1) noundef align 4 %iters) #0 !kernel_arg_base_type !3 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !5 !arg_type_null_val !6 {
entry:
  %write.src = alloca i32, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %0 = load i32, ptr addrspace(1) %iters, align 4, !tbaa !7
  %cmp = icmp slt i32 %i.0, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch1.pipe, align 8, !tbaa !11
  %2 = load ptr addrspace(1), ptr addrspace(1) @ch1, align 4, !tbaa !11
  store i32 42, ptr %write.src, align 4
  %3 = addrspacecast ptr %write.src to ptr addrspace(4)
  %4 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %1, ptr addrspace(4) %3, i32 4, i32 4)
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond
}

define dso_local void @boo() #0 !kernel_arg_base_type !12 !kernel_arg_pipe_depth !12 !kernel_arg_pipe_io !12 !arg_type_null_val !12 {
entry:
  %write.src = alloca i32, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp ult i32 %i.0, 4
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  %idxprom = zext i32 %i.0 to i64
  %arrayidx3 = getelementptr [4 x ptr addrspace(1)], ptr addrspace(1) @ch_arr.pipe, i64 0, i64 %idxprom
  %arrayidx = getelementptr inbounds [4 x target("spirv.Channel")], ptr addrspace(1) @ch_arr, i64 0, i64 %idxprom
  %0 = load ptr addrspace(1), ptr addrspace(1) %arrayidx3, align 8, !tbaa !11
  %1 = load ptr addrspace(1), ptr addrspace(1) %arrayidx, align 4, !tbaa !11
  store i32 42, ptr %write.src, align 4
  %2 = addrspacecast ptr %write.src to ptr addrspace(4)
  %3 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !13
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(ptr addrspace(1) @ch1.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch1.pipe.bs, ptr addrspace(1) @ch1.pipe, align 8
  store ptr addrspace(1) @ch_arr.pipe.bs, ptr addrspace(1) @ch_arr.pipe, align 8
  store ptr addrspace(1) getelementptr inbounds ([1824 x i8], ptr addrspace(1) @ch_arr.pipe.bs, i64 0, i64 456), ptr addrspace(1) getelementptr inbounds ([4 x ptr addrspace(1)], ptr addrspace(1) @ch_arr.pipe, i32 0, i32 1), align 8
  store ptr addrspace(1) getelementptr inbounds ([1824 x i8], ptr addrspace(1) @ch_arr.pipe.bs, i64 0, i64 912), ptr addrspace(1) getelementptr inbounds ([4 x ptr addrspace(1)], ptr addrspace(1) @ch_arr.pipe, i32 0, i32 2), align 8
  store ptr addrspace(1) getelementptr inbounds ([1824 x i8], ptr addrspace(1) @ch_arr.pipe.bs, i64 0, i64 1368), ptr addrspace(1) getelementptr inbounds ([4 x ptr addrspace(1)], ptr addrspace(1) @ch_arr.pipe, i32 0, i32 3), align 8
  call void @__pipe_init_array_fpga(ptr addrspace(1) @ch_arr.pipe, i32 4, i32 4, i32 0, i32 0)
  ret void
}

declare void @__pipe_init_fpga(ptr addrspace(1) noundef, i32 noundef, i32 noundef, i32 noundef) #0

declare void @__pipe_init_array_fpga(ptr addrspace(1) nocapture noundef readonly, i32 noundef, i32 noundef, i32 noundef, i32 noundef) #0

declare i32 @__write_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { convergent norecurse nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!sycl.kernels = !{!2}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{ptr @foo, ptr @boo}
!3 = !{!"int*"}
!4 = !{i32 0}
!5 = !{!""}
!6 = !{ptr addrspace(1) null}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!9, !9, i64 0}
!12 = !{}
!13 = distinct !{!13, !14}
!14 = !{!"llvm.loop.unroll.enable"}

; DEBUGIFY-NOT: WARNING
