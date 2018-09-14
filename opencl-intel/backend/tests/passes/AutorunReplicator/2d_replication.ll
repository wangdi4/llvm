; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
;
; channel int ch[4][2];
;
; __attribute__((autorun))
; __attribute__((max_global_work_dim(0)))
; __attribute__((num_compute_units(2,2,1)))
; __kernel void plus() {
;   size_t x = get_compute_id(0);
;   size_t y = get_compute_id(1);
;   size_t chan_index = 2 * x + y;
;   int a = read_channel_intel(ch[chan_index][0]);
;   write_channel_intel(ch[chan_index][1], a + 1);
; }
; ----------------------------------------------------
; Clang options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; Opt passes: -spir-materializer
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -autorun-replicator -verify %s -S | FileCheck %s --implicit-check-not get_compute_id
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@ch = common addrspace(1) global [4 x [2 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4

; CHECK: define {{.*}}@plus
; CHECK: store i64 0, i64* %x
; CHECK: store i64 0, i64* %y

; CHECK: define {{.*}}@plus.1
; CHECK: store i64 0, i64* %x
; CHECK: store i64 1, i64* %y

; CHECK: define {{.*}}@plus.2
; CHECK: store i64 1, i64* %x
; CHECK: store i64 0, i64* %y

; CHECK: define {{.*}}@plus.3
; CHECK: store i64 1, i64* %x
; CHECK: store i64 1, i64* %y

; CHECK-NOT: define {{.*}}@plus

; Function Attrs: nounwind
define void @plus() #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !5 !kernel_arg_type !5 !kernel_arg_base_type !5 !kernel_arg_type_qual !5 !task !8 !autorun !8 !num_compute_units !9 {
entry:
  %x = alloca i64, align 8
  %y = alloca i64, align 8
  %chan_index = alloca i64, align 8
  %a = alloca i32, align 4
  %0 = bitcast i64* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #3
  %1 = call i64 @get_compute_id(i32 0)
  store i64 %1, i64* %x, align 8, !tbaa !10
  %2 = bitcast i64* %y to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %2) #3
  %3 = call i64 @get_compute_id(i32 1)
  store i64 %3, i64* %y, align 8, !tbaa !10
  %4 = bitcast i64* %chan_index to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %4) #3
  %5 = load i64, i64* %x, align 8, !tbaa !10
  %mul = mul i64 2, %5
  %6 = load i64, i64* %y, align 8, !tbaa !10
  %add = add i64 %mul, %6
  store i64 %add, i64* %chan_index, align 8, !tbaa !10
  %7 = bitcast i32* %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #3
  %8 = load i64, i64* %chan_index, align 8, !tbaa !10
  %arrayidx = getelementptr inbounds [4 x [2 x %opencl.channel_t addrspace(1)*]], [4 x [2 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @ch, i64 0, i64 %8
  %arrayidx1 = getelementptr inbounds [2 x %opencl.channel_t addrspace(1)*], [2 x %opencl.channel_t addrspace(1)*] addrspace(1)* %arrayidx, i64 0, i64 0
  %9 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx1, align 4, !tbaa !14
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)* %9)
  store i32 %call, i32* %a, align 4, !tbaa !15
  %10 = load i64, i64* %chan_index, align 8, !tbaa !10
  %arrayidx2 = getelementptr inbounds [4 x [2 x %opencl.channel_t addrspace(1)*]], [4 x [2 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @ch, i64 0, i64 %10
  %arrayidx3 = getelementptr inbounds [2 x %opencl.channel_t addrspace(1)*], [2 x %opencl.channel_t addrspace(1)*] addrspace(1)* %arrayidx2, i64 0, i64 1
  %11 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* %arrayidx3, align 4, !tbaa !14
  %12 = load i32, i32* %a, align 4, !tbaa !15
  %add4 = add nsw i32 %12, 1
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %11, i32 %add4)
  %13 = bitcast i32* %a to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #3
  %14 = bitcast i64* %chan_index to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %14) #3
  %15 = bitcast i64* %y to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %15) #3
  %16 = bitcast i64* %x to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %16) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

declare i64 @get_compute_id(i32)

declare i32 @_Z18read_channel_intel11ocl_channeli(%opencl.channel_t addrspace(1)*) #2

declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

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

!0 = !{[4 x [2 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @ch, !1, !2}
!1 = !{!"packet_size", i32 4}
!2 = !{!"packet_align", i32 4}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 2, i32 0}
!5 = !{}
!6 = !{!"clang version 5.0.0 )"}
!7 = !{void ()* @plus}
!8 = !{i1 true}
!9 = !{i32 2, i32 2, i32 1}
!10 = !{!11, !11, i64 0}
!11 = !{!"long", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!12, !12, i64 0}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !12, i64 0}
