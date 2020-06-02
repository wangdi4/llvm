; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
;
; channel int ch[4];
;
; __attribute__((noinline))
; void sendData1(channel int a) {
;   write_channel_intel(a, 2);
; }
;
; __attribute__((noinline))
; void sendData2(channel int a, channel int b) {
;   write_channel_intel(a, 3);
;   sendData1(b);
; }
;
; __attribute__((noinline))
; void sendData3(channel int a, channel int b, channel int c) {
;   write_channel_intel(a, 4);
;   sendData2(b, c);
; }
;
; __attribute__((noinline))
; void sendData4(channel int a, channel int b, channel int c, channel int d) {
;   write_channel_intel(a, 5);
;   sendData3(b, c, d);
; }
;
; __kernel void foo() {
;   sendData4(ch[0], ch[1], ch[2], ch[3]);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@ch = common addrspace(1) global [4 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4, !packet_size !0, !packet_align !0

; CHECK: define {{.*}} @foo
; CHECK: %[[PIPE0:.*]] = load {{.*}} getelementptr {{.*}} @ch.pipe, i64 0, i64 0
; CHECK: %[[PIPE1:.*]] = load {{.*}} getelementptr {{.*}} @ch.pipe, i64 0, i64 1
; CHECK: %[[PIPE2:.*]] = load {{.*}} getelementptr {{.*}} @ch.pipe, i64 0, i64 2
; CHECK: %[[PIPE3:.*]] = load {{.*}} getelementptr {{.*}} @ch.pipe, i64 0, i64 3
; CHECK: call void @sendData4({{.*}} %[[PIPE0]], {{.*}} %[[PIPE1]], {{.*}} %[[PIPE2]], {{.*}} %[[PIPE3]])
;
; CHECK: define {{.*}} @sendData4(%opencl.pipe_rw_t {{.*}}* %0, %opencl.pipe_rw_t {{.*}}* %1, %opencl.pipe_rw_t {{.*}}* %2, %opencl.pipe_rw_t {{.*}}* %3)
; CHECK: %[[SD4_PIPE_A_ADDR:.*]] = alloca %opencl.pipe_rw_t
; CHECK: %[[SD4_PIPE_B_ADDR:.*]] = alloca %opencl.pipe_rw_t
; CHECK: %[[SD4_PIPE_C_ADDR:.*]] = alloca %opencl.pipe_rw_t
; CHECK: %[[SD4_PIPE_D_ADDR:.*]] = alloca %opencl.pipe_rw_t
; CHECK: store {{.*}} %0, {{.*}} %[[SD4_PIPE_A_ADDR]]
; CHECK: store {{.*}} %1, {{.*}} %[[SD4_PIPE_B_ADDR]]
; CHECK: store {{.*}} %2, {{.*}} %[[SD4_PIPE_C_ADDR]]
; CHECK: store {{.*}} %3, {{.*}} %[[SD4_PIPE_D_ADDR]]
; CHECK: %[[SD4_PIPE:.*]] = load {{.*}} %[[SD4_PIPE_A_ADDR]]
; CHECK: %[[SD4_PIPE_BITCAST:.*]] = bitcast {{.*}} %[[SD4_PIPE]] to %opencl.pipe_wo_t
; CHECK: call i32 @__write_pipe_2{{.*}} %[[SD4_PIPE_BITCAST]]
; CHECK: %[[SD4_PIPE_B:.*]] = load {{.*}} %[[SD4_PIPE_B_ADDR]]
; CHECK: %[[SD4_PIPE_C:.*]] = load {{.*}} %[[SD4_PIPE_C_ADDR]]
; CHECK: %[[SD4_PIPE_D:.*]] = load {{.*}} %[[SD4_PIPE_D_ADDR]]
; CHECK: call void @sendData3({{.*}} %[[SD4_PIPE_B]], {{.*}} %[[SD4_PIPE_C]], {{.*}} %[[SD4_PIPE_D]])
;
; CHECK: define {{.*}} @sendData3(%opencl.pipe_rw_t {{.*}}* %0, %opencl.pipe_rw_t {{.*}}* %1, %opencl.pipe_rw_t {{.*}}* %2)
; CHECK: %[[SD3_PIPE_A_ADDR:.*]] = alloca %opencl.pipe_rw_t
; CHECK: %[[SD3_PIPE_B_ADDR:.*]] = alloca %opencl.pipe_rw_t
; CHECK: %[[SD3_PIPE_C_ADDR:.*]] = alloca %opencl.pipe_rw_t
; CHECK: store {{.*}} %0, {{.*}} %[[SD3_PIPE_A_ADDR]]
; CHECK: store {{.*}} %1, {{.*}} %[[SD3_PIPE_B_ADDR]]
; CHECK: store {{.*}} %2, {{.*}} %[[SD3_PIPE_C_ADDR]]
; CHECK: %[[SD3_PIPE:.*]] = load {{.*}} %[[SD3_PIPE_A_ADDR]]
; CHECK: %[[SD3_PIPE_BITCAST:.*]] = bitcast {{.*}} %[[SD3_PIPE]] to %opencl.pipe_wo_t
; CHECK: call i32 @__write_pipe_2{{.*}} %[[SD3_PIPE_BITCAST]]
; CHECK: %[[SD3_PIPE_B:.*]] = load {{.*}} %[[SD3_PIPE_B_ADDR]]
; CHECK: %[[SD3_PIPE_C:.*]] = load {{.*}} %[[SD3_PIPE_C_ADDR]]
; CHECK: call void @sendData2({{.*}} %[[SD3_PIPE_B]], {{.*}} %[[SD3_PIPE_C]])
;
; CHECK: define {{.*}} @sendData2(%opencl.pipe_rw_t {{.*}}* %0, %opencl.pipe_rw_t {{.*}}* %1)
; CHECK: %[[SD2_PIPE_A_ADDR:.*]] = alloca %opencl.pipe_rw_t
; CHECK: %[[SD2_PIPE_B_ADDR:.*]] = alloca %opencl.pipe_rw_t
; CHECK: store {{.*}} %0, {{.*}} %[[SD2_PIPE_A_ADDR]]
; CHECK: store {{.*}} %1, {{.*}} %[[SD2_PIPE_B_ADDR]]
; CHECK: %[[SD2_PIPE:.*]] = load {{.*}} %[[SD2_PIPE_A_ADDR]]
; CHECK: %[[SD2_PIPE_BITCAST:.*]] = bitcast {{.*}} %[[SD2_PIPE]] to %opencl.pipe_wo_t
; CHECK: call i32 @__write_pipe_2{{.*}} %[[SD2_PIPE_BITCAST]]
; CHECK: %[[SD2_PIPE_B:.*]] = load {{.*}} %[[SD2_PIPE_B_ADDR]]
; CHECK: call void @sendData1({{.*}} %[[SD2_PIPE_B]])
;
; CHECK: define {{.*}} @sendData1(%opencl.pipe_rw_t {{.*}}* %0)
; CHECK: %[[SD1_PIPE_A_ADDR:.*]] = alloca %opencl.pipe_rw_t
; CHECK: store {{.*}} %0, {{.*}} %[[SD1_PIPE_A_ADDR]]
; CHECK: %[[SD1_PIPE:.*]] = load {{.*}} %[[SD1_PIPE_A_ADDR]]
; CHECK: %[[SD1_PIPE_BITCAST:.*]] = bitcast {{.*}} %[[SD1_PIPE]] to %opencl.pipe_wo_t
; CHECK: call i32 @__write_pipe_2{{.*}} %[[SD1_PIPE_BITCAST]]

; Function Attrs: convergent noinline nounwind
define spir_func void @sendData1(%opencl.channel_t addrspace(1)* %a) #0 {
entry:
  %a.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  store %opencl.channel_t addrspace(1)* %a, %opencl.channel_t addrspace(1)** %a.addr, align 4, !tbaa !7
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %a.addr, align 4, !tbaa !7
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %0, i32 2) #3
  ret void
}

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #1

