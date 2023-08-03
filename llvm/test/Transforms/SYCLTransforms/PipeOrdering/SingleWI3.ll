; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S | FileCheck %s

; IR is dumped before PipeOrderingPass when building following kernel:
; (disable warn_channel_is_used_from_more_than_one_kernel)
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch;
;
; kernel void foo(global int *iters) {
;   for (int i = 0; i < *iters; ++i)
;     write_channel_intel(ch, 42);
; }
;
; __attribute__((max_global_work_dim(0))) kernel void boo(global int *iters) {
;   foo(iters);
; }

; CHECK: define dso_local void @foo(
; CHECK-LABEL: for.cond:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT: br i1 %cmp, label %for.body, label %for.cond.cleanup

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@ch = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@ch.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4

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
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch.pipe, align 8, !tbaa !11
  %2 = load ptr addrspace(1), ptr addrspace(1) @ch, align 4, !tbaa !11
  store i32 42, ptr %write.src, align 4
  %3 = addrspacecast ptr %write.src to ptr addrspace(4)
  %4 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %1, ptr addrspace(4) %3, i32 4, i32 4)
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond
}


define dso_local void @boo(ptr addrspace(1) noundef align 4 %iters) #0 !kernel_arg_base_type !3 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !5 !arg_type_null_val !6 !max_global_work_dim !4 {
entry:
  call void @foo(ptr addrspace(1) noundef %iters) #1
  ret void
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(ptr addrspace(1) @ch.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch.pipe.bs, ptr addrspace(1) @ch.pipe, align 8
  ret void
}

declare void @__pipe_init_fpga(ptr addrspace(1) noundef, i32 noundef, i32 noundef, i32 noundef) #0

declare i32 @__write_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { convergent nounwind }

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

; DEBUGIFY-NOT: WARNING
