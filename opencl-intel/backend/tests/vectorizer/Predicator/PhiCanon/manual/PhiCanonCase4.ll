; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'PhiCanonCase4.c'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; This module was already processed by -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify passes

; CHECK: @PhiCanonCase4
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %if.else11, %if.then16
; CHECK: phi-split-bb1:                                    ; preds = %if.then7, %if.else
; CHECK: ret
define void @PhiCanonCase4(i32 %arg1, i32 %arg2, float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @_Z13get_global_idj(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(1)* %a, i32 %call
  %tmp2 = load float addrspace(1)* %arrayidx, align 4
  %cmp = icmp sgt i32 %arg1, 3
  br i1 %cmp, label %if.then, label %if.else11

if.then:                                          ; preds = %entry
  %mul = fmul float %tmp2, 2.000000e+000
  %cmp6 = icmp sgt i32 %arg2, 5
  br i1 %cmp6, label %if.then7, label %if.else

if.then7:                                         ; preds = %if.then
  %add = fadd float %mul, 1.000000e+001
  br label %if.end20

if.else:                                          ; preds = %if.then
  %conv = sitofp i32 %arg1 to float
  %div = fdiv float %mul, %conv
  br label %if.end20

if.else11:                                        ; preds = %entry
  %sub = fadd float %tmp2, -8.000000e+000
  %cmp14 = icmp slt i32 %arg2, 0
  br i1 %cmp14, label %if.then16, label %if.end20

if.then16:                                        ; preds = %if.else11
  %add18 = fadd float %sub, 2.300000e+001
  br label %if.end20

if.end20:                                         ; preds = %if.then16, %if.else11, %if.else, %if.then7
  %temp.0 = phi float [ %add, %if.then7 ], [ %div, %if.else ], [ %add18, %if.then16 ], [ %sub, %if.else11 ]
  %arrayidx24 = getelementptr inbounds float addrspace(1)* %b, i32 %call
  store float %temp.0, float addrspace(1)* %arrayidx24, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32) readnone