; Function Attrs: convergent noinline nounwind
define spir_func void @sendData2(%opencl.channel_t addrspace(1)* %a, %opencl.channel_t addrspace(1)* %b) #0 {
entry:
  %a.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  %b.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  store %opencl.channel_t addrspace(1)* %a, %opencl.channel_t addrspace(1)** %a.addr, align 4, !tbaa !7
  store %opencl.channel_t addrspace(1)* %b, %opencl.channel_t addrspace(1)** %b.addr, align 4, !tbaa !7
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %a.addr, align 4, !tbaa !7
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %0, i32 3) #3
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %b.addr, align 4, !tbaa !7
  call spir_func void @sendData1(%opencl.channel_t addrspace(1)* %1) #3
  ret void
}

; Function Attrs: convergent noinline nounwind
define spir_func void @sendData3(%opencl.channel_t addrspace(1)* %a, %opencl.channel_t addrspace(1)* %b, %opencl.channel_t addrspace(1)* %c) #0 {
entry:
  %a.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  %b.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  %c.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  store %opencl.channel_t addrspace(1)* %a, %opencl.channel_t addrspace(1)** %a.addr, align 4, !tbaa !7
  store %opencl.channel_t addrspace(1)* %b, %opencl.channel_t addrspace(1)** %b.addr, align 4, !tbaa !7
  store %opencl.channel_t addrspace(1)* %c, %opencl.channel_t addrspace(1)** %c.addr, align 4, !tbaa !7
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %a.addr, align 4, !tbaa !7
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %0, i32 4) #3
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %b.addr, align 4, !tbaa !7
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %c.addr, align 4, !tbaa !7
  call spir_func void @sendData2(%opencl.channel_t addrspace(1)* %1, %opencl.channel_t addrspace(1)* %2) #3
  ret void
}

