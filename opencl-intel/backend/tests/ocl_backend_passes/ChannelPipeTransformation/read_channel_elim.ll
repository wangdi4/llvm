; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels : enable
; channel int bar __attribute__((depth(0)));
; channel float far __attribute__((depth(3)));
;
; struct Foo {
;   int i;
; };
; channel struct Foo star;
;
; channel int bar_arr[5] __attribute__((depth(0)));
; channel float far_arr[5][4] __attribute__((depth(3)));
; channel struct Foo star_arr[5][4][3];
;
; __kernel void foo() {
;   int i = read_channel_altera(bar);
;   float f = read_channel_altera(far);
;   struct Foo st = read_channel_altera(star);
;
;   int ii = read_channel_altera(bar_arr[3]);
;   float ff = read_channel_altera(far_arr[3][2]);
;   struct Foo stst = read_channel_altera(star_arr[3][2][1]);
; }
; ----------------------------------------------------
;
; RUN: opt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

%opencl.channel_t = type opaque
%struct.Foo = type { i32 }

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4
@far_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4
@star_arr = common addrspace(1) global [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] zeroinitializer, align 4

; CHECK:      @[[PIPE_BAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_FAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_STAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
;
; CHECK:      @[[PIPE_BAR_ARR:.*]] = common addrspace(1) global [5 x %opencl.pipe_t{{.*}} addrspace(1)*] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR_ARR:.*]] = common addrspace(1) global [5 x [4 x %opencl.pipe_t{{.*}} addrspace(1)*]] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_STAR_ARR:.*]] = common addrspace(1) global [5 x [4 x [3 x %opencl.pipe_t{{.*}} addrspace(1)*]]] zeroinitializer, align 4

; All calls to read/write_channel_altera should be replaced by
; corresponding calls to pipe built-ins
;
; CHECK-NOT: call{{.*}}read_channel_altera
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE]]
; CHECK: call i32 @__read_pipe_2_bl{{.*}} %[[CAST_BAR_PIPE]]
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE]]
; CHECK: call i32 @__read_pipe_2_bl{{.*}} %[[CAST_FAR_PIPE]]
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE]]
; CHECK: call i32 @__read_pipe_2_bl{{.*}} %[[CAST_STAR_PIPE]]
;
; CHECK: %[[LOAD_BAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[CAST_BAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE_ARR]]
; CHECK: call i32 @__read_pipe_2_bl{{.*}} %[[CAST_BAR_PIPE_ARR]]
;
; CHECK: %[[LOAD_FAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_FAR_ARR]]
; CHECK: %[[CAST_FAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE_ARR]]
; CHECK: call i32 @__read_pipe_2_bl{{.*}} %[[CAST_FAR_PIPE_ARR]]
;
; CHECK: %[[LOAD_STAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE_ARR]]
; CHECK: call i32 @__read_pipe_2_bl{{.*}} %[[CAST_STAR_PIPE_ARR]]

; Function Attrs: nounwind
define spir_kernel void @foo() #0 {
entry:
  %st = alloca %struct.Foo, align 4
  %stst = alloca %struct.Foo, align 4
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !19
  %call = tail call i32 @_Z19read_channel_altera11ocl_channeli(%opencl.channel_t addrspace(1)* %0) #3
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 4, !tbaa !19
  %call1 = tail call float @_Z19read_channel_altera11ocl_channelf(%opencl.channel_t addrspace(1)* %1) #3
  %2 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start(i64 4, i8* %2) #3
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 4, !tbaa !19
  call void @_Z19read_channel_altera11ocl_channel3Foo(%struct.Foo* nonnull sret %st, %opencl.channel_t addrspace(1)* %3) #3
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i32 0, i32 3), align 4, !tbaa !19
  %call2 = call i32 @_Z19read_channel_altera11ocl_channeli(%opencl.channel_t addrspace(1)* %4) #3
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i32 0, i32 3, i32 2), align 4, !tbaa !19
  %call3 = call float @_Z19read_channel_altera11ocl_channelf(%opencl.channel_t addrspace(1)* %5) #3
  %6 = bitcast %struct.Foo* %stst to i8*
  call void @llvm.lifetime.start(i64 4, i8* %6) #3
  %7 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i32 0, i32 3, i32 2, i32 1), align 4, !tbaa !19
  call void @_Z19read_channel_altera11ocl_channel3Foo(%struct.Foo* nonnull sret %stst, %opencl.channel_t addrspace(1)* %7) #3
  call void @llvm.lifetime.end(i64 4, i8* %6) #3
  call void @llvm.lifetime.end(i64 4, i8* %2) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare i32 @_Z19read_channel_altera11ocl_channeli(%opencl.channel_t addrspace(1)*) #2

declare float @_Z19read_channel_altera11ocl_channelf(%opencl.channel_t addrspace(1)*) #2

declare void @_Z19read_channel_altera11ocl_channel3Foo(%struct.Foo* sret, %opencl.channel_t addrspace(1)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.kernels = !{!0}
!opencl.channels = !{!6, !10, !12, !13, !14, !15}
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
!6 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @bar, !7, !8, !9}
!7 = !{!"packet_size", i32 4}
!8 = !{!"packet_align", i32 4}
!9 = !{!"depth", i32 0}
!10 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @far, !7, !8, !11}
!11 = !{!"depth", i32 3}
!12 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @star, !7, !8}
!13 = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, !7, !8, !9}
!14 = !{[5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, !7, !8, !11}
!15 = !{[5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, !7, !8}
!16 = !{i32 2, i32 0}
!17 = !{}
!18 = !{!"clang version 3.8.1 "}
!19 = !{!20, !20, i64 0}
!20 = !{!"omnipotent char", !21, i64 0}
!21 = !{!"Simple C/C++ TBAA"}
