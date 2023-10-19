; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
; channel int a;
;
; __attribute__((noinline))
; int sendOne(channel int ch) {
;   write_channel_intel(ch, 1);
;   return 1;
; }
;
; __kernel void foo(__global int *data) {
;   data[0] = sendOne(a);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; opt -passes=sycl-kernel-target-ext-type-lower,sycl-kernel-equalizer %s -S
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@a = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0

; CHECK: define {{.*}} @foo
; CHECK: %[[PIPE:.*]] = load {{.*}} @a
; CHECK: %[[CALL_RESULT:.*]] = call {{.*}} @sendOne({{.*}} %[[PIPE]])
; CHECK: store i32 %[[CALL_RESULT]]

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo(ptr addrspace(1) noundef align 4 %data) #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !8 !kernel_arg_buffer_location !8 !arg_type_null_val !11 {
entry:
  %data.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %data, ptr %data.addr, align 8, !tbaa !12
  %0 = load ptr addrspace(1), ptr addrspace(1) @a, align 4, !tbaa !16
  %call = call i32 @sendOne(ptr addrspace(1) %0) #3
  %1 = load ptr addrspace(1), ptr %data.addr, align 8, !tbaa !12
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %1, i64 0
  store i32 %call, ptr addrspace(1) %arrayidx, align 4, !tbaa !17
  ret void
}

; Function Attrs: convergent noinline norecurse nounwind
define dso_local i32 @sendOne(ptr addrspace(1) %ch) #1 !arg_type_null_val !19 {
entry:
  %ch.addr = alloca ptr addrspace(1), align 4
  store ptr addrspace(1) %ch, ptr %ch.addr, align 4, !tbaa !16
  %0 = load ptr addrspace(1), ptr %ch.addr, align 4, !tbaa !16
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %0, i32 noundef 1) #3
  ret i32 1
}

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1), i32 noundef) #2

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
!4 = !{ptr @foo}
!5 = !{i32 1}
!6 = !{!"none"}
!7 = !{!"int*"}
!8 = !{!""}
!9 = !{i1 false}
!10 = !{i32 0}
!11 = !{ptr addrspace(1) null}
!12 = !{!13, !13, i64 0}
!13 = !{!"any pointer", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = !{!14, !14, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"int", !14, i64 0}
!19 = !{target("spirv.Channel") zeroinitializer}

; DEBUGIFY-NOT: WARNING: Missing line
