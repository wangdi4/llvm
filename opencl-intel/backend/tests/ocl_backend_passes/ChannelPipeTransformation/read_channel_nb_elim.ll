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
; channel float far_arr[5][4];
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
;   bool * p_to_valid = &valid;
;   __global bool *p_to_g_valid = &g_valid;
;   __local bool *p_to_l_valid = &l_valid;
;   __private bool *p_to_p_valid = &p_valid;
;
;   i = read_channel_nb_altera(bar, &valid);
;   f = read_channel_nb_altera(far, p_to_valid);
;   st = read_channel_nb_altera(star, p_to_g_valid);
;   i = read_channel_nb_altera(bar_arr[3], p_to_l_valid);
;   f = read_channel_nb_altera(far_arr[3][2], p_to_p_valid);
; }
; ----------------------------------------------------
; RUN: opt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

%opencl.channel_t = type opaque
%struct.Foo = type { i32 }

@foo.l_valid = internal addrspace(3) global i8 undef, align 1
@g_valid = common addrspace(1) global i8 0, align 1
@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@bar_arr = common addrspace(1) global [5 x %opencl.channel_t addrspace(1)*] zeroinitializer, align 4
@far_arr = common addrspace(1) global [5 x [4 x %opencl.channel_t addrspace(1)*]] zeroinitializer, align 4

; CHECK:      @[[PIPE_BAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_FAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_STAR:.*]] = common addrspace(1) global %opencl.pipe_t{{.*}} addrspace(1)*
; CHECK-NEXT: @[[PIPE_BAR_ARR:.*]] = common addrspace(1) global [5 x %opencl.pipe_t{{.*}} addrspace(1)*] zeroinitializer, align 4
; CHECK-NEXT: @[[PIPE_FAR_ARR:.*]] = common addrspace(1) global [5 x [4 x %opencl.pipe_t{{.*}} addrspace(1)*]] zeroinitializer, align 4

; All calls to read/write_channel_nb_altera should be replaced by
; corresponding calls to pipe built-ins
;
; CHECK-NOT: call{{.*}}read_channel_nb_altera
;
; CHECK: %[[VALID:.*]] = addrspacecast {{.*}}* %valid
;
; CHECK: %[[LOAD_BAR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR]]
; CHECK: %[[CAST_BAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_PIPE]]
; CHECK: %[[CALL_BAR_PIPE:.*]] = call i32 @__read_pipe_2{{.*}} %[[CAST_BAR_PIPE]]
; CHECK: %[[BOOL_CALL_BAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_BAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_BAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_BAR_PIPE]], {{.*}}* %[[VALID]]
;
; CHECK: %[[LOAD_FAR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR]]
; CHECK: %[[CAST_FAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_PIPE]]
; CHECK: %[[CALL_FAR_PIPE:.*]] = call i32 @__read_pipe_2{{.*}} %[[CAST_FAR_PIPE]]
; CHECK: %[[BOOL_CALL_FAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_FAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_FAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_FAR_PIPE]], {{.*}}* %[[VALID]]
;
; CHECK: %[[LOAD_STAR_PIPE:.*]] = load {{.*}} @[[PIPE_STAR]]
; CHECK: %[[CAST_STAR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_STAR_PIPE]]
; CHECK: %[[CALL_STAR_PIPE:.*]] = call i32 @__read_pipe_2{{.*}} %[[CAST_STAR_PIPE]]
; CHECK: %[[BOOL_CALL_STAR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_STAR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_STAR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_STAR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_STAR_PIPE]], {{.*}}* @g_valid
;
; CHECK: %[[LOAD_BAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_BAR_ARR]]
; CHECK: %[[CAST_BAR_ARR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_BAR_ARR_PIPE]]
; CHECK: %[[CALL_BAR_ARR_PIPE:.*]] = call i32 @__read_pipe_2{{.*}} %[[CAST_BAR_ARR_PIPE]]
; CHECK: %[[BOOL_CALL_BAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_BAR_ARR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_BAR_ARR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_BAR_ARR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_BAR_ARR_PIPE]], {{.*}}* @foo.l_valid
;
; CHECK: %[[PVALID:.*]] = addrspacecast {{.*}} %p_valid
; CHECK: %[[LOAD_FAR_ARR_PIPE:.*]] = load {{.*}} @[[PIPE_FAR_ARR]]
; CHECK: %[[CAST_FAR_ARR_PIPE:.*]] = bitcast %opencl.pipe_t{{.*}} %[[LOAD_FAR_ARR_PIPE]]
; CHECK: %[[CALL_FAR_ARR_PIPE:.*]] = call i32 @__read_pipe_2{{.*}} %[[CAST_FAR_ARR_PIPE]]
; CHECK: %[[BOOL_CALL_FAR_ARR_PIPE:.*]] = icmp eq {{.*}} %[[CALL_FAR_ARR_PIPE]], 0
; CHECK: %[[ZEXT_BOOL_CALL_FAR_ARR_PIPE:.*]] = zext {{.*}} %[[BOOL_CALL_FAR_ARR_PIPE]]
; CHECK: store {{.*}} %[[ZEXT_BOOL_CALL_FAR_ARR_PIPE]], {{.*}}* %[[PVALID]]

