; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
;
; channel int in;
; channel int out;
;
; __attribute__((autorun))
; __attribute__((max_global_work_dim(0)))
; void __kernel test_autorun_1() {
;   int a = read_channel_intel(in);
;   if (a < 0) {
;     while (true) {
;       write_channel_intel(out, -a);
;     }
;   }
; }
; ----------------------------------------------------
; Compilation command:
;   clang -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl
;   opt -sycl-kernel-equalizer -S
; ----------------------------------------------------
; RUN: opt -passes=sycl-kernel-infinite-loop-creator %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-infinite-loop-creator %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

@in = common addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0
@out = common addrspace(1) global ptr addrspace(1) null, align 4, !packet_size !0, !packet_align !0

; CHECK: define void @test_autorun_1
; CHECK: br label %infinite_loop_entry

; CHECK-NOT: ret
; CHECK-NOT: br label %infinite_loop_entry

; CHECK: br label %infinite_loop_entry
; CHECK-NEXT: }

; Function Attrs: convergent nounwind
define void @test_autorun_1() #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !4 !kernel_arg_type !4 !kernel_arg_base_type !4 !kernel_arg_type_qual !4 !kernel_arg_host_accessible !4 !kernel_arg_pipe_depth !4 !kernel_arg_pipe_io !4 !kernel_arg_buffer_location !4 !max_global_work_dim !7 !autorun !8 {
entry:
  %a = alloca i32, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %a) #3
  %0 = load ptr addrspace(1), ptr addrspace(1) @in, align 4, !tbaa !9
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %0) #4
  store i32 %call, ptr %a, align 4, !tbaa !12
  %1 = load i32, ptr %a, align 4, !tbaa !12
  %cmp = icmp slt i32 %1, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %while.cond

while.cond:                                       ; preds = %while.body, %if.then
  br label %while.body

while.body:                                       ; preds = %while.cond
  %2 = load ptr addrspace(1), ptr addrspace(1) @out, align 4, !tbaa !9
  %3 = load i32, ptr %a, align 4, !tbaa !12
  %sub = sub nsw i32 0, %3
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %2, i32 %sub) #4
  br label %while.cond

if.end:                                           ; preds = %entry
  call void @llvm.lifetime.end.p0(i64 4, ptr %a) #3
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: convergent
declare i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1)) #2

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1), i32) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent }

!llvm.module.flags = !{!1}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!2}
!opencl.spir.version = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!5}
!sycl.kernels = !{!6}

!0 = !{i32 4}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, i32 0}
!3 = !{i32 1, i32 2}
!4 = !{}
!5 = !{!"clang version 7.0.0 "}
!6 = !{ptr @test_autorun_1}
!7 = !{i32 0}
!8 = !{i1 true}
!9 = !{!10, !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"int", !10, i64 0}

; DEBUGIFY-NOT: WARNING
