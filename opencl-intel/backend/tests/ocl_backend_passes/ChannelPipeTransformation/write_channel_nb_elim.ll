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
; channel int bar_arr[5];
;
; __global bool g_valid;
;
; __kernel void foo() {
;   int i = 42;
;   float f = 0.42f;
;   struct Foo st = {0};
;
;   bool valid;
;   __private bool p_valid;
;   __local bool l_valid;
;
;   valid = write_channel_nb_altera(bar, i);
;   g_valid = write_channel_nb_altera(far, f);
;   p_valid = write_channel_nb_altera(star, st);
;   l_valid = write_channel_nb_altera(bar_arr[3], i);
; }
; ----------------------------------------------------
; RUN: opt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

%opencl.channel_t = type opaque
%struct.Foo = type { i32 }

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@g_valid = common addrspace(1) global i8 0, align 1
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4

; CHECK:      @[[PIPE_BAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_FAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_STAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_BAR_ARR:.*]] = common addrspace(1) global [5 x %opencl.pipe_t{{.*}} addrspace(1)*] zeroinitializer, align 4

; All calls to read/write_channel_nb_altera should be replaced by
; corresponding calls to pipe built-ins
;
; CHECK-NOT: call{{.*}}write_channel_nb_altera
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE]]
; CHECK: %[[CALL_BAR_PIPE:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_BAR_PIPE]]
; CHECK: %[[BOOL_CALL_BAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE]], 0
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE]]
; CHECK: %[[CALL_FAR_PIPE:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_FAR_PIPE]]
; CHECK: %[[BOOL_CALL_FAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_PIPE]], 0
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE]]
; CHECK: %[[CALL_STAR_PIPE:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_STAR_PIPE]]
; CHECK: %[[BOOL_CALL_STAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_STAR_PIPE]], 0
;
; CHECK: %[[LOAD_BAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[CAST_BAR_ARR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_ARR_PIPE]]
; CHECK: %[[CALL_BAR_ARR_PIPE:.*]] = call i32 @__write_pipe_2{{.*}} %[[CAST_BAR_ARR_PIPE]]
; CHECK: %[[BOOL_CALL_BAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_ARR_PIPE]], 0

; Function Attrs: nounwind
define spir_kernel void @foo() #0 {
entry:
  %st = alloca %struct.Foo, align 4
  %0 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #3
  %1 = getelementptr inbounds %struct.Foo, %struct.Foo* %st, i32 0, i32 0
  store i32 0, i32* %1, align 4
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !17
  %call = tail call zeroext i1 @_Z23write_channel_nb_altera11ocl_channelii(%opencl.channel_t addrspace(1)* %2, i32 42) #3
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 4, !tbaa !17
  %call1 = tail call zeroext i1 @_Z23write_channel_nb_altera11ocl_channelff(%opencl.channel_t addrspace(1)* %3, float 0x3FDAE147A0000000) #3
  %frombool2 = zext i1 %call1 to i8
  store i8 %frombool2, i8 addrspace(1)* @g_valid, align 1, !tbaa !20
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 4, !tbaa !17
  %call3 = call zeroext i1 @_Z23write_channel_nb_altera11ocl_channel3FooS_(%opencl.channel_t addrspace(1)* %4, %struct.Foo* byval nonnull align 4 %st) #3
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i32 0, i32 3), align 4, !tbaa !17
  %call5 = call zeroext i1 @_Z23write_channel_nb_altera11ocl_channelii(%opencl.channel_t addrspace(1)* %5, i32 42) #3
  call void @llvm.lifetime.end(i64 4, i8* %0) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare zeroext i1 @_Z23write_channel_nb_altera11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

declare zeroext i1 @_Z23write_channel_nb_altera11ocl_channelff(%opencl.channel_t addrspace(1)*, float) #2

declare zeroext i1 @_Z23write_channel_nb_altera11ocl_channel3FooS_(%opencl.channel_t addrspace(1)*, %struct.Foo* byval align 4) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.kernels = !{!0}
!opencl.channels = !{!6, !10, !12, !13}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!14}
!opencl.spir.version = !{!14}
!opencl.used.extensions = !{!15}
!opencl.used.optional.core.features = !{!15}
!opencl.compiler.options = !{!15}
!llvm.ident = !{!16}

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
!13 = !{[5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, !7, !8}
!14 = !{i32 2, i32 0}
!15 = !{}
!16 = !{!"clang version 3.8.1 "}
!17 = !{!18, !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{!21, !21, i64 0}
!21 = !{!"bool", !18, i64 0}
