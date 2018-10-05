; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int bar;
; channel int far;
;
; channel float a;
; channel float b;
;
; __kernel void foo(__global int *data) {
;   int i = read_channel_intel(bar);
;   bool v;
;   int j = read_channel_nb_intel(far, &v);
;
;   data[0] = i;
;   data[1] = j;
; }
;
; __kernel void foobar() {
;   float p = 3.14;
;   float e = 2.7;
;
;   write_channel_intel(a, p);
;   write_channel_nb_intel(b, e);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@a = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0
@b = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0

; CHECK: define {{.*}} @foo
; CHECK: %[[READ_DST_ALLOCA:read.dst.*]] = alloca i32
; CHECK: %[[BAR_PIPE_LOAD:.*]] = load {{.*}} @bar.pipe
; CHECK: %[[BAR_PIPE:.*]] = bitcast {{.*}} %[[BAR_PIPE_LOAD]]
; CHECK: %[[READ_DST:.*]] = addrspacecast {{.*}} %[[READ_DST_ALLOCA]]
; CHECK: call i32 @__read_pipe_2{{.*}}({{.*}} %[[BAR_PIPE]], {{.*}} %[[READ_DST]]
; CHECK: load {{.*}} %[[READ_DST_ALLOCA]]
;
; CHECK: %[[FAR_PIPE_LOAD:.*]] = load {{.*}} @far.pipe
; CHECK: %[[FAR_PIPE:.*]] = bitcast {{.*}} %[[FAR_PIPE_LOAD]]
; CHECK: %[[READ_DST1:.*]] = addrspacecast {{.*}} %[[READ_DST_ALLOCA]]
; CHECK: call i32 @__read_pipe_2{{.*}}({{.*}} %[[FAR_PIPE]], {{.*}} %[[READ_DST1]]
; CHECK: load {{.*}} %[[READ_DST_ALLOCA]]
;
; CHECK: define {{.*}} @foobar
; CHECK: %[[WRITE_SRC_ALLOCA:write.src.*]] = alloca float
; CHECK: %[[A_PIPE_LOAD:.*]] = load {{.*}} @a.pipe
; CHECK: store {{.*}} %[[WRITE_SRC_ALLOCA]]
; CHECK: %[[A_PIPE:.*]] = bitcast {{.*}} %[[A_PIPE_LOAD]]
; CHECK: %[[WRITE_SRC:.*]] = addrspacecast {{.*}} %[[WRITE_SRC_ALLOCA]]
; CHECK: call i32 @__write_pipe_2{{.*}}({{.*}} %[[A_PIPE]], {{.*}} %[[WRITE_SRC]]
;
; CHECK: %[[B_PIPE_LOAD:.*]] = load {{.*}} @b.pipe
; CHECK: store {{.*}} %[[WRITE_SRC_ALLOCA]]
; CHECK: %[[B_PIPE:.*]] = bitcast {{.*}} %[[B_PIPE_LOAD]]
; CHECK: %[[WRITE_SRC1:.*]] = addrspacecast {{.*}} %[[WRITE_SRC_ALLOCA]]
; CHECK: call i32 @__write_pipe_2{{.*}}({{.*}} %[[B_PIPE]], {{.*}} %[[WRITE_SRC1]]

; Function Attrs: convergent nounwind
define spir_kernel void @foo(i32 addrspace(1)* %data) #0 !kernel_arg_addr_space !10 !kernel_arg_access_qual !11 !kernel_arg_type !12 !kernel_arg_base_type !12 !kernel_arg_type_qual !13 !kernel_arg_host_accessible !14 {
entry:
  %data.addr = alloca i32 addrspace(1)*, align 8
  %i = alloca i32, align 4
  %v = alloca i8, align 1
  %j = alloca i32, align 4
  store i32 addrspace(1)* %data, i32 addrspace(1)** %data.addr, align 8, !tbaa !15
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !19
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %1) #4
  store i32 %call, i32* %i, align 4, !tbaa !20
  call void @llvm.lifetime.start.p0i8(i64 1, i8* %v) #3
  %2 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #3
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 4, !tbaa !19
  %4 = addrspacecast i8* %v to i8 addrspace(4)*
  %call1 = call i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(%opencl.channel_t addrspace(1)* %3, i8 addrspace(4)* %4) #4
  store i32 %call1, i32* %j, align 4, !tbaa !20
  %5 = load i32, i32* %i, align 4, !tbaa !20
  %6 = load i32 addrspace(1)*, i32 addrspace(1)** %data.addr, align 8, !tbaa !15
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %6, i64 0
  store i32 %5, i32 addrspace(1)* %arrayidx, align 4, !tbaa !20
  %7 = load i32, i32* %j, align 4, !tbaa !20
  %8 = load i32 addrspace(1)*, i32 addrspace(1)** %data.addr, align 8, !tbaa !15
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %8, i64 1
  store i32 %7, i32 addrspace(1)* %arrayidx2, align 4, !tbaa !20
  %9 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %9) #3
  call void @llvm.lifetime.end.p0i8(i64 1, i8* %v) #3
  %10 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent
declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)*) #2

; Function Attrs: convergent
declare i32 @_Z21read_channel_nb_intel11ocl_channeliPU3AS4b(%opencl.channel_t addrspace(1)*, i8 addrspace(4)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind
define spir_kernel void @foobar() #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !8 !kernel_arg_type !8 !kernel_arg_base_type !8 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !8 {
entry:
  %p = alloca float, align 4
  %e = alloca float, align 4
  %0 = bitcast float* %p to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store float 0x40091EB860000000, float* %p, align 4, !tbaa !22
  %1 = bitcast float* %e to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #3
  store float 0x40059999A0000000, float* %e, align 4, !tbaa !22
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @a, align 4, !tbaa !19
  %3 = load float, float* %p, align 4, !tbaa !22
  call void @_Z19write_channel_intel11ocl_channelff(%opencl.channel_t addrspace(1)* %2, float %3) #4
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @b, align 4, !tbaa !19
  %5 = load float, float* %e, align 4, !tbaa !22
  %call = call zeroext i1 @_Z22write_channel_nb_intel11ocl_channelff(%opencl.channel_t addrspace(1)* %4, float %5) #4
  %6 = bitcast float* %e to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %6) #3
  %7 = bitcast float* %p to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %7) #3
  ret void
}

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelff(%opencl.channel_t addrspace(1)*, float) #2

; Function Attrs: convergent
declare zeroext i1 @_Z22write_channel_nb_intel11ocl_channelff(%opencl.channel_t addrspace(1)*, float) #2

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent }

!llvm.module.flags = !{!6}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!7}
!opencl.spir.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!llvm.ident = !{!9}

!0 = !{i32 4}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{i32 2, i32 0}
!8 = !{}
!9 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang f0b0ffae999eb5a17553f9cb2287272993ef4133) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm fcec820f4b80de4ed49d51d31292f09593b932e6)"}
!10 = !{i32 1}
!11 = !{!"none"}
!12 = !{!"int*"}
!13 = !{!""}
!14 = !{i1 false}
!15 = !{!16, !16, i64 0}
!16 = !{!"any pointer", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C/C++ TBAA"}
!19 = !{!17, !17, i64 0}
!20 = !{!21, !21, i64 0}
!21 = !{!"int", !17, i64 0}
!22 = !{!23, !23, i64 0}
!23 = !{!"float", !17, i64 0}
