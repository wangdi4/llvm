; The source code of the program is below:
;
; #pragma OPENCL EXTENSION cl_intel_channels : enable
;
; __kernel void k1(read_only pipe int p1 __attribute__((io("test1"))),
;                  write_only pipe int p2 __attribute__((io("test2"))),
;                  int i1, global float * f1) {
;     int data;
;     int i = read_pipe(p1, &data);
;     if (i > 0)
;         write_pipe(p2, &data);
; }
;
; channel int ichIn __attribute__((io("test_ch_in")));
; channel int ichOut __attribute__((io("test_ch_out")));
;
; __kernel void in_kernel() {
;   long data;
;   data = read_channel_intel(ichIn);
;   if (data > 0)
;         data = write_channel_intel(ichOut);
; }
;
; Compile options: clang -cc1 -triple spir-unknown-unknown-intelfpga -emit-llvm test.cl -o test.ll
; Optimizer options: oclopt -runtimelib=../../vectorizer/Full/runtime.bc -S -channel-pipe-transformation -spir-materializer test.ll -o test-opt.ll
; REQUIRES: fpga-emulator

; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -pipe-io-transformation -verify %s -S | FileCheck %s

%opencl.channel_t = type opaque
%opencl.pipe_t = type opaque
%struct.__pipe_t = type { i32, i32, i32, i32, i32, i32, [0 x i8] }

@ichIn = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !io !1
@ichOut = common local_unnamed_addr addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !io !2
@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_ctor, i8* null }]
@ichIn.pipe = addrspace(1) global %opencl.pipe_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !io !1
@ichIn.pipe.bs = addrspace(1) global [1408 x i8] zeroinitializer, align 4
@ichOut.pipe = addrspace(1) global %opencl.pipe_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0, !io !2
@ichOut.pipe.bs = addrspace(1) global [1408 x i8] zeroinitializer, align 4
; CHECK: @llvm.global_dtors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @__pipe_global_dtor, i8* null }]
; CHECK: @test_ch_in.str = private unnamed_addr constant [11 x i8] c"test_ch_in\00", align 1
; CHECK: @test1.str = private unnamed_addr constant [6 x i8] c"test1\00", align 1
; CHECK: @test2.str = private unnamed_addr constant [6 x i8] c"test2\00", align 1

; Function Attrs: nounwind
; CHECK: define void @k1
define void @k1(%opencl.pipe_t addrspace(1)* %p1, %opencl.pipe_t addrspace(1)* %p2, i32 %i1, float addrspace(1)* nocapture readnone %f1) local_unnamed_addr #0 !kernel_arg_addr_space !9 !kernel_arg_access_qual !10 !kernel_arg_type !11 !kernel_arg_base_type !11 !kernel_arg_type_qual !12 !kernel_arg_host_accessible !13 !kernel_arg_pipe_depth !14 !kernel_arg_pipe_io !15 !kernel_arg_buffer_location !16 {
entry:
  %data = alloca i32, align 4
  %0 = bitcast i32* %data to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #4
  %1 = bitcast %opencl.pipe_t addrspace(1)* %p1 to %struct.__pipe_t addrspace(1)*
  %2 = addrspacecast i8* %0 to i8 addrspace(4)*
  %3 = call i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)* %1, i8 addrspace(4)* nonnull %2) #4
  %cmp = icmp sgt i32 %3, 0
  br i1 %cmp, label %if.then, label %if.end
; CHECK: %[[TEST1STR:[0-9]+]] = addrspacecast [6 x i8]* @test1.str to i8 addrspace(4)*
; CHECK: %[[RET:[0-9]+]] = call i32 @__read_pipe_2_io_intel(%struct.__pipe_t addrspace(1)* %1, i8 addrspace(4)* %2, i8 addrspace(4)* %[[TEST1STR]])
; CHECK: %cmp = icmp sgt i32 %[[RET]], 0

if.then:                                          ; preds = %entry
  %4 = bitcast %opencl.pipe_t addrspace(1)* %p2 to %struct.__pipe_t addrspace(1)*
  %5 = addrspacecast i8* %0 to i8 addrspace(4)*
  %6 = call i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)* %4, i8 addrspace(4)* nonnull %5) #4
  br label %if.end
; CHECK: %[[TEST2STR:[0-9]+]] = addrspacecast [6 x i8]* @test2.str to i8 addrspace(4)*
; CHECK: %{{[0-9]+}} = call i32 @__write_pipe_2_io_intel(%struct.__pipe_t addrspace(1)* %5, i8 addrspace(4)* %6, i8 addrspace(4)* %[[TEST2STR]])

if.end:                                           ; preds = %if.then, %entry
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #4
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare i32 @__read_pipe_2_AS0(%opencl.pipe_t addrspace(1)*, i8*, i32, i32) local_unnamed_addr

declare i32 @__write_pipe_2_AS0(%opencl.pipe_t addrspace(1)*, i8*, i32, i32) local_unnamed_addr

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; CHECK: define void @in_kernel
; Function Attrs: convergent nounwind
define void @in_kernel() local_unnamed_addr #2 !kernel_arg_addr_space !6 !kernel_arg_access_qual !6 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !6 !kernel_arg_host_accessible !6 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !6 !kernel_arg_buffer_location !6 {
entry:
  %write.src = alloca i32
  %0 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)* addrspace(1)* @ichIn.pipe
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ichIn, align 4, !tbaa !17
  %2 = bitcast %opencl.pipe_t addrspace(1)* %0 to %struct.__pipe_t addrspace(1)*
  %3 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %call1 = call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %2, i8 addrspace(4)* %3)
  %4 = load i32, i32* %write.src
  %cmp = icmp sgt i32 %4, 0
  br i1 %cmp, label %if.then, label %if.end