; Function Attrs: convergent noinline nounwind
define spir_func void @sendData4(%opencl.channel_t addrspace(1)* %a, %opencl.channel_t addrspace(1)* %b, %opencl.channel_t addrspace(1)* %c, %opencl.channel_t addrspace(1)* %d) #0 {
entry:
  %a.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  %b.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  %c.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  %d.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  store %opencl.channel_t addrspace(1)* %a, %opencl.channel_t addrspace(1)** %a.addr, align 4, !tbaa !7
  store %opencl.channel_t addrspace(1)* %b, %opencl.channel_t addrspace(1)** %b.addr, align 4, !tbaa !7
  store %opencl.channel_t addrspace(1)* %c, %opencl.channel_t addrspace(1)** %c.addr, align 4, !tbaa !7
  store %opencl.channel_t addrspace(1)* %d, %opencl.channel_t addrspace(1)** %d.addr, align 4, !tbaa !7
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %a.addr, align 4, !tbaa !7
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %0, i32 5) #3
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %b.addr, align 4, !tbaa !7
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %c.addr, align 4, !tbaa !7
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %d.addr, align 4, !tbaa !7
  call spir_func void @sendData3(%opencl.channel_t addrspace(1)* %1, %opencl.channel_t addrspace(1)* %2, %opencl.channel_t addrspace(1)* %3) #3
  ret void
}

; Function Attrs: convergent nounwind
define spir_kernel void @foo() #2 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 !kernel_arg_host_accessible !5 {
entry:
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 0), align 4, !tbaa !7
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 1), align 4, !tbaa !7
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 2), align 4, !tbaa !7
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch, i64 0, i64 3), align 4, !tbaa !7
  call spir_func void @sendData4(%opencl.channel_t addrspace(1)* %0, %opencl.channel_t addrspace(1)* %1, %opencl.channel_t addrspace(1)* %2, %opencl.channel_t addrspace(1)* %3) #3
  ret void
}

attributes #0 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent }

!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!5}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}

!0 = !{i32 4}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 2, i32 0}
!5 = !{}
!6 = !{!"clang version 6.0.0 "}
!7 = !{!8, !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
