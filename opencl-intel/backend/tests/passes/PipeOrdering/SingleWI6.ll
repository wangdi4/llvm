; The source code of the program is below:
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch;
;
; __attribute__ ((noinline)) void foo(int iters) {
;   for (int i = 0; i < iters; ++i) {
;     write_channel_intel(ch, 42);
;   }
; }
;
; __attribute__ ((noinline)) void boo(int iters) {
;   foo(iters);
; }
;
; __attribute__ ((task)) __kernel void moo(__global int *iters) {
;   boo(*iters);
; }
; Compile options: clang -cc1 -cl-std=CL2.0 -triple spir64-unknown-unknown-intelfpga -emit-llvm -x cl -disable-llvm-passes -finclude-default-header test.cl -o test.ll
; Optimizer options: oclopt -runtimelib=../..s/vectorizer/Full/runtime.bc -S -channel-pipe-transformation -spir-materializer test.ll -o test-opt.ll

; RUN: %oclopt -pipe-ordering -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
; REQUIRES: fpga-emulator

%opencl.channel_t = type opaque
%opencl.pipe_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@ch = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@pipe.ch = common addrspace(1) global %opencl.pipe_t addrspace(1)* null, align 4
@pipe.ch.bs = common addrspace(1) global [388 x i8] zeroinitializer, align 4
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__global_pipes_ctor, i8* null }]

; Function Attrs: noinline nounwind
define void @foo(i32 %iters) #0 {
entry:
  %write.src = alloca i32
  %iters.addr = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %iters, i32* %iters.addr, align 4, !tbaa !8
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #4
  store i32 0, i32* %i, align 4, !tbaa !8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4, !tbaa !8
  %2 = load i32, i32* %iters.addr, align 4, !tbaa !8
  %cmp = icmp slt i32 %1, %2
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %3 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %3) #4
  br label %for.end

for.body:                                         ; preds = %for.cond
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch, align 4, !tbaa !12
  store i32 42, i32* %write.src
  %5 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)* addrspace(1)* @pipe.ch
  %6 = bitcast %opencl.pipe_t addrspace(1)* %5 to %struct.__pipe_t addrspace(1)*
  %7 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %8 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %6, i8 addrspace(4)* %7)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %9 = load i32, i32* %i, align 4, !tbaa !8
  %inc = add nsw i32 %9, 1
  store i32 %inc, i32* %i, align 4, !tbaa !8
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  ret void
}
; CHECK: define void @foo(i32 %iters)
; CHECK-LABEL: for.cond:
; CHECK-NOT: call void @_Z7barrierj(i32 1)
; CHECK: br i1 %cmp, label %for.body, label %for.cond.cleanup
; CHECK-NOT: declare void @_Z7barrierj(i32)

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: noinline nounwind
define void @boo(i32 %iters) #0 {
entry:
  %iters.addr = alloca i32, align 4
  store i32 %iters, i32* %iters.addr, align 4, !tbaa !8
  %0 = load i32, i32* %iters.addr, align 4, !tbaa !8
  call void @foo(i32 %0)
  ret void
}

; Function Attrs: nounwind
define void @moo(i32 addrspace(1)* %iters) #2 !kernel_arg_addr_space !13 !kernel_arg_access_qual !14 !kernel_arg_type !15 !kernel_arg_base_type !15 !kernel_arg_type_qual !16 !task !17 {
entry:
  %iters.addr = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %iters, i32 addrspace(1)** %iters.addr, align 8, !tbaa !18
  %0 = load i32 addrspace(1)*, i32 addrspace(1)** %iters.addr, align 8, !tbaa !18
  %1 = load i32, i32 addrspace(1)* %0, align 4, !tbaa !8
  call void @boo(i32 %1)
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_intel(%struct.__pipe_t addrspace(1)*, i32, i32) #3

; Function Attrs: nounwind readnone
declare void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)*, i32, i32, i32) #3

define void @__global_pipes_ctor() {
entry:
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([388 x i8] addrspace(1)* @pipe.ch.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 1)
  store %opencl.pipe_t addrspace(1)* bitcast ([388 x i8] addrspace(1)* @pipe.ch.bs to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* @pipe.ch
  ret void
}

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture) #3

declare i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*)

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture readonly) #3

declare i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*)

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind }

!opencl.channels = !{!0}
!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!5}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}
!opencl.kernels = !{!7}

!0 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @ch, !1, !2}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 2, i32 0}
!5 = !{}
!6 = !{!"clang version 5.0.0 (cfe/trunk)"}
!7 = !{void (i32 addrspace(1)*)* @moo}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!10, !10, i64 0}
!13 = !{i32 1}
!14 = !{!"none"}
!15 = !{!"int*"}
!16 = !{!""}
!17 = !{i1 true}
!18 = !{!19, !19, i64 0}
!19 = !{!"any pointer", !10, i64 0}
