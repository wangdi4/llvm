; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
; channel int a;
;
; __attribute__((noinline))
; int sendOne(channel int ch) {
;   write_channel_intel(ch, 1);
;   return 1;
; }
;
; __kernel void foo(__global int *data) {
;   data[0] = sendOne(a);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -cl-std=CL2.0
; ----------------------------------------------------
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@a = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !packet_size !0, !packet_align !0

; CHECK: define {{.*}} @foo
; CHECK: %[[PIPE:.*]] = load {{.*}} @a.pipe
; CHECK: %[[CALL_RESULT:.*]] = call {{.*}} @sendOne({{.*}} %[[PIPE]])
; CHECK: store i32 %[[CALL_RESULT]]

; Function Attrs: convergent noinline nounwind
define spir_func i32 @sendOne(%opencl.channel_t addrspace(1)* %ch) #0 {
entry:
  %ch.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  store %opencl.channel_t addrspace(1)* %ch, %opencl.channel_t addrspace(1)** %ch.addr, align 4, !tbaa !7
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %ch.addr, align 4, !tbaa !7
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %0, i32 1) #3
  ret i32 1
}

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #1

; Function Attrs: convergent nounwind
define spir_kernel void @foo(i32 addrspace(1)* %data) #2 !kernel_arg_addr_space !10 !kernel_arg_access_qual !11 !kernel_arg_type !12 !kernel_arg_base_type !12 !kernel_arg_type_qual !13 !kernel_arg_host_accessible !14 {
entry:
  %data.addr = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %data, i32 addrspace(1)** %data.addr, align 8, !tbaa !15
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @a, align 4, !tbaa !7
  %call = call spir_func i32 @sendOne(%opencl.channel_t addrspace(1)* %0) #3
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %data.addr, align 8, !tbaa !15
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %1, i64 0
  store i32 %call, i32 addrspace(1)* %arrayidx, align 4, !tbaa !17
  ret void
}

attributes #0 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent }

!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!5}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!5}
!llvm.ident = !{!6}

!0 = !{i32 4}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 2, i32 0}
!5 = !{}
!6 = !{!"clang version 6.0.0 "}
!7 = !{!8, !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{i32 1}
!11 = !{!"none"}
!12 = !{!"int*"}
!13 = !{!""}
!14 = !{i1 false}
!15 = !{!16, !16, i64 0}
!16 = !{!"any pointer", !8, i64 0}
!17 = !{!18, !18, i64 0}
!18 = !{!"int", !8, i64 0}


; DEBUGIFY-NOT: WARNING: Missing line
