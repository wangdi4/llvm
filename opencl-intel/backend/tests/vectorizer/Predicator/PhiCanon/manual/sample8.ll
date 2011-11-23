; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample8.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.cond10.preheader, %for.end48.loopexit
; CHECK: ret
define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %n.addr = alloca i64, align 8                   ; <i64*> [#uses=4]
  %A.addr = alloca i64*, align 8                  ; <i64**> [#uses=4]
  %B.addr = alloca i64*, align 8                  ; <i64**> [#uses=4]
  %sum = alloca i64, align 8                      ; <i64*> [#uses=7]
  %i = alloca i64, align 8                        ; <i64*> [#uses=5]
  %i9 = alloca i64, align 8                       ; <i64*> [#uses=10]
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
  %tmp2 = load i64* %n.addr                       ; <i64> [#uses=1]
  %rem = srem i64 %tmp2, 5                        ; <i64> [#uses=1]
  %tobool = icmp ne i64 %rem, 0                   ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %tmp3 = load i64* %i                            ; <i64> [#uses=1]
  %tmp4 = load i64** %A.addr                      ; <i64*> [#uses=1]
  %arrayidx = getelementptr inbounds i64* %tmp4, i64 %tmp3 ; <i64*> [#uses=1]
  %tmp5 = load i64* %arrayidx                     ; <i64> [#uses=1]
  %tmp6 = load i64* %sum                          ; <i64> [#uses=1]
  %add = add nsw i64 %tmp6, %tmp5                 ; <i64> [#uses=1]
  store i64 %add, i64* %sum
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %tmp7 = load i64* %i                            ; <i64> [#uses=1]
  %inc = add nsw i64 %tmp7, 1                     ; <i64> [#uses=1]
  store i64 %inc, i64* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i64 0, i64* %i9
  br label %for.cond10

for.cond10:                                       ; preds = %for.inc45, %for.end
  %tmp11 = load i64* %i9                          ; <i64> [#uses=1]
  %tmp12 = load i64* %n.addr                      ; <i64> [#uses=1]
  %cmp13 = icmp slt i64 %tmp11, %tmp12            ; <i1> [#uses=1]
  br i1 %cmp13, label %for.body14, label %for.end48

for.body14:                                       ; preds = %for.cond10
  %tmp15 = load i64* %i9                          ; <i64> [#uses=1]
  %tmp16 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx17 = getelementptr inbounds i64* %tmp16, i64 %tmp15 ; <i64*> [#uses=1]
  %tmp18 = load i64* %arrayidx17                  ; <i64> [#uses=1]
  %tobool19 = icmp ne i64 %tmp18, 0               ; <i1> [#uses=1]
  br i1 %tobool19, label %land.lhs.true, label %if.else

land.lhs.true:                                    ; preds = %for.body14
  %tmp20 = load i64* %i9                          ; <i64> [#uses=1]
  %tmp21 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx22 = getelementptr inbounds i64* %tmp21, i64 %tmp20 ; <i64*> [#uses=1]
  %tmp23 = load i64* %arrayidx22                  ; <i64> [#uses=1]
  %tobool24 = icmp ne i64 %tmp23, 0               ; <i1> [#uses=1]
  br i1 %tobool24, label %land.lhs.true25, label %if.else

land.lhs.true25:                                  ; preds = %land.lhs.true
  %tmp26 = load i64* %i9                          ; <i64> [#uses=1]
  %cmp27 = icmp ne i64 %tmp26, 9                  ; <i1> [#uses=1]
  br i1 %cmp27, label %if.then28, label %if.else

if.then28:                                        ; preds = %land.lhs.true25
  %tmp29 = load i64* %i9                          ; <i64> [#uses=1]
  %tmp30 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx31 = getelementptr inbounds i64* %tmp30, i64 %tmp29 ; <i64*> [#uses=1]
  %tmp32 = load i64* %arrayidx31                  ; <i64> [#uses=1]
  %tmp33 = load i64* %sum                         ; <i64> [#uses=1]
  %add34 = add nsw i64 %tmp33, %tmp32             ; <i64> [#uses=1]
  store i64 %add34, i64* %sum
  br label %if.end44

if.else:                                          ; preds = %land.lhs.true25, %land.lhs.true, %for.body14
  %tmp35 = load i64* %sum                         ; <i64> [#uses=1]
  %add36 = add nsw i64 %tmp35, 1                  ; <i64> [#uses=1]
  store i64 %add36, i64* %sum
  %tmp37 = load i64* %i9                          ; <i64> [#uses=1]
  %tmp38 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx39 = getelementptr inbounds i64* %tmp38, i64 %tmp37 ; <i64*> [#uses=1]
  %tmp40 = load i64* %arrayidx39                  ; <i64> [#uses=1]
  %tmp41 = load i64* %i9                          ; <i64> [#uses=1]
  %tmp42 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx43 = getelementptr inbounds i64* %tmp42, i64 %tmp41 ; <i64*> [#uses=1]
  store i64 %tmp40, i64* %arrayidx43
  br label %if.end44

if.end44:                                         ; preds = %if.else, %if.then28
  br label %for.inc45

for.inc45:                                        ; preds = %if.end44
  %tmp46 = load i64* %i9                          ; <i64> [#uses=1]
  %inc47 = add nsw i64 %tmp46, 1                  ; <i64> [#uses=1]
  store i64 %inc47, i64* %i9
  br label %for.cond10

for.end48:                                        ; preds = %for.cond10
  ret void
}
