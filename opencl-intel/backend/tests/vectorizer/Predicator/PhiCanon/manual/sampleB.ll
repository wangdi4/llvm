; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sampleB.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.cond.preheader, %if.end42.loopexit
; CHECK: ret
define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %n.addr = alloca i64, align 8                   ; <i64*> [#uses=3]
  %A.addr = alloca i64*, align 8                  ; <i64**> [#uses=3]
  %B.addr = alloca i64*, align 8                  ; <i64**> [#uses=4]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %i16 = alloca i32, align 4                      ; <i32*> [#uses=7]
  store i64 %n, i64* %n.addr
  store i64* %A, i64** %A.addr
  store i64* %B, i64** %B.addr
  %tmp = load i64** %B.addr                       ; <i64*> [#uses=1]
  %arrayidx = getelementptr inbounds i64* %tmp, i64 0 ; <i64*> [#uses=1]
  %tmp1 = load i64* %arrayidx                     ; <i64> [#uses=1]
  %tobool = icmp ne i64 %tmp1, 0                  ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %crazy

crazy:                                            ; preds = %if.then27, %if.then
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %crazy
  %tmp3 = load i32* %i                            ; <i32> [#uses=1]
  %conv = sext i32 %tmp3 to i64                   ; <i64> [#uses=1]
  %tmp4 = load i64* %n.addr                       ; <i64> [#uses=1]
  %cmp = icmp slt i64 %conv, %tmp4                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp6 = load i32* %i                            ; <i32> [#uses=1]
  %tmp7 = load i64** %B.addr                      ; <i64*> [#uses=1]
  %idxprom = sext i32 %tmp6 to i64                ; <i64> [#uses=1]
  %arrayidx8 = getelementptr inbounds i64* %tmp7, i64 %idxprom ; <i64*> [#uses=1]
  %tmp9 = load i64* %arrayidx8                    ; <i64> [#uses=1]
  %tmp10 = load i32* %i                           ; <i32> [#uses=1]
  %tmp11 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %idxprom12 = sext i32 %tmp10 to i64             ; <i64> [#uses=1]
  %arrayidx13 = getelementptr inbounds i64* %tmp11, i64 %idxprom12 ; <i64*> [#uses=1]
  store i64 %tmp9, i64* %arrayidx13
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp14 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp14, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %if.end42

if.else:                                          ; preds = %entry
  store i32 0, i32* %i16
  br label %for.cond17

for.cond17:                                       ; preds = %for.inc38, %if.else
  %tmp18 = load i32* %i16                         ; <i32> [#uses=1]
  %conv19 = sext i32 %tmp18 to i64                ; <i64> [#uses=1]
  %tmp20 = load i64* %n.addr                      ; <i64> [#uses=1]
  %add = add nsw i64 %tmp20, 4                    ; <i64> [#uses=1]
  %cmp21 = icmp slt i64 %conv19, %add             ; <i1> [#uses=1]
  br i1 %cmp21, label %for.body23, label %for.end41

for.body23:                                       ; preds = %for.cond17
  %tmp24 = load i32* %i16                         ; <i32> [#uses=1]
  %cmp25 = icmp eq i32 %tmp24, 80                 ; <i1> [#uses=1]
  br i1 %cmp25, label %if.then27, label %if.end

if.then27:                                        ; preds = %for.body23
  br label %crazy

if.end:                                           ; preds = %for.body23
  %tmp28 = load i32* %i16                         ; <i32> [#uses=1]
  %mul = mul i32 %tmp28, 2                        ; <i32> [#uses=1]
  %tmp29 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %idxprom30 = sext i32 %mul to i64               ; <i64> [#uses=1]
  %arrayidx31 = getelementptr inbounds i64* %tmp29, i64 %idxprom30 ; <i64*> [#uses=1]
  %tmp32 = load i64* %arrayidx31                  ; <i64> [#uses=1]
  %tmp33 = load i32* %i16                         ; <i32> [#uses=1]
  %add34 = add nsw i32 %tmp33, 4                  ; <i32> [#uses=1]
  %tmp35 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %idxprom36 = sext i32 %add34 to i64             ; <i64> [#uses=1]
  %arrayidx37 = getelementptr inbounds i64* %tmp35, i64 %idxprom36 ; <i64*> [#uses=1]
  store i64 %tmp32, i64* %arrayidx37
  br label %for.inc38

for.inc38:                                        ; preds = %if.end
  %tmp39 = load i32* %i16                         ; <i32> [#uses=1]
  %inc40 = add nsw i32 %tmp39, 1                  ; <i32> [#uses=1]
  store i32 %inc40, i32* %i16
  br label %for.cond17

for.end41:                                        ; preds = %for.cond17
  br label %if.end42

if.end42:                                         ; preds = %for.end41, %for.end
  ret void
}
