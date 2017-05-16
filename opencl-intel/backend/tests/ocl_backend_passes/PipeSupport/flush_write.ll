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
;   int i = 42;
;   float f = 0.42f;
;   struct Foo st = {0};
;
;   write_channel_altera(bar, i);
;   write_channel_altera(far, f);
;   write_channel_altera(star, st);
;
;   write_channel_altera(bar_arr[3], i);
;   write_channel_altera(far_arr[3][2], f);
;   write_channel_altera(star_arr[3][2][1], st);
; }
;
; __kernel void nbfoo() {
;   bool valid;
;   int i = 42;
;   float f = 0.42f;
;   struct Foo st = {0};
;
;   valid = write_channel_nb_altera(bar, i);
;   valid = write_channel_nb_altera(far, f);
;   valid = write_channel_nb_altera(star, st);
;
;   valid = write_channel_nb_altera(bar_arr[3], i);
;   valid = write_channel_nb_altera(far_arr[3][2], f);
;   valid = write_channel_nb_altera(star_arr[3][2][1], st);
; }
; ----------------------------------------------------
; RUNX: opt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -pipe-support -verify %s -S | FileCheck %s
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
; CHECK:      @[[PIPE_BAR_ARR:.*]] = common addrspace(1) global [5 x %opencl.pipe_t{{.*}} addrspace(1)*] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR_ARR:.*]] = common addrspace(1) global [5 x [4 x %opencl.pipe_t{{.*}} addrspace(1)*]] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_STAR_ARR:.*]] = common addrspace(1) global [5 x [4 x [3 x %opencl.pipe_t{{.*}} addrspace(1)*]]] zeroinitializer, align 4

; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_BAR_PIPE]]
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_FAR_PIPE]]
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_STAR_PIPE]]
;
; CHECK: %[[LOAD_BAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[CAST_BAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE_ARR]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_BAR_PIPE_ARR]]
;
; CHECK: %[[LOAD_FAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_FAR_ARR]]
; CHECK: %[[CAST_FAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE_ARR]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_FAR_PIPE_ARR]]
;
; CHECK: %[[LOAD_STAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE_ARR]]
; CHECK: call i32 @__write_pipe_2_bl{{.*}} %[[CAST_STAR_PIPE_ARR]]

;; Now check that flushes are inserted at the end:

; CHECK: %[[FLUSH_LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[FLUSH_CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[FLUSH_LOAD_BAR_PIPE]]
; CHECK: call void @__flush_write_pipe{{.*}} %[[FLUSH_CAST_BAR_PIPE]]

; CHECK: %[[FLUSH_LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[FLUSH_CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[FLUSH_LOAD_FAR_PIPE]]
; CHECK: call void @__flush_write_pipe{{.*}} %[[FLUSH_CAST_FAR_PIPE]]

; CHECK: %[[FLUSH_LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[FLUSH_CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[FLUSH_LOAD_STAR_PIPE]]
; CHECK: call void @__flush_write_pipe{{.*}} %[[FLUSH_CAST_STAR_PIPE]]

; CHECK: call void @__flush_write_pipe_array{{.*}} bitcast{{.*}} @[[PIPE_BAR_ARR]] to %struct.__pipe_t{{.*}}, i32 5

; CHECK: call void @__flush_write_pipe_array{{.*}} bitcast{{.*}} @[[PIPE_FAR_ARR]] to %struct.__pipe_t{{.*}}, i32 20

; CHECK: call void @__flush_write_pipe_array{{.*}} bitcast{{.*}} @[[PIPE_STAR_ARR]] to %struct.__pipe_t{{.*}}, i32 60

; CHECK-NEXT: ret void

; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_BAR_PIPE]]
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_FAR_PIPE]]
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_STAR_PIPE]]
;
; CHECK: %[[LOAD_BAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[CAST_BAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE_ARR]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_BAR_PIPE_ARR]]
;
; CHECK: %[[LOAD_FAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_FAR_ARR]]
; CHECK: %[[CAST_FAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE_ARR]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_FAR_PIPE_ARR]]
;
; CHECK: %[[LOAD_STAR_PIPE_ARR:.*]] = load {{.*}} @[[PIPE_STAR_ARR]]
; CHECK: %[[CAST_STAR_PIPE_ARR:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE_ARR]]
; CHECK: call i32 @__write_pipe_2{{.*}} %[[CAST_STAR_PIPE_ARR]]

;; Now check that flushes are inserted at the end:

