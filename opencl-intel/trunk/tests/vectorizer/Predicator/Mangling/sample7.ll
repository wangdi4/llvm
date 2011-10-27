; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample7.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
define void @func(i64 %n, i64* %A, i64* %B) nounwind {
; CHECK-NOT: @masked
; CHECK: ret
entry:
  %n.addr = alloca i64, align 8                   ; <i64*> [#uses=3]
  %A.addr = alloca i64*, align 8                  ; <i64**> [#uses=2]
  %B.addr = alloca i64*, align 8                  ; <i64**> [#uses=3]
  %sum = alloca i64, align 8                      ; <i64*> [#uses=6]
  %i = alloca i64, align 8                        ; <i64*> [#uses=4]
  %i7 = alloca i64, align 8                       ; <i64*> [#uses=5]
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
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  %tmp2 = load i64** %A.addr                      ; <i64*> [#uses=1]
  %idxprom = zext i32 %call to i64                ; <i64> [#uses=1]
  %arrayidx = getelementptr inbounds i64* %tmp2, i64 %idxprom ; <i64*> [#uses=1]
  %tmp3 = load i64* %arrayidx                     ; <i64> [#uses=1]
  %tmp4 = load i64* %sum                          ; <i64> [#uses=1]
  %add = add nsw i64 %tmp4, %tmp3                 ; <i64> [#uses=1]
  store i64 %add, i64* %sum
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %tmp5 = load i64* %i                            ; <i64> [#uses=1]
  %inc = add nsw i64 %tmp5, 1                     ; <i64> [#uses=1]
  store i64 %inc, i64* %i
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i64 0, i64* %i7
  br label %for.cond8

for.cond8:                                        ; preds = %for.inc19, %for.end
  %tmp9 = load i64* %i7                           ; <i64> [#uses=1]
  %tmp10 = load i64* %n.addr                      ; <i64> [#uses=1]
  %cmp11 = icmp slt i64 %tmp9, %tmp10             ; <i1> [#uses=1]
  br i1 %cmp11, label %for.body12, label %for.end22

for.body12:                                       ; preds = %for.cond8
  %tmp13 = load i64* %i7                          ; <i64> [#uses=1]
  %tmp14 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx15 = getelementptr inbounds i64* %tmp14, i64 %tmp13 ; <i64*> [#uses=1]
  %tmp16 = load i64* %arrayidx15                  ; <i64> [#uses=1]
  %tmp17 = load i64* %sum                         ; <i64> [#uses=1]
  %add18 = add nsw i64 %tmp17, %tmp16             ; <i64> [#uses=1]
  store i64 %add18, i64* %sum
  br label %for.inc19

for.inc19:                                        ; preds = %for.body12
  %tmp20 = load i64* %i7                          ; <i64> [#uses=1]
  %inc21 = add nsw i64 %tmp20, 1                  ; <i64> [#uses=1]
  store i64 %inc21, i64* %i7
  br label %for.cond8

for.end22:                                        ; preds = %for.cond8
  %tmp23 = load i64* %sum                         ; <i64> [#uses=1]
  %tmp24 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx25 = getelementptr inbounds i64* %tmp24, i64 0 ; <i64*> [#uses=1]
  store i64 %tmp23, i64* %arrayidx25
  ret void
}

declare i32 @get_local_id(i32)
