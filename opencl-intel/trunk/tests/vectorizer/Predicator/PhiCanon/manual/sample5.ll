; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample5.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: ret
define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %n.addr = alloca i64, align 8                   ; <i64*> [#uses=3]
  %A.addr = alloca i64*, align 8                  ; <i64**> [#uses=3]
  %B.addr = alloca i64*, align 8                  ; <i64**> [#uses=2]
  %sum = alloca i64, align 8                      ; <i64*> [#uses=6]
  %i = alloca i64, align 8                        ; <i64*> [#uses=5]
  %j = alloca i64, align 8                        ; <i64*> [#uses=5]
  store i64 %n, i64* %n.addr
  store i64* %A, i64** %A.addr
  store i64* %B, i64** %B.addr
  store i64 0, i64* %sum
  store i64 0, i64* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc20, %entry
  %tmp = load i64* %i                             ; <i64> [#uses=1]
  %tmp1 = load i64* %n.addr                       ; <i64> [#uses=1]
  %div = sdiv i64 %tmp1, 10                       ; <i64> [#uses=1]
  %cmp = icmp slt i64 %tmp, %div                  ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end23

for.body:                                         ; preds = %for.cond
  %tmp2 = load i64* %i                            ; <i64> [#uses=1]
  %tmp3 = load i64** %B.addr                      ; <i64*> [#uses=1]
  %arrayidx = getelementptr inbounds i64* %tmp3, i64 %tmp2 ; <i64*> [#uses=1]
  %tmp4 = load i64* %arrayidx                     ; <i64> [#uses=1]
  %tmp5 = load i64* %sum                          ; <i64> [#uses=1]
  %add = add nsw i64 %tmp5, %tmp4                 ; <i64> [#uses=1]
  store i64 %add, i64* %sum
  store i64 0, i64* %j
  br label %for.cond7

for.cond7:                                        ; preds = %for.inc, %for.body
  %tmp8 = load i64* %j                            ; <i64> [#uses=1]
  %tmp9 = load i64* %n.addr                       ; <i64> [#uses=1]
  %div10 = sdiv i64 %tmp9, 10                     ; <i64> [#uses=1]
  %cmp11 = icmp slt i64 %tmp8, %div10             ; <i1> [#uses=1]
  br i1 %cmp11, label %for.body12, label %for.end

for.body12:                                       ; preds = %for.cond7
  %tmp13 = load i64* %j                           ; <i64> [#uses=1]
  %tmp14 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx15 = getelementptr inbounds i64* %tmp14, i64 %tmp13 ; <i64*> [#uses=1]
  %tmp16 = load i64* %arrayidx15                  ; <i64> [#uses=1]
  %tmp17 = load i64* %sum                         ; <i64> [#uses=1]
  %add18 = add nsw i64 %tmp17, %tmp16             ; <i64> [#uses=1]
  store i64 %add18, i64* %sum
  br label %for.inc

for.inc:                                          ; preds = %for.body12
  %tmp19 = load i64* %j                           ; <i64> [#uses=1]
  %inc = add nsw i64 %tmp19, 1                    ; <i64> [#uses=1]
  store i64 %inc, i64* %j
  br label %for.cond7

for.end:                                          ; preds = %for.cond7
  br label %for.inc20

for.inc20:                                        ; preds = %for.end
  %tmp21 = load i64* %i                           ; <i64> [#uses=1]
  %inc22 = add nsw i64 %tmp21, 1                  ; <i64> [#uses=1]
  store i64 %inc22, i64* %i
  br label %for.cond

for.end23:                                        ; preds = %for.cond
  %tmp24 = load i64* %sum                         ; <i64> [#uses=1]
  %tmp25 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx26 = getelementptr inbounds i64* %tmp25, i64 5 ; <i64*> [#uses=1]
  store i64 %tmp24, i64* %arrayidx26
  ret void
}
