; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sampleC.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: ret
define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %n.addr = alloca i64, align 8                   ; <i64*> [#uses=3]
  %A.addr = alloca i64*, align 8                  ; <i64**> [#uses=3]
  %B.addr = alloca i64*, align 8                  ; <i64**> [#uses=3]
  %i = alloca i64, align 8                        ; <i64*> [#uses=5]
  %j = alloca i64, align 8                        ; <i64*> [#uses=5]
  %j21 = alloca i64, align 8                      ; <i64*> [#uses=6]
  store i64 %n, i64* %n.addr
  store i64* %A, i64** %A.addr
  store i64* %B, i64** %B.addr
  %call = call i32 @get_global_id(i32 2)          ; <i32> [#uses=1]
  %tobool = icmp ne i32 %call, 0                  ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i64 0, i64* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc16, %if.then
  %tmp = load i64* %i                             ; <i64> [#uses=1]
  %tmp1 = load i64* %n.addr                       ; <i64> [#uses=1]
  %div = sdiv i64 %tmp1, 30                       ; <i64> [#uses=1]
  %cmp = icmp slt i64 %tmp, %div                  ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end19

for.body:                                         ; preds = %for.cond
  store i64 0, i64* %j
  br label %for.cond3

for.cond3:                                        ; preds = %for.inc, %for.body
  %tmp4 = load i64* %j                            ; <i64> [#uses=1]
  %call5 = call i32 @get_local_id(i32 0)          ; <i32> [#uses=1]
  %conv = zext i32 %call5 to i64                  ; <i64> [#uses=1]
  %cmp6 = icmp slt i64 %tmp4, %conv               ; <i1> [#uses=1]
  br i1 %cmp6, label %for.body8, label %for.end

for.body8:                                        ; preds = %for.cond3
  %tmp9 = load i64* %i                            ; <i64> [#uses=1]
  %tmp10 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx = getelementptr inbounds i64* %tmp10, i64 %tmp9 ; <i64*> [#uses=1]
  %tmp11 = load i64* %arrayidx                    ; <i64> [#uses=1]
  %tmp12 = load i64* %j                           ; <i64> [#uses=1]
  %tmp13 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx14 = getelementptr inbounds i64* %tmp13, i64 %tmp12 ; <i64*> [#uses=1]
  store i64 %tmp11, i64* %arrayidx14
  br label %for.inc

for.inc:                                          ; preds = %for.body8
  %tmp15 = load i64* %j                           ; <i64> [#uses=1]
  %inc = add nsw i64 %tmp15, 1                    ; <i64> [#uses=1]
  store i64 %inc, i64* %j
  br label %for.cond3

for.end:                                          ; preds = %for.cond3
  br label %for.inc16

for.inc16:                                        ; preds = %for.end
  %tmp17 = load i64* %i                           ; <i64> [#uses=1]
  %inc18 = add nsw i64 %tmp17, 1                  ; <i64> [#uses=1]
  store i64 %inc18, i64* %i
  br label %for.cond

for.end19:                                        ; preds = %for.cond
  br label %if.end

if.else:                                          ; preds = %entry
  store i64 0, i64* %j21
  br label %for.cond22

for.cond22:                                       ; preds = %for.inc36, %if.else
  %tmp23 = load i64* %j21                         ; <i64> [#uses=1]
  %tmp24 = load i64* %n.addr                      ; <i64> [#uses=1]
  %div25 = sdiv i64 %tmp24, 30                    ; <i64> [#uses=1]
  %cmp26 = icmp slt i64 %tmp23, %div25            ; <i1> [#uses=1]
  br i1 %cmp26, label %for.body28, label %for.end39

for.body28:                                       ; preds = %for.cond22
  %tmp29 = load i64* %j21                         ; <i64> [#uses=1]
  %tmp30 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx31 = getelementptr inbounds i64* %tmp30, i64 %tmp29 ; <i64*> [#uses=1]
  %tmp32 = load i64* %arrayidx31                  ; <i64> [#uses=1]
  %tmp33 = load i64* %j21                         ; <i64> [#uses=1]
  %tmp34 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx35 = getelementptr inbounds i64* %tmp34, i64 %tmp33 ; <i64*> [#uses=1]
  store i64 %tmp32, i64* %arrayidx35
  br label %for.inc36

for.inc36:                                        ; preds = %for.body28
  %tmp37 = load i64* %j21                         ; <i64> [#uses=1]
  %inc38 = add nsw i64 %tmp37, 1                  ; <i64> [#uses=1]
  store i64 %inc38, i64* %j21
  br label %for.cond22

for.end39:                                        ; preds = %for.cond22
  br label %if.end

if.end:                                           ; preds = %for.end39, %for.end19
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_local_id(i32)
