; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S | FileCheck %s

; IR is dumped before PipeOrderingPass when building following kernel:
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch1;
; channel int ch2;
;
; kernel void foo(global int* iters) {
;   for (int i = 0; i < *iters; ++i) {
;     if (*iters > 10) {
;       write_channel_intel(ch1, 42);
;     }
;     else {
;       write_channel_intel(ch2, 42);
;     }
;     // implicit work-group barrier here
;   }
; }

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

define dso_local void @foo(ptr addrspace(1) noundef align 4 %iters) #0 !kernel_arg_base_type !3 !arg_type_null_val !4 {
entry:
  %write.src = alloca i32, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.inc ]
  %0 = load i32, ptr addrspace(1) %iters, align 4, !tbaa !5
  %cmp = icmp slt i32 %i.0, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr addrspace(1) %iters, align 4, !tbaa !5
  %cmp1 = icmp sgt i32 %1, 10
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %2 = load ptr addrspace(1), ptr addrspace(1) @ch1.pipe, align 8, !tbaa !9
  %3 = load ptr addrspace(1), ptr addrspace(1) @ch1, align 4, !tbaa !9
  store i32 42, ptr %write.src, align 4
  %4 = addrspacecast ptr %write.src to ptr addrspace(4)
  %5 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %2, ptr addrspace(4) %4, i32 4, i32 4)
  br label %for.inc

if.else:                                          ; preds = %for.body
  %6 = load ptr addrspace(1), ptr addrspace(1) @ch2.pipe, align 8, !tbaa !9
  %7 = load ptr addrspace(1), ptr addrspace(1) @ch2, align 4, !tbaa !9
  store i32 42, ptr %write.src, align 4
  %8 = addrspacecast ptr %write.src to ptr addrspace(4)
  %9 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %6, ptr addrspace(4) %8, i32 4, i32 4)
  br label %for.inc

for.inc:                                          ; preds = %if.else, %if.then
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

declare void @__pipe_init_fpga(ptr addrspace(1) noundef, i32 noundef, i32 noundef, i32 noundef) #0

declare i32 @__write_pipe_2_bl_fpga(ptr addrspace(1), ptr addrspace(4), i32, i32)

attributes #0 = { convergent norecurse nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!sycl.kernels = !{!2}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{ptr @foo}
!3 = !{!"int*"}
!4 = !{ptr addrspace(1) null}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!7, !7, i64 0}

; DEBUGIFY-NOT: WARNING
