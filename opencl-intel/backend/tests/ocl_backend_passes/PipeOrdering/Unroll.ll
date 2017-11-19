; The source code of the program is below:
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch1;
; channel int ch_arr[4];
;
; __kernel void foo(__global int* iters) {
;   for (int i = 0; i < *iters; ++i) {
;     write_channel_intel(ch1, 42);
;     // implicit work-group barrier here
;   }
; }
;
; __kernel void boo() {
;   #pragma unroll
;   for (int i = 0; i < 4; ++i) {
;     write_channel_intel(ch_arr[i], 42);
;   }
; }
;
; Compile options: clang -cc1 -cl-std=CL2.0 -triple spir64-unknown-unknown-intelfpga -emit-llvm -x cl -disable-llvm-passes -finclude-default-header test.cl -o test.ll
; Optimizer options: oclopt -runtimelib=../..s/vectorizer/Full/runtime.bc -S -spir-materializer -channel-pipe-transformation test.ll -o test-opt.ll

; RUN: %oclopt -pipe-ordering -S %s | FileCheck %s
; REQUIRES: fpga-emulator

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

;; Check that barrier is not inserted if a loop has #pragma unroll

; CHECK:   define void @foo
; CHECK:     call void @_Z7barrierj(i32 1)
; CHECK:   define void @boo
; CHECK-NOT: call void @_Z7barrierj(i32 1)

%opencl.channel_t = type opaque
%opencl.pipe_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@ch1 = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@ch_arr = common addrspace(1) global [4 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4
@pipe.ch1 = common addrspace(1) global %opencl.pipe_t addrspace(1)* null, align 4
@pipe.ch_arr = common addrspace(1) global [4 x %opencl.pipe_t addrspace(1)*] zeroinitializer, align 4
@pipe.ch_arr.bs = common addrspace(1) global [5632 x i8] zeroinitializer, align 4
@pipe.ch1.bs = common addrspace(1) global [1408 x i8] zeroinitializer, align 4
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__global_pipes_ctor, i8* null }]

; Function Attrs: nounwind
define void @foo(i32 addrspace(1)* %iters) #0 !kernel_arg_addr_space !9 !kernel_arg_access_qual !10 !kernel_arg_type !11 !kernel_arg_base_type !11 !kernel_arg_type_qual !12 !kernel_arg_host_accessible !13 {
entry:
  %write.src = alloca i32
  %iters.addr = alloca i32 addrspace(1)*, align 8
  %i = alloca i32, align 4
  store i32 addrspace(1)* %iters, i32 addrspace(1)** %iters.addr, align 8, !tbaa !14
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 0, i32* %i, align 4, !tbaa !18
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4, !tbaa !18
  %2 = load i32 addrspace(1)*, i32 addrspace(1)** %iters.addr, align 8, !tbaa !14
  %3 = load i32, i32 addrspace(1)* %2, align 4, !tbaa !18
  %cmp = icmp slt i32 %1, %3
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %4 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #3
  br label %for.end

for.body:                                         ; preds = %for.cond
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch1, align 4, !tbaa !20
  store i32 42, i32* %write.src
  %6 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)* addrspace(1)* @pipe.ch1
  %7 = bitcast %opencl.pipe_t addrspace(1)* %6 to %struct.__pipe_t addrspace(1)*
  %8 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %9 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %7, i8 addrspace(4)* %8)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %10 = load i32, i32* %i, align 4, !tbaa !18
  %inc = add nsw i32 %10, 1
  store i32 %inc, i32* %i, align 4, !tbaa !18
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
define void @boo() #0 !kernel_arg_addr_space !6 !kernel_arg_access_qual !6 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !6 !kernel_arg_host_accessible !6 {
entry:
  %write.src = alloca i32
  %i = alloca i32, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 0, i32* %i, align 4, !tbaa !18
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4, !tbaa !18
  %cmp = icmp slt i32 %1, 4
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %2 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #3
  br label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load i32, i32* %i, align 4, !tbaa !18
  %idxprom = sext i32 %3 to i64
  %arrayidx = getelementptr inbounds [4 x %opencl.channel_t addrspace(1)*], [4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch_arr, i64 0, i64 %idxprom
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx, align 4, !tbaa !20
  store i32 42, i32* %write.src
  %5 = getelementptr [4 x %opencl.pipe_t addrspace(1)*], [4 x %opencl.pipe_t addrspace(1)*] addrspace(1)* @pipe.ch_arr, i64 0, i64 %idxprom
  %6 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)* addrspace(1)* %5
  %7 = bitcast %opencl.pipe_t addrspace(1)* %6 to %struct.__pipe_t addrspace(1)*
  %8 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %9 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %7, i8 addrspace(4)* %8)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %10 = load i32, i32* %i, align 4, !tbaa !18
  %inc = add nsw i32 %10, 1
  store i32 %inc, i32* %i, align 4, !tbaa !18
  br label %for.cond, !llvm.loop !21

