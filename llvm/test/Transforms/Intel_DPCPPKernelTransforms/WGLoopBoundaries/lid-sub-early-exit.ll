; RUN: opt -passes="dpcpp-kernel-analysis,dpcpp-kernel-wg-loop-bound" %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes="dpcpp-kernel-analysis,dpcpp-kernel-wg-loop-bound" %s -S -debug -disable-output 2>&1 | FileCheck %s
; RUN: opt -dpcpp-kernel-analysis -dpcpp-kernel-wg-loop-bound %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-analysis -dpcpp-kernel-wg-loop-bound %s -S -debug -disable-output 2>&1 | FileCheck %s

; The source code for the test is:
; __kernel void test(__global float* a, long uniform1, long uniform2) {
;     size_t lid = get_local_id(0);
;     if (lid - uniform1 <= uniform2) a[lid] = 3.f;
;     return;
; }

; CHECK: WGLoopBoundaries
; CHECK: found 2 early exit boundaries
; CHECK: Dim=0, Contains=T, IsGID=F, IsSigned=F, IsUpperBound=T
; CHECK-SAME: %final_right_bound = select i1 %right_lt_left, i64 -1, i64 %right_boundary_align
; CHECK-NEXT: Dim=0, Contains=T, IsGID=F, IsSigned=F, IsUpperBound=F
; CHECK-SAME: %non_negative_left_bound = select i1 %left_lt_zero, i64 0, i64 %uniform1
; CHECK: found 0 uniform early exit conditions

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

define void @test(float addrspace(1)* noalias nocapture %a, i64 %uniform1, i64 %uniform2) #0 {
entry:
  %call = tail call i64 @_Z12get_local_idj(i32 0) #2
  %sub = sub i64 %call, %uniform1
  %cmp = icmp ugt i64 %sub, %uniform2
  br i1 %cmp, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %a, i64 %call
  store float 3.000000e+00, float addrspace(1)* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  ret void
}

; Function Attrs: nounwind readnone
declare i64 @_Z12get_local_idj(i32) #1

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

!0 = !{void (float addrspace(1)*, i64, i64)* @test}
!7 = !{i32 1, i32 2}
!8 = !{}

; DEBUGIFY-COUNT-5: Instruction with empty DebugLoc in function test
; DEBUGIFY-COUNT-35: Instruction with empty DebugLoc in function WG.boundaries.test
; DEBUGIFY-COUNT-1: Missing line
; DEBUGIFY-NOT: WARNING
