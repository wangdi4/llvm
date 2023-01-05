; RUN: opt -passes=dpcpp-kernel-phi-canonicalization %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY-ALL
; RUN: opt -passes=dpcpp-kernel-phi-canonicalization %s -S -o - | FileCheck %s -check-prefix=SKIP

; RUN: opt -passes=dpcpp-kernel-phi-canonicalization -dpcpp-skip-non-barrier-function=false %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefixes=DEBUGIFY-NOSKIP,DEBUGIFY-ALL
; RUN: opt -passes=dpcpp-kernel-phi-canonicalization -dpcpp-skip-non-barrier-function=false %s -S -o - | FileCheck %s -check-prefix=NOSKIP

; ModuleID = 'PhiCanonCase1.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; This module was already processed by -O3 -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify passes

; SKIP-NOT: phi-split-bb

; NOSKIP: @PhiCanonCase1
; NOSKIP-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; NOSKIP: phi-split-bb:                                     ; preds = %if.then7, %if.then14
; NOSKIP: phi-split-bb1:                                    ; preds = %if.then, %phi-split-bb
; NOSKIP: ret

define void @PhiCanonCase1(i32 %arg1, i32 %arg2, float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %0 = sext i32 %call to i64
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %a, i64 %0
  %tmp2 = load float, float addrspace(1)* %arrayidx, align 4
  %cmp = icmp sgt i32 %arg1, 3
  br i1 %cmp, label %if.then, label %if.end18

if.then:                                          ; preds = %entry
  %mul = fmul float %tmp2, 2.000000e+000
  %cmp6 = icmp sgt i32 %arg2, 5
  br i1 %cmp6, label %if.then7, label %if.end18

if.then7:                                         ; preds = %if.then
  %add = fadd float %mul, 1.000000e+001
  %cmp13 = fcmp ogt float %tmp2, 1.000000e+002
  br i1 %cmp13, label %if.then14, label %if.end18

if.then14:                                        ; preds = %if.then7
  %conv = sitofp i32 %arg1 to float
  %div = fdiv float %add, %conv
  br label %if.end18

if.end18:                                         ; preds = %if.then14, %if.then7, %if.then, %entry
  %temp.0 = phi float [ %div, %if.then14 ], [ %add, %if.then7 ], [ %mul, %if.then ], [ %tmp2, %entry ]
  %arrayidx22 = getelementptr inbounds float, float addrspace(1)* %b, i64 %0
  store float %temp.0, float addrspace(1)* %arrayidx22, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32) readnone

; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function PhiCanonCase1 --  br label %phi-split-bb1
; DEBUGIFY-NOSKIP: WARNING: Instruction with empty DebugLoc in function PhiCanonCase1 --  br label %if.end18
; DEBUGIFY-ALL-NOT: WARNING