for.end:                                          ; preds = %for.cond.cleanup
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_intel(%struct.__pipe_t addrspace(1)*, i32, i32) #2

; Function Attrs: nounwind readnone
declare void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)*, i32, i32, i32) #2

define void @__global_pipes_ctor() {
entry:
  store %opencl.pipe_t addrspace(1)* bitcast ([5632 x i8] addrspace(1)* @pipe.ch_arr.bs to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.pipe_t addrspace(1)*], [4 x %opencl.pipe_t addrspace(1)*] addrspace(1)* @pipe.ch_arr, i32 0, i32 0)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([5632 x i8], [5632 x i8] addrspace(1)* @pipe.ch_arr.bs, i64 0, i64 1408) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.pipe_t addrspace(1)*], [4 x %opencl.pipe_t addrspace(1)*] addrspace(1)* @pipe.ch_arr, i32 0, i32 1)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([5632 x i8], [5632 x i8] addrspace(1)* @pipe.ch_arr.bs, i64 0, i64 2816) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.pipe_t addrspace(1)*], [4 x %opencl.pipe_t addrspace(1)*] addrspace(1)* @pipe.ch_arr, i32 0, i32 2)
  store %opencl.pipe_t addrspace(1)* bitcast (i8 addrspace(1)* getelementptr inbounds ([5632 x i8], [5632 x i8] addrspace(1)* @pipe.ch_arr.bs, i64 0, i64 4224) to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* getelementptr inbounds ([4 x %opencl.pipe_t addrspace(1)*], [4 x %opencl.pipe_t addrspace(1)*] addrspace(1)* @pipe.ch_arr, i32 0, i32 3)
  call void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)* bitcast ([4 x %opencl.pipe_t addrspace(1)*] addrspace(1)* @pipe.ch_arr to %struct.__pipe_t addrspace(1)* addrspace(1)*), i32 4, i32 4, i32 1)
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @pipe.ch1.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 1)
  store %opencl.pipe_t addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @pipe.ch1.bs to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* @pipe.ch1
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture) #2

declare i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*)

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture readonly) #2

declare i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*)

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind }

!opencl.channels = !{!0, !3}
!llvm.module.flags = !{!4}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!5}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}
!opencl.kernels = !{!8}

!0 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @ch1, !1, !2}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{[4 x %opencl.channel_t addrspace(1)*] addrspace(1)* @ch_arr, !1, !2}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 2, i32 0}
!6 = !{}
!7 = !{!"clang version 6.0.0 (/home/asavonic/dev/xmain/xmain/llvm/tools/clang a191343ca5d82f7194b7b97c5f4c613fd01333ab) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 9bc194ffdbe384130f50a5feb10bbc197f7a5664)"}
!8 = !{void (i32 addrspace(1)*)* @foo, void ()* @boo}
!9 = !{i32 1}
!10 = !{!"none"}
!11 = !{!"int*"}
!12 = !{!""}
!13 = !{i1 false}
!14 = !{!15, !15, i64 0}
!15 = !{!"any pointer", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !{!19, !19, i64 0}
!19 = !{!"int", !16, i64 0}
!20 = !{!16, !16, i64 0}
!21 = distinct !{!21, !22}
!22 = !{!"llvm.loop.unroll.enable"}
