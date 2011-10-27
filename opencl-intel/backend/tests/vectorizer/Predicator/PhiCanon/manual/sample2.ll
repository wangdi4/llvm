; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample2.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: ret
define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %n.addr = alloca i64, align 8                   ; <i64*> [#uses=2]
  %A.addr = alloca i64*, align 8                  ; <i64**> [#uses=3]
  %B.addr = alloca i64*, align 8                  ; <i64**> [#uses=1]
  %sum = alloca i64, align 8                      ; <i64*> [#uses=4]
  %i = alloca i64, align 8                        ; <i64*> [#uses=5]
  store i64 %n, i64* %n.addr
  store i64* %A, i64** %A.addr
  store i64* %B, i64** %B.addr
  store i64 0, i64* %sum
  store i64 0, i64* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %tmp = load i64* %i                             ; <i64> [#uses=1]
  %tmp1 = load i64* %n.addr                       ; <i64> [#uses=1]
  %cmp = icmp slt i64 %tmp, %tmp1                 ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp2 = load i64* %i                            ; <i64> [#uses=1]
  %tmp3 = load i64** %A.addr                      ; <i64*> [#uses=1]
  %arrayidx = getelementptr inbounds i64* %tmp3, i64 %tmp2 ; <i64*> [#uses=1]
  %tmp4 = load i64* %arrayidx                     ; <i64> [#uses=1]
  %tmp5 = load i64* %sum                          ; <i64> [#uses=1]
  %add = add nsw i64 %tmp5, %tmp4                 ; <i64> [#uses=1]
  store i64 %add, i64* %sum
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp6 = load i64* %i                            ; <i64> [#uses=1]
  %inc = add nsw i64 %tmp6, 1                     ; <i64> [#uses=1]
  store i64 %inc, i64* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %tmp7 = load i64* %sum                          ; <i64> [#uses=1]
  %tmp8 = load i64** %A.addr                      ; <i64*> [#uses=1]
  %arrayidx9 = getelementptr inbounds i64* %tmp8, i64 0 ; <i64*> [#uses=1]
  store i64 %tmp7, i64* %arrayidx9
  ret void
}