; CHECK: %[[FLUSH_LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[FLUSH_CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[FLUSH_LOAD_BAR_PIPE]]
; CHECK: call void @__flush_write_pipe{{.*}} %[[FLUSH_CAST_BAR_PIPE]]

; CHECK: %[[FLUSH_LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[FLUSH_CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[FLUSH_LOAD_FAR_PIPE]]
; CHECK: call void @__flush_write_pipe{{.*}} %[[FLUSH_CAST_FAR_PIPE]]

; CHECK: %[[FLUSH_LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[FLUSH_CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[FLUSH_LOAD_STAR_PIPE]]
; CHECK: call void @__flush_write_pipe{{.*}} %[[FLUSH_CAST_STAR_PIPE]]

; CHECK: call void @__flush_write_pipe_array{{.*}} bitcast{{.*}} @[[PIPE_BAR_ARR]] to %struct.__pipe_t{{.*}}, i32 5

; CHECK: call void @__flush_write_pipe_array{{.*}} bitcast{{.*}} @[[PIPE_FAR_ARR]] to %struct.__pipe_t{{.*}}, i32 20

; CHECK: call void @__flush_write_pipe_array{{.*}} bitcast{{.*}} @[[PIPE_STAR_ARR]] to %struct.__pipe_t{{.*}}, i32 60

; CHECK-NEXT: ret void

; Function Attrs: nounwind
define spir_kernel void @foo() #0 {
entry:
  %st = alloca %struct.Foo, align 4
  %0 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #3
  %1 = getelementptr inbounds %struct.Foo, %struct.Foo* %st, i32 0, i32 0
  store i32 0, i32* %1, align 4
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !20
  tail call void @_Z20write_channel_altera11ocl_channelii(%opencl.channel_t addrspace(1)* %2, i32 42) #3
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 4, !tbaa !20
  tail call void @_Z20write_channel_altera11ocl_channelff(%opencl.channel_t addrspace(1)* %3, float 0x3FDAE147A0000000) #3
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 4, !tbaa !20
  call void @_Z20write_channel_altera11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %4, %struct.Foo* byval nonnull align 4 %st) #3
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i32 0, i32 3), align 4, !tbaa !20
  call void @_Z20write_channel_altera11ocl_channelii(%opencl.channel_t addrspace(1)* %5, i32 42) #3
  %6 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i32 0, i32 3, i32 2), align 4, !tbaa !20
  call void @_Z20write_channel_altera11ocl_channelff(%opencl.channel_t addrspace(1)* %6, float 0x3FDAE147A0000000) #3
  %7 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i32 0, i32 3, i32 2, i32 1), align 4, !tbaa !20
  call void @_Z20write_channel_altera11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %7, %struct.Foo* byval nonnull align 4 %st) #3
  call void @llvm.lifetime.end(i64 4, i8* %0) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare void @_Z20write_channel_altera11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

declare void @_Z20write_channel_altera11ocl_channelff(%opencl.channel_t addrspace(1)*, float) #2

declare void @_Z20write_channel_altera11ocl_channel3FooS_(%opencl.channel_t addrspace(1)*, %struct.Foo* byval align 4) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: nounwind
define spir_kernel void @nbfoo() #0 {
entry:
  %st = alloca %struct.Foo, align 4
  %0 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #3
  %1 = getelementptr inbounds %struct.Foo, %struct.Foo* %st, i32 0, i32 0
  store i32 0, i32* %1, align 4
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !20
  %call = tail call zeroext i1 @_Z23write_channel_nb_altera11ocl_channelii(%opencl.channel_t addrspace(1)* %2, i32 42) #3
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 4, !tbaa !20
  %call1 = tail call zeroext i1 @_Z23write_channel_nb_altera11ocl_channelff(%opencl.channel_t addrspace(1)* %3, float 0x3FDAE147A0000000) #3
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 4, !tbaa !20
  %call3 = call zeroext i1 @_Z23write_channel_nb_altera11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %4, %struct.Foo* byval nonnull align 4 %st) #3
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i32 0, i32 3), align 4, !tbaa !20
  %call5 = call zeroext i1 @_Z23write_channel_nb_altera11ocl_channelii(%opencl.channel_t addrspace(1)* %5, i32 42) #3
  %6 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i32 0, i32 3, i32 2), align 4, !tbaa !20
  %call7 = call zeroext i1 @_Z23write_channel_nb_altera11ocl_channelff(%opencl.channel_t addrspace(1)* %6, float 0x3FDAE147A0000000) #3
  %7 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]], [5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, i32 0, i32 3, i32 2, i32 1), align 4, !tbaa !20
  %call9 = call zeroext i1 @_Z23write_channel_nb_altera11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %7, %struct.Foo* byval nonnull align 4 %st) #3
  call void @llvm.lifetime.end(i64 4, i8* %0) #3
  ret void
}

declare zeroext i1 @_Z23write_channel_nb_altera11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

declare zeroext i1 @_Z23write_channel_nb_altera11ocl_channelff(%opencl.channel_t addrspace(1)*, float) #2

declare zeroext i1 @_Z23write_channel_nb_altera11ocl_channel3FooS_(%opencl.channel_t addrspace(1)*, %struct.Foo* byval align 4) #2

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.kernels = !{!0, !6}
!opencl.channels = !{!7, !11, !13, !14, !15, !16}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!17}
!opencl.spir.version = !{!17}
!opencl.used.extensions = !{!18}
!opencl.used.optional.core.features = !{!18}
!opencl.compiler.options = !{!18}
!llvm.ident = !{!19}

!0 = !{void ()* @foo, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space"}
!2 = !{!"kernel_arg_access_qual"}
!3 = !{!"kernel_arg_type"}
!4 = !{!"kernel_arg_base_type"}
!5 = !{!"kernel_arg_type_qual"}
!6 = !{void ()* @nbfoo, !1, !2, !3, !4, !5}
!7 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @bar, !8, !9, !10}
!8 = !{!"packet_size", i32 4}
!9 = !{!"packet_align", i32 4}
!10 = !{!"depth", i32 0}
!11 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @far, !8, !9, !12}
!12 = !{!"depth", i32 3}
!13 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @star, !8, !9}
!14 = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, !8, !9, !10}
!15 = !{[5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, !8, !9, !12}
!16 = !{[5 x [4 x [3 x %opencl.channel_t addrspace(1)*]]] addrspace(1)* @star_arr, !8, !9}
!17 = !{i32 2, i32 0}
!18 = !{}
!19 = !{!"clang version 3.8.1 "}
!20 = !{!21, !21, i64 0}
!21 = !{!"omnipotent char", !22, i64 0}
!22 = !{!"Simple C/C++ TBAA"}
