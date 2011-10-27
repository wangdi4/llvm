; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample4.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: ret
define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %n.addr = alloca i64, align 8                   ; <i64*> [#uses=4]
  %A.addr = alloca i64*, align 8                  ; <i64**> [#uses=2]
  %B.addr = alloca i64*, align 8                  ; <i64**> [#uses=3]
  %sum = alloca i64, align 8                      ; <i64*> [#uses=5]
  %i = alloca i64, align 8                        ; <i64*> [#uses=5]
  store i64 %n, i64* %n.addr
  store i64* %A, i64** %A.addr
  store i64* %B, i64** %B.addr
  %call = call i32 @get_local_id(i32 0)           ; <i32> [#uses=1]
  %conv = zext i32 %call to i64                   ; <i64> [#uses=1]
  store i64 %conv, i64* %sum
  store i64 0, i64* %i
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %tmp = load i64* %sum                           ; <i64> [#uses=1]
  %tmp1 = load i64* %n.addr                       ; <i64> [#uses=1]
  %cmp = icmp slt i64 %tmp, %tmp1                 ; <i1> [#uses=1]
  br i1 %cmp, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %while.cond
  %tmp3 = load i64* %i                            ; <i64> [#uses=1]
  %tmp4 = load i64* %n.addr                       ; <i64> [#uses=1]
  %cmp5 = icmp slt i64 %tmp3, %tmp4               ; <i1> [#uses=1]
  br label %land.end

land.end:                                         ; preds = %land.rhs, %while.cond
  %0 = phi i1 [ false, %while.cond ], [ %cmp5, %land.rhs ] ; <i1> [#uses=1]
  br i1 %0, label %while.body, label %while.end

while.body:                                       ; preds = %land.end
  %tmp7 = load i64* %i                            ; <i64> [#uses=1]
  %inc = add nsw i64 %tmp7, 1                     ; <i64> [#uses=2]
  store i64 %inc, i64* %i
  %tmp8 = load i64** %A.addr                      ; <i64*> [#uses=1]
  %arrayidx = getelementptr inbounds i64* %tmp8, i64 %inc ; <i64*> [#uses=1]
  %tmp9 = load i64* %arrayidx                     ; <i64> [#uses=1]
  %tmp10 = load i64* %sum                         ; <i64> [#uses=1]
  %add = add nsw i64 %tmp10, %tmp9                ; <i64> [#uses=1]
  store i64 %add, i64* %sum
  %call11 = call i32 @get_local_id(i32 0)         ; <i32> [#uses=1]
  %conv12 = zext i32 %call11 to i64               ; <i64> [#uses=1]
  %tmp13 = load i64* %i                           ; <i64> [#uses=1]
  %tmp14 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx15 = getelementptr inbounds i64* %tmp14, i64 %tmp13 ; <i64*> [#uses=1]
  store i64 %conv12, i64* %arrayidx15
  br label %while.cond

while.end:                                        ; preds = %land.end
  %tmp16 = load i64* %sum                         ; <i64> [#uses=1]
  %tmp17 = load i64* %n.addr                      ; <i64> [#uses=1]
  %div = sdiv i64 %tmp17, 2                       ; <i64> [#uses=1]
  %tmp18 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx19 = getelementptr inbounds i64* %tmp18, i64 %div ; <i64*> [#uses=1]
  store i64 %tmp16, i64* %arrayidx19
  ret void
}

declare i32 @get_local_id(i32)
