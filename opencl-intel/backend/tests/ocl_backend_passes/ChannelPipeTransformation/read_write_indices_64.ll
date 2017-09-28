; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels : enable
;
; struct st {
;   int i;
; };
;
; channel int bar_arr[5] __attribute__((depth(0)));
; channel float far_arr[5][4] __attribute__((depth(3)));
; channel struct st star_arr[5][4][3];
; channel long lar_arr[6][5][4][3];
;
; __kernel void foo() {
;   char char_index = 4;
;   short short_index = 3;
;   int int_index = 2;
;   long long_index = 1;
;
;   int i = 42;
;   float f = 3.14;
;   struct st s = { 100 };
;   long l = 500;
;
;   write_channel_altera(bar_arr[char_index], i);
;   write_channel_altera(far_arr[char_index][short_index], f);
;   write_channel_altera(star_arr[char_index][short_index][int_index], s);
;   write_channel_altera(lar_arr[char_index][short_index][int_index][long_index], l);
;
;   i = read_channel_altera(bar_arr[char_index]);
;   f = read_channel_altera(far_arr[char_index][short_index]);
;   s = read_channel_altera(star_arr[char_index][short_index][int_index]);
;   l = read_channel_altera(lar_arr[char_index][short_index][int_index][long_index]);
; }
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"

%opencl.channel_t = type opaque
%struct.st = type { i32 }

@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4
@far_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4
@star_arr = common addrspace(1) global [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] zeroinitializer, align 4
@lar_arr = common addrspace(1) global [6 x [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]]] zeroinitializer, align 8

; CHECK: %[[LOAD_BAR_PIPE_ARR_W:.*]] = load {{.*}} @pipe.bar_arr, i64 0, i64 4
; CHECK: %[[CAST_BAR_PIPE_ARR_W:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE_ARR_W]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_BAR_PIPE_ARR_W]]
;
; CHECK: %[[LOAD_FAR_PIPE_ARR_W:.*]] = load {{.*}} @pipe.far_arr, i64 0, i64 4, i64 3
; CHECK: %[[CAST_FAR_PIPE_ARR_W:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE_ARR_W]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_FAR_PIPE_ARR_W]]
;
; CHECK: %[[LOAD_STAR_PIPE_ARR_W:.*]] = load {{.*}} @pipe.star_arr, i64 0, i64 4, i64 3, i64 2
; CHECK: %[[CAST_STAR_PIPE_ARR_W:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE_ARR_W]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_STAR_PIPE_ARR_W]]
;
; CHECK-DAG: %[[LOAD_LAR_PIPE_ARR_W:.*]] = load {{.*}} @pipe.lar_arr, i64 0, i64 4, i64 3, i64 2, i64 1
; CHECK: %[[CAST_LAR_PIPE_ARR_W:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_LAR_PIPE_ARR_W]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_LAR_PIPE_ARR_W]]
;
; CHECK: %[[LOAD_BAR_PIPE_ARR_R:.*]] = load {{.*}} @pipe.bar_arr, i64 0, i64 4
; CHECK: %[[CAST_BAR_PIPE_ARR_R:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE_ARR_R]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[CAST_BAR_PIPE_ARR_R]]
;
; CHECK: %[[LOAD_FAR_PIPE_ARR_R:.*]] = load {{.*}} @pipe.far_arr, i64 0, i64 4, i64 3
; CHECK: %[[CAST_FAR_PIPE_ARR_R:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE_ARR_R]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[CAST_FAR_PIPE_ARR_R]]
;
; CHECK: %[[LOAD_STAR_PIPE_ARR_R:.*]] = load {{.*}} @pipe.star_arr, i64 0, i64 4, i64 3, i64 2
; CHECK: %[[CAST_STAR_PIPE_ARR_R:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE_ARR_R]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[CAST_STAR_PIPE_ARR_R]]
;
; CHECK-DAG: %[[LOAD_LAR_PIPE_ARR_R:.*]] = load {{.*}} @pipe.lar_arr, i64 0, i64 4, i64 3, i64 2, i64 1
; CHECK: %[[CAST_LAR_PIPE_ARR_R:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_LAR_PIPE_ARR_R]]
; CHECK: call i32 @__read_pipe_2{{.*}} %[[CAST_LAR_PIPE_ARR_R]]


