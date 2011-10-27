; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sampleA.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
define void @func(i64 %n, i64* %A, i64* %B) nounwind {
; CHECK-NOT: @masked
; CHECK: ret
entry:
  %n.addr = alloca i64, align 8                   ; <i64*> [#uses=3]
  %A.addr = alloca i64*, align 8                  ; <i64**> [#uses=3]
  %B.addr = alloca i64*, align 8                  ; <i64**> [#uses=4]
  %i = alloca i32, align 4                        ; <i32*> [#uses=6]
  %i12 = alloca i32, align 4                      ; <i32*> [#uses=6]
  store i64 %n, i64* %n.addr
  store i64* %A, i64** %A.addr
  store i64* %B, i64** %B.addr
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  %rem = urem i32 %call, 2                        ; <i32> [#uses=1]
  %tobool = icmp ne i32 %rem, 0                   ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i32 0, i32* %i
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.then
  %tmp = load i32* %i                             ; <i32> [#uses=1]
  %conv = sext i32 %tmp to i64                    ; <i64> [#uses=1]
  %tmp1 = load i64* %n.addr                       ; <i64> [#uses=1]
  %cmp = icmp slt i64 %conv, %tmp1                ; <i1> [#uses=1]
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %tmp3 = load i32* %i                            ; <i32> [#uses=1]
  %tmp4 = load i64** %B.addr                      ; <i64*> [#uses=1]
  %idxprom = sext i32 %tmp3 to i64                ; <i64> [#uses=1]
  %arrayidx = getelementptr inbounds i64* %tmp4, i64 %idxprom ; <i64*> [#uses=1]
  %tmp5 = load i64* %arrayidx                     ; <i64> [#uses=1]
  %tmp6 = load i32* %i                            ; <i32> [#uses=1]
  %tmp7 = load i64** %A.addr                      ; <i64*> [#uses=1]
  %idxprom8 = sext i32 %tmp6 to i64               ; <i64> [#uses=1]
  %arrayidx9 = getelementptr inbounds i64* %tmp7, i64 %idxprom8 ; <i64*> [#uses=1]
  store i64 %tmp5, i64* %arrayidx9
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp10 = load i32* %i                           ; <i32> [#uses=1]
  %inc = add nsw i32 %tmp10, 1                    ; <i32> [#uses=1]
  store i32 %inc, i32* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %if.end

if.else:                                          ; preds = %entry
  store i32 0, i32* %i12
  br label %for.cond13

for.cond13:                                       ; preds = %for.inc29, %if.else
  %tmp14 = load i32* %i12                         ; <i32> [#uses=1]
  %conv15 = sext i32 %tmp14 to i64                ; <i64> [#uses=1]
  %tmp16 = load i64* %n.addr                      ; <i64> [#uses=1]
  %add = add nsw i64 %tmp16, 4                    ; <i64> [#uses=1]
  %cmp17 = icmp slt i64 %conv15, %add             ; <i1> [#uses=1]
  br i1 %cmp17, label %for.body19, label %for.end32

for.body19:                                       ; preds = %for.cond13
  %tmp20 = load i32* %i12                         ; <i32> [#uses=1]
  %tmp21 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %idxprom22 = sext i32 %tmp20 to i64             ; <i64> [#uses=1]
  %arrayidx23 = getelementptr inbounds i64* %tmp21, i64 %idxprom22 ; <i64*> [#uses=1]
  %tmp24 = load i64* %arrayidx23                  ; <i64> [#uses=1]
  %tmp25 = load i32* %i12                         ; <i32> [#uses=1]
  %tmp26 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %idxprom27 = sext i32 %tmp25 to i64             ; <i64> [#uses=1]
  %arrayidx28 = getelementptr inbounds i64* %tmp26, i64 %idxprom27 ; <i64*> [#uses=1]
  store i64 %tmp24, i64* %arrayidx28
  br label %for.inc29

for.inc29:                                        ; preds = %for.body19
  %tmp30 = load i32* %i12                         ; <i32> [#uses=1]
  %inc31 = add nsw i32 %tmp30, 1                  ; <i32> [#uses=1]
  store i32 %inc31, i32* %i12
  br label %for.cond13

for.end32:                                        ; preds = %for.cond13
  br label %if.end

if.end:                                           ; preds = %for.end32, %for.end
  %call33 = call i32 @get_global_id(i32 0)        ; <i32> [#uses=1]
  %conv34 = zext i32 %call33 to i64               ; <i64> [#uses=1]
  %tmp35 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx36 = getelementptr inbounds i64* %tmp35, i64 0 ; <i64*> [#uses=1]
  store i64 %conv34, i64* %arrayidx36
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)
