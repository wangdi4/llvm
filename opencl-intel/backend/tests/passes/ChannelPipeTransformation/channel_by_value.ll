; Compiled from:
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
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@a = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@b = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0

; CHECK: define {{.*}} @foo
; CHECK: %[[A_PIPE:.*]] = load {{.*}} @a.pipe
; CHECK: call void @sendData({{.*}} %[[A_PIPE]])
; CHECK: %[[B_PIPE:.*]] = load {{.*}} @b.pipe
; CHECK: call void @sendData({{.*}} %[[B_PIPE]])
;
; CHECK: define {{.*}} @bar
; CHECK: %[[A_PIPE1:.*]] = load {{.*}} @a.pipe
; CHECK: %[[B_PIPE1:.*]] = load {{.*}} @b.pipe
; CHECK: call void @sendData2{{.*}} %[[A_PIPE1]], {{.*}} %[[B_PIPE1]]
;
; CHECK: define {{.*}} @sendData(%opencl.pipe_rw_t {{.*}}* %0)
; CHECK: %[[PIPE_ARG_ADDR:.*]] = alloca %opencl.pipe_rw_t {{.*}}*
; CHECK: store {{.*}} %0, {{.*}} %[[PIPE_ARG_ADDR]]
; CHECK: %[[PIPE:.*]] = load {{.*}} %[[PIPE_ARG_ADDR]]
; CHECK: %[[PIPE_BITCAST:.*]] = bitcast {{.*}} %[[PIPE]] to %opencl.pipe_wo_t
; CHECK: call i32 @__write_pipe_2{{.*}} %[[PIPE_BITCAST]]
;
; CHECK: define {{.*}} @sendData2(%opencl.pipe_rw_t {{.*}}* %0, %opencl.pipe_rw_t {{.*}}* %1, i32 %2)
; CHECK: %[[PIPE_ARG0_ADDR:.*]] = alloca %opencl.pipe_rw_t {{.*}}*
; CHECK: %[[PIPE_ARG1_ADDR:.*]] = alloca %opencl.pipe_rw_t {{.*}}*
; CHECK: store {{.*}} %0, {{.*}} %[[PIPE_ARG0_ADDR]]
; CHECK: store {{.*}} %1, {{.*}} %[[PIPE_ARG1_ADDR]]
; CHECK: %[[PIPE0:.*]] = load {{.*}} %[[PIPE_ARG0_ADDR]]
; CHECK: %[[PIPE0_BITCAST:.*]] = bitcast {{.*}} %[[PIPE0]] to %opencl.pipe_wo_t
; CHECK: call i32 @__write_pipe{{.*}} %[[PIPE0_BITCAST]]
; CHECK: %[[PIPE1:.*]] = load {{.*}} %[[PIPE_ARG1_ADDR]]
; CHECK: %[[PIPE1_BITCAST:.*]] = bitcast {{.*}} %[[PIPE1]] to %opencl.pipe_wo_t
; CHECK: call i32 @__write_pipe{{.*}} %[[PIPE1_BITCAST]]

; Function Attrs: convergent noinline nounwind
define spir_func void @sendData(%opencl.channel_t addrspace(1)* %ch) #0 {
entry:
  %ch.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  store %opencl.channel_t addrspace(1)* %ch, %opencl.channel_t addrspace(1)** %ch.addr, align 4, !tbaa !8
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %ch.addr, align 4, !tbaa !8
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %0, i32 5) #3
  ret void
}

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #1

; Function Attrs: convergent nounwind
define spir_kernel void @foo(i32 %f) #2 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 {
entry:
  %f.addr = alloca i32, align 4
  store i32 %f, i32* %f.addr, align 4, !tbaa !16
  %0 = load i32, i32* %f.addr, align 4, !tbaa !16
  %cmp = icmp slt i32 %0, 0
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @a, align 4, !tbaa !8
  call spir_func void @sendData(%opencl.channel_t addrspace(1)* %1) #3
  br label %if.end

if.else:                                          ; preds = %entry
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @b, align 4, !tbaa !8
  call spir_func void @sendData(%opencl.channel_t addrspace(1)* %2) #3
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  ret void
}

; Function Attrs: convergent noinline nounwind
define spir_func void @sendData2(%opencl.channel_t addrspace(1)* %a, %opencl.channel_t addrspace(1)* %b, i32 %f) #0 {
entry:
  %a.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  %b.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  %f.addr = alloca i32, align 4
  store %opencl.channel_t addrspace(1)* %a, %opencl.channel_t addrspace(1)** %a.addr, align 4, !tbaa !8
  store %opencl.channel_t addrspace(1)* %b, %opencl.channel_t addrspace(1)** %b.addr, align 4, !tbaa !8
  store i32 %f, i32* %f.addr, align 4, !tbaa !16
  %0 = load i32, i32* %f.addr, align 4, !tbaa !16
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %a.addr, align 4, !tbaa !8
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %1, i32 5) #3
  br label %cond.end

cond.false:                                       ; preds = %entry
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %b.addr, align 4, !tbaa !8
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %2, i32 5) #3
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  ret void
}

; Function Attrs: convergent nounwind
define spir_kernel void @bar(i32 %f) #2 !kernel_arg_addr_space !11 !kernel_arg_access_qual !12 !kernel_arg_type !13 !kernel_arg_base_type !13 !kernel_arg_type_qual !14 !kernel_arg_host_accessible !15 {
entry:
  %f.addr = alloca i32, align 4
  store i32 %f, i32* %f.addr, align 4, !tbaa !16
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @a, align 4, !tbaa !8
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @b, align 4, !tbaa !8
  %2 = load i32, i32* %f.addr, align 4, !tbaa !16
  call spir_func void @sendData2(%opencl.channel_t addrspace(1)* %0, %opencl.channel_t addrspace(1)* %1, i32 %2) #3
  ret void
}

attributes #0 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent }

!llvm.module.flags = !{!4}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 4}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 2, i32 0}
!6 = !{}
!7 = !{!"clang version 6.0.0 "}
!8 = !{!9, !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{i32 0}
!12 = !{!"none"}
!13 = !{!"int"}
!14 = !{!""}
!15 = !{i1 false}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !9, i64 0}