; Function Attrs: nounwind
define spir_kernel void @foo() #0 {
entry:
  %s = alloca %struct.st, align 4
  %tmp = alloca %struct.st, align 4
  %0 = bitcast %struct.st* %s to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #3
  %1 = getelementptr inbounds %struct.st, %struct.st* %s, i64 0, i32 0
  store i32 100, i32* %1, align 4
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i64 0, i64 4), align 4, !tbaa !19
  tail call void @_Z20write_channel_altera11ocl_channelii(%opencl.channel_t addrspace(1)* %2, i32 42) #3
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i64 0, i64 4, i64 3), align 4, !tbaa !19
  tail call void @_Z20write_channel_altera11ocl_channelff(%opencl.channel_t addrspace(1)* %3, float 0x40091EB860000000) #3
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i64 0, i64 4, i64 3, i64 2), align 4, !tbaa !19
  call void @_Z20write_channel_altera11ocl_channel2stS_(%opencl.channel_t addrspace(1)* %4, %struct.st* byval nonnull align 4 %s) #3
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([6 x [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]]], [6 x [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]]] addrspace(1)* @lar_arr, i64 0, i64 4, i64 3, i64 2, i64 1), align 8, !tbaa !19
  call void @_Z20write_channel_altera11ocl_channelll(%opencl.channel_t addrspace(1)* %5, i64 500) #3
  %6 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i64 0, i64 4), align 4, !tbaa !19
  %call = call i32 @_Z19read_channel_altera11ocl_channeli(%opencl.channel_t addrspace(1)* %6) #3
  %7 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i64 0, i64 4, i64 3), align 4, !tbaa !19
  %call24 = call float @_Z19read_channel_altera11ocl_channelf(%opencl.channel_t addrspace(1)* %7) #3
  %8 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i64 0, i64 4, i64 3, i64 2), align 4, !tbaa !19
  call void @_Z19read_channel_altera11ocl_channel2st(%struct.st* nonnull sret %tmp, %opencl.channel_t addrspace(1)* %8) #3
  %9 = getelementptr inbounds %struct.st, %struct.st* %tmp, i64 0, i32 0
  %10 = load i32, i32* %9, align 4, !tbaa !22
  store i32 %10, i32* %1, align 4, !tbaa !22
  %11 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([6 x [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]]], [6 x [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]]] addrspace(1)* @lar_arr, i64 0, i64 4, i64 3, i64 2, i64 1), align 8, !tbaa !19
  %call38 = call i64 @_Z19read_channel_altera11ocl_channell(%opencl.channel_t addrspace(1)* %11) #3
  call void @llvm.lifetime.end(i64 4, i8* %0) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare void @_Z20write_channel_altera11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

declare void @_Z20write_channel_altera11ocl_channelff(%opencl.channel_t addrspace(1)*, float) #2

declare void @_Z20write_channel_altera11ocl_channel2stS_(%opencl.channel_t addrspace(1)*, %struct.st* byval align 4) #2

declare void @_Z20write_channel_altera11ocl_channelll(%opencl.channel_t addrspace(1)*, i64) #2

declare i32 @_Z19read_channel_altera11ocl_channeli(%opencl.channel_t addrspace(1)*) #2

declare float @_Z19read_channel_altera11ocl_channelf(%opencl.channel_t addrspace(1)*) #2

declare void @_Z19read_channel_altera11ocl_channel2st(%struct.st* sret, %opencl.channel_t addrspace(1)*) #2

declare i64 @_Z19read_channel_altera11ocl_channell(%opencl.channel_t addrspace(1)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.kernels = !{!0}
!opencl.channels = !{!6, !10, !12, !13}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!16}
!opencl.spir.version = !{!16}
!opencl.used.extensions = !{!17}
!opencl.used.optional.core.features = !{!17}
!opencl.compiler.options = !{!17}
!llvm.ident = !{!18}

!0 = !{void ()* @foo, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space"}
!2 = !{!"kernel_arg_access_qual"}
!3 = !{!"kernel_arg_type"}
!4 = !{!"kernel_arg_base_type"}
!5 = !{!"kernel_arg_type_qual"}
!6 = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, !7, !8, !9}
!7 = !{!"packet_size", i32 4}
!8 = !{!"packet_align", i32 4}
!9 = !{!"depth", i32 0}
!10 = !{[5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, !7, !8, !11}
!11 = !{!"depth", i32 3}
!12 = !{[5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, !7, !8}
!13 = !{[6 x [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]]] addrspace(1)* @lar_arr, !14, !15}
!14 = !{!"packet_size", i32 8}
!15 = !{!"packet_align", i32 8}
!16 = !{i32 2, i32 0}
!17 = !{}
!18 = !{!"clang version 3.8.1 "}
!19 = !{!20, !20, i64 0}
!20 = !{!"omnipotent char", !21, i64 0}
!21 = !{!"Simple C/C++ TBAA"}
!22 = !{!23, !23, i64 0}
!23 = !{!"int", !20, i64 0}
