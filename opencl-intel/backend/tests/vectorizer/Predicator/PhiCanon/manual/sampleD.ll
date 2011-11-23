; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sampleD.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.end26.loopexit, %for.end26.loopexit13
; CHECK: ret
define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %n.addr = alloca i64, align 8                   ; <i64*> [#uses=2]
  %A.addr = alloca i64*, align 8                  ; <i64**> [#uses=3]
  %B.addr = alloca i64*, align 8                  ; <i64**> [#uses=2]
  %i = alloca i64, align 8                        ; <i64*> [#uses=7]
  %j = alloca i64, align 8                        ; <i64*> [#uses=4]
  store i64 %n, i64* %n.addr
  store i64* %A, i64** %A.addr
  store i64* %B, i64** %B.addr
  store i64 0, i64* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc23, %entry
  %tmp = load i64* %i                             ; <i64> [#uses=1]
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  %conv = zext i32 %call to i64                   ; <i64> [#uses=1]
  %cmp = icmp slt i64 %tmp, %conv                 ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end26

for.body:                                         ; preds = %for.cond
  store i64 0, i64* %j
  br label %for.cond3

for.cond3:                                        ; preds = %for.inc, %for.body
  %tmp4 = load i64* %j                            ; <i64> [#uses=1]
  %tmp5 = load i64* %n.addr                       ; <i64> [#uses=1]
  %div = sdiv i64 %tmp5, 10                       ; <i64> [#uses=1]
  %cmp6 = icmp slt i64 %tmp4, %div                ; <i1> [#uses=1]
  br i1 %cmp6, label %for.body8, label %for.end

for.body8:                                        ; preds = %for.cond3
  %tmp9 = load i64* %i                            ; <i64> [#uses=1]
  %add = add nsw i64 %tmp9, 10                    ; <i64> [#uses=1]
  %tmp10 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx = getelementptr inbounds i64* %tmp10, i64 %add ; <i64*> [#uses=1]
  %tmp11 = load i64* %arrayidx                    ; <i64> [#uses=1]
  %tmp12 = load i64* %i                           ; <i64> [#uses=1]
  %xor = xor i64 %tmp12, 90                       ; <i64> [#uses=1]
  %tmp13 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx14 = getelementptr inbounds i64* %tmp13, i64 %xor ; <i64*> [#uses=1]
  store i64 %tmp11, i64* %arrayidx14
  br label %for.inc

for.inc:                                          ; preds = %for.body8
  %tmp15 = load i64* %j                           ; <i64> [#uses=1]
  %inc = add nsw i64 %tmp15, 1                    ; <i64> [#uses=1]
  store i64 %inc, i64* %j
  br label %for.cond3

for.end:                                          ; preds = %for.cond3
  %call16 = call i32 @get_local_id(i32 0)         ; <i32> [#uses=1]
  %conv17 = zext i32 %call16 to i64               ; <i64> [#uses=1]
  %tmp18 = load i64* %i                           ; <i64> [#uses=1]
  %tmp19 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx20 = getelementptr inbounds i64* %tmp19, i64 %tmp18 ; <i64*> [#uses=2]
  %tmp21 = load i64* %arrayidx20                  ; <i64> [#uses=1]
  %xor22 = xor i64 %tmp21, %conv17                ; <i64> [#uses=1]
  store i64 %xor22, i64* %arrayidx20
  br label %for.inc23

for.inc23:                                        ; preds = %for.end
  %tmp24 = load i64* %i                           ; <i64> [#uses=1]
  %inc25 = add nsw i64 %tmp24, 1                  ; <i64> [#uses=1]
  store i64 %inc25, i64* %i
  br label %for.cond

for.end26:                                        ; preds = %for.cond
  %call27 = call i32 @get_local_id(i32 2)         ; <i32> [#uses=0]
  ret void
}

declare i32 @get_local_id(i32)
