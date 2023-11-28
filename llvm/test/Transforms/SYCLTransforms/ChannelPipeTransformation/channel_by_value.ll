; Compiled from:
; (disable warn_channel_is_used_from_more_than_one_kernel)
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
;
; channel int a;
; channel int b;
;
; __attribute__((noinline))
; void sendData(channel int ch) {
;   write_channel_intel(ch, 5);
; }
;
; __kernel void foo(int f) {
;   if (f < 0) {
;     sendData(a);
;   } else {
;     sendData(b);
;   }
; }
;
; __attribute__((noinline))
; void sendData2(channel int a, channel int b, int f) {
;   f ? write_channel_intel(a, 5) : write_channel_intel(b, 5);
; }
;
; __kernel void bar(int f) {
;   sendData2(a, b, f);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@a = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0
@b = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0

; CHECK: define {{.*}} @foo
; CHECK: %[[A_PIPE:.*]] = load {{.*}} @a
; CHECK: call void @sendData({{.*}} %[[A_PIPE]])
; CHECK: %[[B_PIPE:.*]] = load {{.*}} @b
; CHECK: call void @sendData({{.*}} %[[B_PIPE]])
;
; CHECK: define {{.*}} @bar
; CHECK: %[[A_PIPE1:.*]] = load {{.*}} @a
; CHECK: %[[B_PIPE1:.*]] = load {{.*}} @b
; CHECK: call void @sendData2{{.*}} %[[A_PIPE1]], {{.*}} %[[B_PIPE1]]
;
; CHECK: define {{.*}} @sendData(ptr addrspace(1) %ch)
; CHECK: %[[PIPE_ARG_ADDR:.*]] = alloca ptr addrspace(1)
; CHECK: store {{.*}} %ch, {{.*}} %[[PIPE_ARG_ADDR]]
; CHECK: %[[PIPE:.*]] = load {{.*}} %[[PIPE_ARG_ADDR]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[PIPE]]
;
; CHECK: define {{.*}} @sendData2(ptr addrspace(1) %a, ptr addrspace(1) %b, i32 noundef %f)
; CHECK: %[[PIPE_ARG0_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: %[[PIPE_ARG1_ADDR:.*]] = alloca ptr addrspace(1), align 8
; CHECK: store {{.*}} %a, {{.*}} %[[PIPE_ARG0_ADDR]]
; CHECK: store {{.*}} %b, {{.*}} %[[PIPE_ARG1_ADDR]]
; CHECK: %[[PIPE0:.*]] = load {{.*}} %[[PIPE_ARG0_ADDR]]
; CHECK: call i32 @__write_pipe{{.*}} %[[PIPE0]]
; CHECK: %[[PIPE1:.*]] = load {{.*}} %[[PIPE_ARG1_ADDR]]
; CHECK: call i32 @__write_pipe{{.*}} %[[PIPE1]]

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo(i32 noundef %f) #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !5 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !arg_type_null_val !5 {
entry:
  %f.addr = alloca i32, align 4
  store i32 %f, ptr %f.addr, align 4, !tbaa !10
  %0 = load i32, ptr %f.addr, align 4, !tbaa !10
  %cmp = icmp slt i32 %0, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %1 = load ptr addrspace(1), ptr addrspace(1) @a, align 8, !tbaa !14
  call void @sendData(ptr addrspace(1) %1) #3
  br label %if.end

if.else:                                          ; preds = %entry
  %2 = load ptr addrspace(1), ptr addrspace(1) @b, align 8, !tbaa !14
  call void @sendData(ptr addrspace(1) %2) #3
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @bar(i32 noundef %f) #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !5 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !arg_type_null_val !5 {
entry:
  %f.addr = alloca i32, align 4
  store i32 %f, ptr %f.addr, align 4, !tbaa !10
  %0 = load ptr addrspace(1), ptr addrspace(1) @a, align 8, !tbaa !14
  %1 = load ptr addrspace(1), ptr addrspace(1) @b, align 8, !tbaa !14
  %2 = load i32, ptr %f.addr, align 4, !tbaa !10
  call void @sendData2(ptr addrspace(1) %0, ptr addrspace(1) %1, i32 noundef %2) #3
  ret void
}

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @sendData(ptr addrspace(1) %ch) #1 !arg_type_null_val !15 {
entry:
  %ch.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %ch, ptr %ch.addr, align 8, !tbaa !14
  %0 = load ptr addrspace(1), ptr %ch.addr, align 8, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %0, i32 noundef 5) #3
  ret void
}

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1), i32 noundef) #2

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @sendData2(ptr addrspace(1) %a, ptr addrspace(1) %b, i32 noundef %f) #1 !arg_type_null_val !16 {
entry:
  %a.addr = alloca ptr addrspace(1), align 8
  %b.addr = alloca ptr addrspace(1), align 8
  %f.addr = alloca i32, align 4
  store ptr addrspace(1) %a, ptr %a.addr, align 8, !tbaa !14
  store ptr addrspace(1) %b, ptr %b.addr, align 8, !tbaa !14
  store i32 %f, ptr %f.addr, align 4, !tbaa !10
  %0 = load i32, ptr %f.addr, align 4, !tbaa !10
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %1 = load ptr addrspace(1), ptr %a.addr, align 8, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %1, i32 noundef 5) #3
  br label %cond.end

cond.false:                                       ; preds = %entry
  %2 = load ptr addrspace(1), ptr %b.addr, align 8, !tbaa !14
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %2, i32 noundef 5) #3
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  ret void
}

attributes #0 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" }
attributes #1 = { convergent noinline norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #2 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { convergent nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!4 = !{ptr @foo, ptr @bar}
!5 = !{i32 0}
!6 = !{!"none"}
!7 = !{!"int"}
!8 = !{!""}
!9 = !{i1 false}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!12, !12, i64 0}
!15 = !{target("spirv.Channel") zeroinitializer}
!16 = !{target("spirv.Channel") zeroinitializer, target("spirv.Channel") zeroinitializer, i32 0}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function sendData --  %write.src = alloca i32, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function sendData2 --  %write.src = alloca i32, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor --  call void @__pipe_init_fpga(ptr addrspace(1) @a.bs, i32 4, i32 0, i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor --  store ptr addrspace(1) @a.bs, ptr addrspace(1) @a, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor --  call void @__pipe_init_fpga(ptr addrspace(1) @b.bs, i32 4, i32 0, i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor --  store ptr addrspace(1) @b.bs, ptr addrspace(1) @b, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor --  ret void
; DEBUGIFY-NOT: WARNING
