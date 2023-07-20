; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S | FileCheck %s

; IR is dumped before PipeOrderingPass when building following kernel:
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
; 
; channel int ch;
; 
; __attribute__((noinline)) void foo(int iters) {
;   for (int i = 0; i < iters; ++i)
;     write_channel_intel(ch, 42);
; }
; 
; __attribute__((noinline)) void boo(int iters) { foo(iters); }
; 
; __attribute__((max_global_work_dim(0))) kernel void moo(global int *iters) {
;   boo(*iters);
; }

; CHECK: define internal void @foo(
; CHECK-LABEL: for.cond:
; CHECK-NOT: call void @_Z18work_group_barrierj
; CHECK: br i1 %cmp, label %for.body, label %for.cond.cleanup

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@ch = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@ch.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4

define internal void @foo(i32 noundef %iters) #0 {
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
  %0 = load ptr addrspace(1), ptr addrspace(1) @ch.pipe, align 8, !tbaa !3
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch, align 4, !tbaa !3
  store i32 42, ptr %write.src, align 4
  %2 = addrspacecast ptr %write.src to ptr addrspace(4)
  %3 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond
}

define internal void @boo(i32 noundef %iters) #0 {
entry:
  call void @foo(i32 noundef %iters) #2
  ret void
}

define dso_local void @moo(ptr addrspace(1) noundef align 4 %iters) #1 !kernel_arg_base_type !6 !kernel_arg_pipe_depth !7 !kernel_arg_pipe_io !8 !arg_type_null_val !9 !max_global_work_dim !7 {
entry:
  %0 = load i32, ptr addrspace(1) %iters, align 4, !tbaa !10
  call void @boo(i32 noundef %0) #2
  ret void
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_fpga(ptr addrspace(1) @ch.pipe.bs, i32 4, i32 0, i32 0)
  store ptr addrspace(1) @ch.pipe.bs, ptr addrspace(1) @ch.pipe, align 8
  ret void
}

declare void @__pipe_init_fpga(ptr addrspace(1) noundef, i32 noundef, i32 noundef, i32 noundef) #1

declare i32 @__write_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { convergent noinline norecurse nounwind }
attributes #1 = { convergent norecurse nounwind }
attributes #2 = { convergent nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!sycl.kernels = !{!2}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{ptr @moo}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!"int*"}
!7 = !{i32 0}
!8 = !{!""}
!9 = !{ptr addrspace(1) null}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !4, i64 0}

; DEBUGIFY-NOT: WARNING
