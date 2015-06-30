; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../../Full/runtime.bc -analyze -kernel-analysis -cl-loop-bound -verify %t.bc -S -o %t2.ll
; RUN: FileCheck %s --input-file=%t2.ll

; The source code for the test is:
;__kernel void test(long l,__global long* gl){
;  int ggid = get_global_id(0);
;  if(ggid<l){
;    gl[ggid] = 2*gl[ggid];
;  }
;}

; CHECK: CLWGLoopBoundaries
; CHECK: found 1 early exit boundaries
; CHECK: dim=0, contains=F, isGID=T, isSigned=T, isUpper=T
; CHECK-NEXT: i64 %l
; CHECK: found 0 uniform early exit conditions

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-win32"
; Function Attrs: nounwind
define void @test(i64 %l, i64 addrspace(1)* noalias nocapture %gl) #0 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #2
  %sext = shl i64 %call, 32
  %conv1 = ashr exact i64 %sext, 32
  %cmp = icmp slt i64 %conv1, %l
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %arrayidx = getelementptr inbounds i64 addrspace(1)* %gl, i64 %conv1
  %0 = load i64 addrspace(1)* %arrayidx, align 8
  %mul = shl nsw i64 %0, 1
  store i64 %mul, i64 addrspace(1)* %arrayidx, align 8
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
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

!0 = !{void (i64, i64 addrspace(1)*)* @test, !1, !2, !3, !4, !5, !6}
!1 = !{!"kernel_arg_addr_space", i32 0, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"long", !"long*"}
!4 = !{!"kernel_arg_type_qual", !"", !""}
!5 = !{!"kernel_arg_base_type", !"long", !"long*"}
!6 = !{!"kernel_arg_name", !"l", !"gl"}
!7 = !{i32 1, i32 2}
!8 = !{}
