; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_altera_channels : enable
; channel int bar;
; channel float far;
;
; struct Foo {
;   int i;
; };
; channel struct Foo star;
;
; void __write_channel_altera(channel int ch, const int val, int packet_size, int align) __attribute__((overloadable));
; void __write_channel_altera(channel float ch, const float val, int packet_size, int align) __attribute__((overloadable));
; void __write_channel_altera(channel struct Foo ch, const struct Foo val, int packet_size, int align) __attribute__((overloadable));
;
; __kernel void foo() {
;   int i = 42;
;   float f = 0.42f;
;   struct Foo st = {0};
;
;   __write_channel_altera(bar, 42, sizeof(int), 4);
;   __write_channel_altera(far, f, sizeof(float), 4);
;   __write_channel_altera(star, st, sizeof(struct Foo), 4);
; }
; ----------------------------------------------------
; RUN: opt -runtimelib=../../Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-p:32:32-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir"

%opencl.channel_t = type opaque
%struct.Foo = type { i32 }

@bar = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@far = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4
@star = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4


; CHECK:      @[[PIPE_BAR:.*]] = common global %opencl.pipe_t[[PIPE_INDEX]] addrspace(1)*
; CHECK-NEXT: @[[PIPE_FAR:.*]] = common global %opencl.pipe_t[[PIPE_INDEX]] addrspace(1)*
; CHECK-NEXT: @[[PIPE_STAR:.*]] = common global %opencl.pipe_t[[PIPE_INDEX]] addrspace(1)*
;
; CHECK-DAG: @[[PIPE_BAR]].bs = common addrspace(1) global [48 x i8] zeroinitializer, align 4
; CHECK-DAG: @[[PIPE_FAR]].bs = common addrspace(1) global [64 x i8] zeroinitializer, align 4
; CHECK-DAG: @[[PIPE_STAR]].bs = common addrspace(1) global [48 x i8] zeroinitializer, align 4
;


; CHECK: @llvm.global_ctors = {{.*}} @__global_pipes_ctor

; CHECK: declare void @__pipe_init

; CHECK: define void @__global_pipes_ctor()
;
; CHECK-DAG: call void @__pipe_init{{.*}}@[[PIPE_BAR]].bs
; CHECK-DAG: store {{.*}}@[[PIPE_BAR]].bs to {{.*}}@[[PIPE_BAR]]
;
; CHECK-DAG: call void @__pipe_init{{.*}}@[[PIPE_FAR]].bs
; CHECK-DAG: store {{.*}}@[[PIPE_FAR]].bs to {{.*}}@[[PIPE_FAR]]
;
; CHECK-DAG: call void @__pipe_init{{.*}}@[[PIPE_STAR]].bs
; CHECK-DAG: store {{.*}}@[[PIPE_STAR]].bs to {{.*}}@[[PIPE_STAR]]


; Function Attrs: nounwind
define spir_kernel void @foo() #0 {
entry:
  %st = alloca %struct.Foo, align 4
  %0 = bitcast %struct.Foo* %st to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #3
  %1 = getelementptr inbounds %struct.Foo, %struct.Foo* %st, i32 0, i32 0
  store i32 0, i32* %1, align 4
  %2 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @bar, align 4, !tbaa !12
  tail call spir_func void @_Z22__write_channel_altera11ocl_channeliiii(%opencl.channel_t addrspace(1)* %2, i32 42, i32 4, i32 4) #3
  %3 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @far, align 4, !tbaa !12
  tail call spir_func void @_Z22__write_channel_altera11ocl_channelffii(%opencl.channel_t addrspace(1)* %3, float 0x3FDAE147A0000000, i32 4, i32 4) #3
  %4 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @star, align 4, !tbaa !12
  call spir_func void @_Z22__write_channel_altera11ocl_channel3FooS_ii(%opencl.channel_t addrspace(1)* %4, %struct.Foo* byval nonnull align 4 %st, i32 4, i32 4) #3
  call void @llvm.lifetime.end(i64 4, i8* %0) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

declare spir_func void @_Z22__write_channel_altera11ocl_channeliiii(%opencl.channel_t addrspace(1)*, i32, i32, i32) #2

declare spir_func void @_Z22__write_channel_altera11ocl_channelffii(%opencl.channel_t addrspace(1)*, float, i32, i32) #2

declare spir_func void @_Z22__write_channel_altera11ocl_channel3FooS_ii(%opencl.channel_t addrspace(1)*, %struct.Foo* byval align 4, i32, i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!opencl.kernels = !{!0}
!opencl.channels = !{!6, !7, !8}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!9}
!opencl.spir.version = !{!9}
!opencl.used.extensions = !{!10}
!opencl.used.optional.core.features = !{!10}
!opencl.compiler.options = !{!10}
!llvm.ident = !{!11}

!0 = !{void ()* @foo, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space"}
!2 = !{!"kernel_arg_access_qual"}
!3 = !{!"kernel_arg_type"}
!4 = !{!"kernel_arg_base_type"}
!5 = !{!"kernel_arg_type_qual"}
!6 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @bar, i32 4, i32 4}
!7 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @far, i32 4, i32 4}
!8 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @star, i32 4, i32 4}
!9 = !{i32 2, i32 0}
!10 = !{}
!11 = !{!"clang version 3.8.1 "}
!12 = !{!13, !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
