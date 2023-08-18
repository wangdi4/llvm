; RUN: opt -passes="sycl-kernel-analysis,sycl-kernel-wg-loop-bound" %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes="sycl-kernel-analysis,sycl-kernel-wg-loop-bound" %s -S -debug -disable-output 2>&1 | FileCheck %s

; The source code for the test is:
;__kernel void test(__global float* input,__global float *res,int n,long l)
;{
;       int i = get_global_id(0)-l;
;       if (i>n-2)
;           return;
;       res[i]=input[i];
;

; CHECK: WGLoopBoundaries
; CHECK: found 0 early exit boundaries
; CHECK: found 0 uniform early exit conditions

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind
define void @test(ptr addrspace(1) nocapture %input, ptr addrspace(1) nocapture %res, i32 %n, i64 %l) #0 !kernel_arg_addr_space !1 !kernel_arg_access_qual !2 !kernel_arg_type !3 !kernel_arg_base_type !4 !kernel_arg_type_qual !5 !kernel_arg_name !6 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2
  %sub = sub i64 %call, %l
  %conv = trunc i64 %sub to i32
  %sub1 = add nsw i32 %n, -2
  %cmp = icmp sgt i32 %conv, %sub1
  br i1 %cmp, label %return, label %if.end

if.end:                                           ; preds = %entry
  %sext = shl i64 %sub, 32
  %idxprom = ashr exact i64 %sext, 32
  %arrayidx = getelementptr inbounds float, ptr addrspace(1) %input, i64 %idxprom
  %0 = load float, ptr addrspace(1) %arrayidx, align 4
  %arrayidx4 = getelementptr inbounds float, ptr addrspace(1) %res, i64 %idxprom
  store float %0, ptr addrspace(1) %arrayidx4, align 4
  br label %return

return:                                           ; preds = %entry, %if.end
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{ptr @test}
!1 = !{i32 1, i32 1, i32 0, i32 0}
!2 = !{!"none", !"none", !"none", !"none"}
!3 = !{!"ptr", !"ptr", !"int", !"long"}
!4 = !{!"", !"", !"", !""}
!5 = !{!"ptr", !"ptr", !"int", !"long"}
!6 = !{!"input", !"res", !"n", !"l"}
!7 = !{i32 1, i32 2}
!8 = !{}

; DEBUGIFY-COUNT-1: Instruction with empty DebugLoc in function test
; DEBUGIFY-NOT: WARNING
