; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-pipe-ordering %s -S | FileCheck %s

; IR is dumped before PipeOrderingPass when building following kernel:
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch;
;
; kernel void foo(global int* flags) {
;   // Loop only in for header.
;   for (int i = 0; ; ++i) {
;     write_channel_intel(ch, 42);
;     if (flags[i]) {
;       break;
;     }
;     write_channel_intel(ch, 42);
;     // implicit work-group barrier here
;   }
; }

; CHECK-LABEL: for.cond:
; CHECK: call void @_Z18work_group_barrierj(i32 1)
; CHECK-NEXT: br i1 %tobool.not, label %if.end, label %if.then

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@ch = internal addrspace(1) global target("spirv.Channel") zeroinitializer, align 4, !packet_size !0, !packet_align !0
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @__pipe_global_ctor, ptr null }]
@ch.pipe = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@ch.pipe.bs = addrspace(1) global [456 x i8] zeroinitializer, align 4

define dso_local void @foo(ptr addrspace(1) noundef align 4 %flags) #0 !kernel_arg_base_type !3 !arg_type_null_val !4 {
entry:
  %write.src = alloca i32, align 4
  br label %for.cond

for.cond:                                         ; preds = %if.end, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %if.end ]
  %0 = load ptr addrspace(1), ptr addrspace(1) @ch.pipe, align 8, !tbaa !5
  %1 = load ptr addrspace(1), ptr addrspace(1) @ch, align 4, !tbaa !5
  store i32 42, ptr %write.src, align 4
  %2 = addrspacecast ptr %write.src to ptr addrspace(4)
  %3 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %0, ptr addrspace(4) %2, i32 4, i32 4)
  %idxprom = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %flags, i64 %idxprom
  %4 = load i32, ptr addrspace(1) %arrayidx, align 4, !tbaa !8
  %tobool.not = icmp eq i32 %4, 0
  br i1 %tobool.not, label %if.end, label %if.then

if.then:                                          ; preds = %for.cond
  ret void

if.end:                                           ; preds = %for.cond
  %5 = load ptr addrspace(1), ptr addrspace(1) @ch.pipe, align 8, !tbaa !5
  %6 = load ptr addrspace(1), ptr addrspace(1) @ch, align 4, !tbaa !5
  store i32 42, ptr %write.src, align 4
  %7 = addrspacecast ptr %write.src to ptr addrspace(4)
  %8 = call i32 @__write_pipe_2_bl_fpga(ptr addrspace(1) %5, ptr addrspace(4) %7, i32 4, i32 4)
  %inc = add nuw nsw i32 %i.0, 1
  br label %for.cond
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
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !6, i64 0}

; DEBUGIFY-NOT: WARNING