; Function Attrs: nounwind
define spir_kernel void @foo() #0 {
entry:
  %valid = alloca i8, align 1
  %p_valid = alloca i8, align 1
  %tmp = alloca %struct.Foo, align 4
  call void @llvm.lifetime.start(i64 1, i8* nonnull %valid) #3
  call void @llvm.lifetime.start(i64 1, i8* nonnull %p_valid) #3
  %0 = addrspacecast i8* %valid to i8 addrspace(4)*
  %1 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !18
  %call = call i32 @_Z22read_channel_nb_altera11ocl_channeliPU3AS4b(%opencl.channel_t addrspace(1)* %1, i8 addrspace(4)* %0) #3
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 4, !tbaa !18
  %call1 = call float @_Z22read_channel_nb_altera11ocl_channelfPU3AS4b(%opencl.channel_t addrspace(1)* %2, i8 addrspace(4)* %0) #3
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 4, !tbaa !18
  call void @_Z22read_channel_nb_altera11ocl_channel3FooPU3AS4b(%struct.Foo* nonnull sret %tmp, %opencl.channel_t addrspace(1)* %3, i8 addrspace(4)* addrspacecast (i8 addrspace(1)* @g_valid to i8 addrspace(4)*)) #3
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x %opencl.channel_t addrspace(1)*], [5 x %opencl.channel_t addrspace(1)*] addrspace(1)* @bar_arr, i32 0, i32 3), align 4, !tbaa !18
  %call2 = call i32 @_Z22read_channel_nb_altera11ocl_channeliPU3AS4b(%opencl.channel_t addrspace(1)* %4, i8 addrspace(4)* addrspacecast (i8 addrspace(3)* @foo.l_valid to i8 addrspace(4)*)) #3
  %5 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* getelementptr inbounds ([5 x [4 x %opencl.channel_t addrspace(1)*]], [5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, i32 0, i32 3, i32 2), align 4, !tbaa !18
  %6 = addrspacecast i8* %p_valid to i8 addrspace(4)*
  %call3 = call float @_Z22read_channel_nb_altera11ocl_channelfPU3AS4b(%opencl.channel_t addrspace(1)* %5, i8 addrspace(4)* %6) #3
  call void @llvm.lifetime.end(i64 1, i8* nonnull %p_valid) #3
  call void @llvm.lifetime.end(i64 1, i8* nonnull %valid) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare i32 @_Z22read_channel_nb_altera11ocl_channeliPU3AS4b(%opencl.channel_t addrspace(1)*, i8 addrspace(4)*) #2

declare float @_Z22read_channel_nb_altera11ocl_channelfPU3AS4b(%opencl.channel_t addrspace(1)*, i8 addrspace(4)*) #2

declare void @_Z22read_channel_nb_altera11ocl_channel3FooPU3AS4b(%struct.Foo* sret, %opencl.channel_t addrspace(1)*, i8 addrspace(4)*) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.kernels = !{!0}
!opencl.channels = !{!6, !10, !12, !13, !14}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!15}
!opencl.spir.version = !{!15}
!opencl.used.extensions = !{!16}
!opencl.used.optional.core.features = !{!16}
!opencl.compiler.options = !{!16}
!llvm.ident = !{!17}

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
!14 = !{[5 x [4 x %opencl.channel_t addrspace(1)*]] addrspace(1)* @far_arr, !7, !8}
!15 = !{i32 2, i32 0}
!16 = !{}
!17 = !{!"clang version 3.8.1 "}
!18 = !{!19, !19, i64 0}
!19 = !{!"omnipotent char", !20, i64 0}
!20 = !{!"Simple C/C++ TBAA"}
