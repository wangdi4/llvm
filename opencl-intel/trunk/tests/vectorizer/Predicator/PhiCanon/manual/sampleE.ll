; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sampleE.bc'
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
  %i = alloca i64, align 8                        ; <i64*> [#uses=8]
  %t = alloca i64, align 8                        ; <i64*> [#uses=2]
  %j = alloca i64, align 8                        ; <i64*> [#uses=4]
  %j18 = alloca i64, align 8                      ; <i64*> [#uses=4]
  store i64 %n, i64* %n.addr
  store i64* %A, i64** %A.addr
  store i64* %B, i64** %B.addr
  store i64 0, i64* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc36, %entry
  %tmp = load i64* %i                             ; <i64> [#uses=1]
  %tmp1 = load i64* %n.addr                       ; <i64> [#uses=1]
  %cmp = icmp slt i64 %tmp, %tmp1                 ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end39

for.body:                                         ; preds = %for.cond
  %tmp2 = load i64* %i                            ; <i64> [#uses=1]
  %rem = srem i64 %tmp2, 2                        ; <i64> [#uses=1]
  %tobool = icmp ne i64 %rem, 0                   ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  store i64 0, i64* %t
  store i64 0, i64* %j
  br label %for.cond5

for.cond5:                                        ; preds = %for.inc, %if.then
  %tmp6 = load i64* %j                            ; <i64> [#uses=1]
  %tmp7 = load i64* %t                            ; <i64> [#uses=1]
  %add = add nsw i64 %tmp7, 2                     ; <i64> [#uses=1]
  %cmp8 = icmp slt i64 %tmp6, %add                ; <i1> [#uses=1]
  br i1 %cmp8, label %for.body9, label %for.end

for.body9:                                        ; preds = %for.cond5
  %tmp10 = load i64* %n.addr                      ; <i64> [#uses=1]
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  %conv = zext i32 %call to i64                   ; <i64> [#uses=1]
  %add11 = add nsw i64 %tmp10, %conv              ; <i64> [#uses=1]
  %tmp12 = load i64* %i                           ; <i64> [#uses=1]
  %tmp13 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx = getelementptr inbounds i64* %tmp13, i64 %tmp12 ; <i64*> [#uses=2]
  %tmp14 = load i64* %arrayidx                    ; <i64> [#uses=1]
  %add15 = add nsw i64 %tmp14, %add11             ; <i64> [#uses=1]
  store i64 %add15, i64* %arrayidx
  br label %for.inc

for.inc:                                          ; preds = %for.body9
  %tmp16 = load i64* %j                           ; <i64> [#uses=1]
  %inc = add nsw i64 %tmp16, 1                    ; <i64> [#uses=1]
  store i64 %inc, i64* %j
  br label %for.cond5

for.end:                                          ; preds = %for.cond5
  br label %if.end

if.else:                                          ; preds = %for.body
  store i64 0, i64* %j18
  br label %for.cond19

for.cond19:                                       ; preds = %for.inc32, %if.else
  %tmp20 = load i64* %j18                         ; <i64> [#uses=1]
  %cmp21 = icmp slt i64 %tmp20, 600               ; <i1> [#uses=1]
  br i1 %cmp21, label %for.body23, label %for.end35

for.body23:                                       ; preds = %for.cond19
  %tmp24 = load i64* %i                           ; <i64> [#uses=1]
  %tmp25 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx26 = getelementptr inbounds i64* %tmp25, i64 %tmp24 ; <i64*> [#uses=1]
  %tmp27 = load i64* %arrayidx26                  ; <i64> [#uses=1]
  %add28 = add nsw i64 %tmp27, 1                  ; <i64> [#uses=1]
  %tmp29 = load i64* %i                           ; <i64> [#uses=1]
  %tmp30 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx31 = getelementptr inbounds i64* %tmp30, i64 %tmp29 ; <i64*> [#uses=1]
  store i64 %add28, i64* %arrayidx31
  br label %for.inc32

for.inc32:                                        ; preds = %for.body23
  %tmp33 = load i64* %j18                         ; <i64> [#uses=1]
  %add34 = add nsw i64 %tmp33, 2                  ; <i64> [#uses=1]
  store i64 %add34, i64* %j18
  br label %for.cond19

for.end35:                                        ; preds = %for.cond19
  br label %if.end

if.end:                                           ; preds = %for.end35, %for.end
  br label %for.inc36

for.inc36:                                        ; preds = %if.end
  %tmp37 = load i64* %i                           ; <i64> [#uses=1]
  %inc38 = add nsw i64 %tmp37, 1                  ; <i64> [#uses=1]
  store i64 %inc38, i64* %i
  br label %for.cond

for.end39:                                        ; preds = %for.cond
  ret void
}

declare i32 @get_local_id(i32)