; CHECK: %[[TESTCHINSTR:[0-9]+]] = addrspacecast [11 x i8]* @test_ch_in.str to i8 addrspace(4)*
; CHECK: %{{[a-z0-9]+}} = call i32 @__read_pipe_2_bl_io_intel(%struct.__pipe_t addrspace(1)* %2, i8 addrspace(4)* %3, i8 addrspace(4)* %[[TESTCHINSTR]])

if.then:                                          ; preds = %entry
  %5 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)* addrspace(1)* @ichOut.pipe
  %6 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @ichOut, align 4, !tbaa !17
  store i32 %4, i32* %write.src
  %7 = bitcast %opencl.pipe_t addrspace(1)* %5 to %struct.__pipe_t addrspace(1)*
  %8 = addrspacecast i32* %write.src to i8 addrspace(4)*
  %9 = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %7, i8 addrspace(4)* %8)
  br label %if.end
; CHECK: %[[TESTCHOUTSTR:[0-9]+]] = addrspacecast [12 x i8]* @test_ch_out.str to i8 addrspace(4)*
; CHECK: %{{[a-z0-9]+}} = call i32 @__write_pipe_2_bl_io_intel(%struct.__pipe_t addrspace(1)* %8, i8 addrspace(4)* %9, i8 addrspace(4)* %[[TESTCHOUTSTR]])

if.end:                                           ; preds = %if.then, %entry
  ret void
}

define void @__pipe_global_ctor() {
entry:
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @ichIn.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 2)
  store %opencl.pipe_t addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @ichIn.pipe.bs to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* @ichIn.pipe
  call void @__pipe_init_intel(%struct.__pipe_t addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @ichOut.pipe.bs to %struct.__pipe_t addrspace(1)*), i32 4, i32 0, i32 2)
  store %opencl.pipe_t addrspace(1)* bitcast ([1408 x i8] addrspace(1)* @ichOut.pipe.bs to %opencl.pipe_t addrspace(1)*), %opencl.pipe_t addrspace(1)* addrspace(1)* @ichOut.pipe
  ret void
}

; Function Attrs: nounwind readnone
declare void @__pipe_init_intel(%struct.__pipe_t addrspace(1)*, i32, i32, i32) #3

; Function Attrs: nounwind readnone
declare i32 @__write_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture readonly) #3

declare i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*)

; Function Attrs: nounwind readnone
declare i32 @__read_pipe_2_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)* nocapture) #3

declare i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)*, i8 addrspace(4)*)

; CHECK: define void @__pipe_global_dtor() {
; CHECK: call void @__pipe_release_intel(%struct.__pipe_t addrspace(1)* bitcast (%opencl.pipe_t addrspace(1)* addrspace(1)* @ichIn.pipe to %struct.__pipe_t addrspace(1)*))
; CHECK: call void @__pipe_release_intel(%struct.__pipe_t addrspace(1)* bitcast (%opencl.pipe_t addrspace(1)* addrspace(1)* @ichOut.pipe to %struct.__pipe_t addrspace(1)*))
; CHECK: declare void @__pipe_release_intel(%struct.__pipe_t addrspace(1)* nocapture readonly) #3
; CHECK: declare i32 @__read_pipe_2_bl_io_intel(%struct.__pipe_t addrspace(1)* nocapture, i8 addrspace(4)* nocapture, i8 addrspace(4)* nocapture readonly) #3
; CHECK: declare i32 @__write_pipe_2_bl_io_intel(%struct.__pipe_t addrspace(1)* nocapture, i8 addrspace(4)* nocapture, i8 addrspace(4)* nocapture readonly) #3
; CHECK: declare i32 @__read_pipe_2_io_intel(%struct.__pipe_t addrspace(1)* nocapture, i8 addrspace(4)* nocapture, i8 addrspace(4)* nocapture readonly) #3
; CHECK: declare i32 @__write_pipe_2_io_intel(%struct.__pipe_t addrspace(1)* nocapture, i8 addrspace(4)* nocapture, i8 addrspace(4)* nocapture readonly) #3

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind }

!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}
!opencl.kernels = !{!8}

!0 = !{i32 4}
!1 = !{!"test_ch_in"}
!2 = !{!"test_ch_out"}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 1, i32 0}
!5 = !{i32 1, i32 2}
!6 = !{}
!7 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c6d6a9ff56af3a739a4e26fd2d27d3c0d595d1ac) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm bc27103eecb30c3ea8a73ad546051e4a9a309e68)"}
!8 = !{void (%opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)*, i32, float addrspace(1)*)* @k1, void ()* @in_kernel}
!9 = !{i32 1, i32 1, i32 0, i32 1}
!10 = !{!"read_only", !"write_only", !"none", !"none"}
!11 = !{!"int", !"int", !"int", !"float*"}
!12 = !{!"pipe", !"pipe", !"", !""}
!13 = !{i1 false, i1 false, i1 false, i1 false}
!14 = !{i32 0, i32 0, i32 0, i32 0}
!15 = !{!"test1", !"test2", !"", !""}
!16 = !{!"", !"", !"", !""}
!17 = !{!18, !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
