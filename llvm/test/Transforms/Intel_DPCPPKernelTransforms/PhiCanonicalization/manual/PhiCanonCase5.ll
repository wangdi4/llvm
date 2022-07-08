; RUN: opt -passes=dpcpp-kernel-phi-canonicalization %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY-ALL
; RUN: opt -passes=dpcpp-kernel-phi-canonicalization %s -S -o - | FileCheck %s -check-prefix=SKIP
; RUN: opt -dpcpp-kernel-phi-canonicalization %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY-ALL
; RUN: opt -dpcpp-kernel-phi-canonicalization %s -S -o - | FileCheck %s -check-prefix=SKIP

; RUN: opt -passes=dpcpp-kernel-phi-canonicalization -dpcpp-skip-non-barrier-function=false %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefixes=DEBUGIFY-NOSKIP,DEBUGIFY-ALL
; RUN: opt -passes=dpcpp-kernel-phi-canonicalization -dpcpp-skip-non-barrier-function=false %s -S -o - | FileCheck %s -check-prefix=NOSKIP
; RUN: opt -dpcpp-kernel-phi-canonicalization -dpcpp-skip-non-barrier-function=false %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefixes=DEBUGIFY-NOSKIP,DEBUGIFY-ALL
; RUN: opt -dpcpp-kernel-phi-canonicalization -dpcpp-skip-non-barrier-function=false %s -S -o - | FileCheck %s -check-prefix=NOSKIP

; ModuleID = 'PhiCanonCase5.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; This module was already processed by -O3 -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify passes
; SKIP-NOT: phi-split-bb

; NOSKIP: @PhiCanonCase5
; NOSKIP-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; NOSKIP: phi-split-bb:                                     ; preds = %if.then23, %if.then32
; NOSKIP: phi-split-bb1:                                    ; preds = %if.else, %phi-split-bb
; NOSKIP: phi-split-bb3:                                    ; preds = %if.then7, %if.then14
; NOSKIP: phi-split-bb5:                                    ; preds = %if.then, %phi-split-bb3
; NOSKIP: ret

define void @PhiCanonCase5(i32 %arg1, i32 %arg2, float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %a, i32 %call
  %tmp2 = load float, float addrspace(1)* %arrayidx, align 4
  %cmp = icmp sgt i32 %arg1, 3
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %mul = fmul float %tmp2, 2.000000e+000
  %cmp6 = icmp sgt i32 %arg2, 5
  br i1 %cmp6, label %if.then7, label %if.end39

if.then7:                                         ; preds = %if.then
  %add = fadd float %mul, 1.000000e+001
  %cmp13 = fcmp ogt float %tmp2, 1.000000e+002
  br i1 %cmp13, label %if.then14, label %if.end39

if.then14:                                        ; preds = %if.then7
  %conv = sitofp i32 %arg1 to float
  %div = fdiv float %add, %conv
  br label %if.end39

if.else:                                          ; preds = %entry
  %mul19 = fmul float %tmp2, 1.200000e+001
  %cmp21 = icmp slt i32 %arg2, 0
  br i1 %cmp21, label %if.then23, label %if.end39

if.then23:                                        ; preds = %if.else
  %add25 = fadd float %mul19, 1.800000e+001
  %cmp30 = fcmp olt float %tmp2, 5.000000e+002
  br i1 %cmp30, label %if.then32, label %if.end39

if.then32:                                        ; preds = %if.then23
  %conv34 = sitofp i32 %arg1 to float
  %div36 = fdiv float %add25, %conv34
  br label %if.end39

if.end39:                                         ; preds = %if.then32, %if.then23, %if.else, %if.then14, %if.then7, %if.then
  %temp.0 = phi float [ %div, %if.then14 ], [ %add, %if.then7 ], [ %mul, %if.then ], [ %div36, %if.then32 ], [ %add25, %if.then23 ], [ %mul19, %if.else ]
  %arrayidx43 = getelementptr inbounds float, float addrspace(1)* %b, i32 %call
  store float %temp.0, float addrspace(1)* %arrayidx43, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32) readnone

!opencl.kernels = !{!0}
!opencl.compiler.options = !{}

!0 = !{void (i32, i32, float addrspace(1)*, float addrspace(1)*)* @PhiCanonCase5, !1, !1, !"", !"int, int, float __attribute__((address_space(1))) *, float __attribute__((address_space(1))) *", !"opencl_PhiCanonCase5_locals_anchor", !2, !3, !4, !5, !""}
!1 = !{i32 0, i32 0, i32 0}
!2 = !{i32 0, i32 0, i32 1, i32 1}
!3 = !{i32 3, i32 3, i32 3, i32 3}
!4 = !{!"int", !"int", !"float*", !"float*"}
!5 = !{!"arg1", !"arg2", !"a", !"b"}

; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function PhiCanonCase5 --  br label %phi-split-bb1
; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function PhiCanonCase5 --  br label %if.end39
; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function PhiCanonCase5 --  br label %phi-split-bb5
; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function PhiCanonCase5 --  br label %if.end39
; DEBUGIFY-ALL-NOT: WARNING
