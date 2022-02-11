; RUN: opt -passes=dpcpp-kernel-phi-canonicalization %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=dpcpp-kernel-phi-canonicalization %s -S -o - | FileCheck %s
; RUN: opt -dpcpp-kernel-phi-canonicalization %s -S -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -dpcpp-kernel-phi-canonicalization %s -S -o - | FileCheck %s

; ModuleID = 'PhiCanonCase2.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; This module was already processed by -O3 -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify passes

; CHECK: @PhiCanonCase2
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %if.then7, %if.else
; CHECK: ret
define void @PhiCanonCase2(i32 %arg1, i32 %arg2, float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float, float addrspace(1)* %a, i32 %call
  %tmp2 = load float, float addrspace(1)* %arrayidx, align 4
  %cmp = icmp sgt i32 %arg1, 3
  br i1 %cmp, label %if.then, label %if.end11

if.then:                                          ; preds = %entry
  %mul = fmul float %tmp2, 2.000000e+000
  %cmp6 = icmp sgt i32 %arg2, 5
  br i1 %cmp6, label %if.then7, label %if.else

if.then7:                                         ; preds = %if.then
  %add = fadd float %mul, 1.000000e+001
  br label %if.end11

if.else:                                          ; preds = %if.then
  %conv = sitofp i32 %arg1 to float
  %div = fdiv float %mul, %conv
  br label %if.end11

if.end11:                                         ; preds = %if.else, %if.then7, %entry
  %temp.0 = phi float [ %add, %if.then7 ], [ %div, %if.else ], [ %tmp2, %entry ]
  %arrayidx15 = getelementptr inbounds float, float addrspace(1)* %b, i32 %call
  store float %temp.0, float addrspace(1)* %arrayidx15, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32) readnone

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function PhiCanonCase2 --  br label %if.end11
; DEBUGIFY-NOT: WARNING
