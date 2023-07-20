; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S | FileCheck %s

; IR is dumped before PipeOrderingPass when building following kernel:
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch1;
; channel int ch2;
;
; __attribute__((noinline)) void write_to_channel_in_loop(int iters) {
;   for (int i = 0i < iters++ i) {
;     write_channel_intel(ch1, 42);
;     // implicit work-group barrier here
;   }
; }
;
; __attribute__((noinline)) void write_to_channel() {
;   write_to_channel_in_loop(64);
;   write_channel_intel(ch2, 42);
; }
;
; __kernel void foo(global int *iters) {
;   for (int i = 0; i < *iters; ++i) {
;     write_to_channel();
;     // implicit work-group barrier here
;   }
; }

; CHECK: define internal void @write_to_channel_in_loop(
; CHECK-LABEL: for.cond:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: br i1 %cmp, label %for.body, label %for.cond.cleanup
; CHECK: define dso_local void @foo(
; CHECK-LABEL: for.cond:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK: br i1 %cmp, label %for.body, label %for.cond.cleanup

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@ch1 = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@ch2 = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@ch1.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch1.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4
@ch2.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch2.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4

define internal void @write_to_channel_in_loop(i32 noundef %iters) #0 {
entry:
  %write.src = alloca i32, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp slt i32 %i.0, %iters
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  %0 = load ptr addrspace(1), ptr addrspace(1) @ch1.pipe, align 8, !tbaa !5
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch1, align 4, !tbaa !5
  store i32 42, ptr %write.src, align 4
  %2 = addrspacecast ptr %write.src to ptr addrspace(4)
  %3 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

define internal void @write_to_channel() #0 {
entry:
  %write.src = alloca i32, align 4
  call void @write_to_channel_in_loop(i32 noundef 64) #4
  %0 = load ptr addrspace(1), ptr addrspace(1) @ch2.pipe, align 8, !tbaa !5
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch2, align 4, !tbaa !5
  store i32 42, ptr %write.src, align 4
  %2 = addrspacecast ptr %write.src to ptr addrspace(4)
  %3 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  ret void
}

define dso_local void @foo(ptr addrspace(1) noundef align 4 %iters) #2 !kernel_arg_base_type !10 !kernel_arg_pipe_depth !14 !kernel_arg_pipe_io !11 !arg_type_null_val !15 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %0 = load i32, ptr addrspace(1) %iters, align 4, !tbaa !16
  %cmp = icmp slt i32 %i.0, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  call void @write_to_channel() #4
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(ptr addrspace(1) @ch1.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch1.pipe.bs, ptr addrspace(1) @ch1.pipe, align 8
  call void @__pipe_init_fpga(ptr addrspace(1) @ch2.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch2.pipe.bs, ptr addrspace(1) @ch2.pipe, align 8
  ret void
}

declare void @__pipe_init_fpga(ptr addrspace(1) noundef, i32 noundef, i32 noundef, i32 noundef) #3

declare i32 @__write_pipe_2_fpga(ptr addrspace(1), ptr addrspace(4) nocapture noundef readonly, i32 noundef, i32 noundef) #3

declare i32 @__write_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { convergent noinline norecurse nounwind }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { convergent norecurse nounwind }
attributes #3 = { convergent norecurse nounwind }
attributes #4 = { convergent nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!sycl.kernels = !{!4}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!4 = !{ptr @foo}
!5 = !{!6, !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{i32 1}
!9 = !{!"none"}
!10 = !{!"int*"}
!11 = !{!""}
!12 = !{!"iters"}
!13 = !{i1 false}
!14 = !{i32 0}
!15 = !{ptr addrspace(1) null}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !6, i64 0}

; DEBUGIFY-NOT: WARNING
