; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -analyze -kernel-analysis -cl-loop-bound -verify %t.bc -S -o %t2.ll
; RUN: FileCheck %s --input-file=%t2.ll

; The source code for the test is:
;__kernel void test(__global float* input,__global float *res,int n,long l)
;{
;       int i = get_global_id(0)-l;
;       if (i>n-2)
;           return;
;       res[i]=input[i];
;

; CHECK: CLWGLoopBoundaries
; CHECK: found 0 early exit boundaries
; CHECK: found 0 uniform early exit conditions

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: nounwind
define void @test(float addrspace(1)* nocapture %input, float addrspace(1)* nocapture %res, i32 %n, i64 %l) #0 {
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
  %arrayidx = getelementptr inbounds float addrspace(1)* %input, i64 %idxprom
  %0 = load float addrspace(1)* %arrayidx, align 4
  %arrayidx4 = getelementptr inbounds float addrspace(1)* %res, i64 %idxprom
  store float %0, float addrspace(1)* %arrayidx4, align 4
  br label %return

return:                                           ; preds = %entry, %if.end
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z13get_global_idj(i32) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-frame-pointer-elim-non-leaf"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!opencl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!7}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (float addrspace(1)*, float addrspace(1)*, i32, i64)* @test, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 0, i32 0}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"float*", !"float*", !"int", !"long"}
!4 = !{!"kernel_arg_type_qual", !"", !"", !"", !""}
!5 = !{!"kernel_arg_base_type", !"float*", !"float*", !"int", !"long"}
!6 = !{!"kernel_arg_name", !"input", !"res", !"n", !"l"}
!7 = !{i32 1, i32 2}
!8 = !{}
