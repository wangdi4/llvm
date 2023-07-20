; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S | FileCheck %s

; IR is dumped before PipeOrderingPass when building following kernel:
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch;
;
; kernel void foo(global int* iters) {
;   for (int i = 0; i < *iters; ++i) {
;     for (int j = 0; j < i*i+1; ++j) {
;       write_channel_intel(ch, 42);
;       // implicit work-group barrier here
;     }
;     for (int j = 0; j < i*i*i+1; ++j) {
;       write_channel_intel(ch, 42);
;       // implicit work-group barrier here
;     }
;   }
; }

; CHECK-LABEL: for.cond1:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT: br i1 %cmp2.not, label %for.cond.cleanup3, label %for.body4
; CHECK-LABEL: for.cond6:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT: br i1 %cmp10.not, label %for.cond.cleanup11, label %for.body12

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@ch = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@ch.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4

define dso_local void @foo(ptr addrspace(1) noundef align 4 %iters) #0 !kernel_arg_base_type !3 !arg_type_null_val !4 {
entry:
  %write.src = alloca i32, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.cond.cleanup11, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc17, %for.cond.cleanup11 ]
  %0 = load i32, ptr addrspace(1) %iters, align 4, !tbaa !5
  %cmp = icmp slt i32 %i.0, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.body4, %for.body
  %j.0 = phi i32 [ 0, %for.body ], [ %inc, %for.body4 ]
  %mul = mul nsw i32 %i.0, %i.0
  %cmp2.not = icmp ugt i32 %j.0, %mul
  br i1 %cmp2.not, label %for.cond.cleanup3, label %for.body4

for.cond.cleanup3:                                ; preds = %for.cond1
  br label %for.cond6

for.body4:                                        ; preds = %for.cond1
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch.pipe, align 8, !tbaa !9
  %2 = load ptr addrspace(1), ptr addrspace(1) @ch, align 4, !tbaa !9
  store i32 42, ptr %write.src, align 4
  %3 = addrspacecast ptr %write.src to ptr addrspace(4)
  %4 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %1, ptr addrspace(4) %3, i32 4, i32 4)
  %inc = add nuw nsw i32 %j.0, 1
  br label %for.cond1

for.cond6:                                        ; preds = %for.body12, %for.cond.cleanup3
  %j5.0 = phi i32 [ 0, %for.cond.cleanup3 ], [ %inc14, %for.body12 ]
  %mul7 = mul nsw i32 %i.0, %i.0
  %mul8 = mul nsw i32 %mul7, %i.0
  %cmp10.not = icmp ugt i32 %j5.0, %mul8
  br i1 %cmp10.not, label %for.cond.cleanup11, label %for.body12

for.cond.cleanup11:                               ; preds = %for.cond6
  %inc17 = add nuw nsw i32 %i.0, 1
  br label %for.cond

for.body12:                                       ; preds = %for.cond6
  %5 = load ptr addrspace(1), ptr addrspace(1) @ch.pipe, align 8, !tbaa !9
  %6 = load ptr addrspace(1), ptr addrspace(1) @ch, align 4, !tbaa !9
  store i32 42, ptr %write.src, align 4
  %7 = addrspacecast ptr %write.src to ptr addrspace(4)
  %8 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %5, ptr addrspace(4) %7, i32 4, i32 4)
  %inc14 = add nuw nsw i32 %j5.0, 1
  br label %for.cond6
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
