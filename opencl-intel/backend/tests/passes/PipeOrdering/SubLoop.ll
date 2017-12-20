; The source code of the program is below:
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; channel int ch;
;
; __kernel void foo(__global int* iters) {
;   for (int i = 0; i < *iters; ++i) {
;     for (int j = 0; j < (i*i+1); ++j) {
;       write_channel_intel(ch, 42);
;       // implicit work-group barrier here
;     }
;   }
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

; function attrs: nounwind
define void @foo(i32 addrspace(1)* %iters) #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 {
entry:
  %write.src = alloca i32
  %iters.addr = alloca i32 addrspace(1)*, align 8
  %i = alloca i32, align 4
  %cleanup.dest.slot = alloca i32
  %j = alloca i32, align 4
  store i32 addrspace(1)* %iters, i32 addrspace(1)** %iters.addr, align 8, !tbaa !12
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  store i32 0, i32* %i, align 4, !tbaa !16
  br label %for.cond

for.cond:                                         ; preds = %for.inc5, %entry
  %1 = load i32, i32* %i, align 4, !tbaa !16
  %2 = load i32 addrspace(1)*, i32 addrspace(1)** %iters.addr, align 8, !tbaa !12
  %3 = load i32, i32 addrspace(1)* %2, align 4, !tbaa !16
  %cmp = icmp slt i32 %1, %3
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  store i32 2, i32* %cleanup.dest.slot, align 4
  %4 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #3
  br label %for.end7

for.body:                                         ; preds = %for.cond
  %5 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #3
  store i32 0, i32* %j, align 4, !tbaa !16
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %6 = load i32, i32* %j, align 4, !tbaa !16
  %7 = load i32, i32* %i, align 4, !tbaa !16
  %8 = load i32, i32* %i, align 4, !tbaa !16
  %mul = mul nsw i32 %7, %8
  %add = add nsw i32 %mul, 1
  %cmp2 = icmp slt i32 %6, %add
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3
; CHECK-LABEL: for.cond1:
; CHECK: call void @_Z7barrierj(i32 1)
; CHECK: br i1 %cmp2, label %for.body4, label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond1
  store i32 5, i32* %cleanup.dest.slot, align 4
  %9 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %9) #3
  br label %for.end

for.body4:                                        ; preds = %for.cond1
  %10 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ch, align 4, !tbaa !18
  store i32 42, i32* %write.src
  %11 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)* addrspace(1)* @pipe.ch
  %12 = bitcast %opencl.pipe_t addrspace(1)* %11 to %struct.__pipe_t addrspace(1)*
  %13 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %14 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %12, i8 addrspace(4)* %13)
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %15 = load i32, i32* %j, align 4, !tbaa !16
  %inc = add nsw i32 %15, 1
  store i32 %inc, i32* %j, align 4, !tbaa !16
  br label %for.cond1

for.end:                                          ; preds = %for.cond.cleanup3
  br label %for.inc5

for.inc5:                                         ; preds = %for.end
  %16 = load i32, i32* %i, align 4, !tbaa !16
  %inc6 = add nsw i32 %16, 1
  store i32 %inc6, i32* %i, align 4, !tbaa !16
  br label %for.cond

for.end7:                                         ; preds = %for.cond.cleanup
  ret void
}

; function attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; function attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; function attrs: nounwind readnone
declare void @__pipe_init_intel(%struct.__pipe_t addrspace(1)*, i32, i32) #2

; function attrs: nounwind readnone
declare void @__pipe_init_array_intel(%struct.__pipe_t addrspace(1)* addrspace(1)*, i32, i32, i32) #2

define void @__global_pipes_ctor() {
entry:
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([388 x i8] addrspace(1)* @pipe.ch.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 1)
  store %opencl.pipe_t addrspace(1)* bitcast ([388 x i8] addrspace(1)* @pipe.ch.bs to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* @pipe.ch
  ret void
}

; function attrs: nounwind readnone
declare i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture) #2

declare i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*)

; function attrs: nounwind readnone
declare i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture readonly) #2

declare i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*)

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind }

!opencl.channels = !{!0}
!llvm.module.flags = !{!3}
!opencl.enable.fp_contract = !{}
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
!7 = !{void (i32 addrspace(1)*)* @foo}
!8 = !{i32 1}
!9 = !{!"none"}
!10 = !{!"int*"}
!11 = !{!""}
!12 = !{!13, !13, i64 0}
!13 = !{!"any pointer", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"simple c/c++ tbaa"}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !14, i64 0}
!18 = !{!14, !14, i64 0}
