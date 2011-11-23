; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -predicate -specialize -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'BypassCase5.ll'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; This module was already processed by -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify passes

; CHECK: @BypassCase5
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                         ; preds = %header{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: header{{[0-9]*}}:
; CHECK:   br i1 %jumpover{{[0-9]*}}, label %footer{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                         ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                         ; preds = %header{{[0-9]*}}
; CHECK: footer{{[0-9]*}}:                                         ; preds = %header{{[0-9]*}}
; CHECK: ret
define void @BypassCase5(i32 %arg1, i32 %arg2, float addrspace(1)* nocapture %a, float addrspace(1)* nocapture %b) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds float addrspace(1)* %a, i32 %call
  %tmp2 = load float addrspace(1)* %arrayidx, align 4
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
  %arrayidx43 = getelementptr inbounds float addrspace(1)* %b, i32 %call
  store float %temp.0, float addrspace(1)* %arrayidx43, align 4
  ret void
}

declare i32 @get_global_id(i32) readnone
