; RUN: llvm-as %s -o %t.bc
; RUN: opt -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll


; ModuleID = 'sample1.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret
define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %n.addr = alloca i64, align 8                   ; <i64*> [#uses=1]
  %A.addr = alloca i64*, align 8                  ; <i64**> [#uses=8]
  %B.addr = alloca i64*, align 8                  ; <i64**> [#uses=2]
  %v = alloca i64, align 8                        ; <i64*> [#uses=1]
  store i64 %n, i64* %n.addr
  store i64* %A, i64** %A.addr
  store i64* %B, i64** %B.addr
  %tmp = load i64** %A.addr                       ; <i64*> [#uses=1]
  %arrayidx = getelementptr inbounds i64* %tmp, i64 122 ; <i64*> [#uses=1]
  store i64 6, i64* %arrayidx
  %tmp2 = load i64** %A.addr                      ; <i64*> [#uses=1]
  %arrayidx3 = getelementptr inbounds i64* %tmp2, i64 0 ; <i64*> [#uses=1]
  %tmp4 = load i64* %arrayidx3                    ; <i64> [#uses=1]
  %tobool = icmp ne i64 %tmp4, 0                  ; <i1> [#uses=1]
  br i1 %tobool, label %if.then, label %if.end18

if.then:                                          ; preds = %entry
  %tmp5 = load i64** %A.addr                      ; <i64*> [#uses=1]
  %arrayidx6 = getelementptr inbounds i64* %tmp5, i64 12 ; <i64*> [#uses=1]
  store i64 2, i64* %arrayidx6
  %tmp7 = load i64** %A.addr                      ; <i64*> [#uses=1]
  %arrayidx8 = getelementptr inbounds i64* %tmp7, i64 1 ; <i64*> [#uses=1]
  %tmp9 = load i64* %arrayidx8                    ; <i64> [#uses=1]
  %tobool10 = icmp ne i64 %tmp9, 0                ; <i1> [#uses=1]
  br i1 %tobool10, label %if.then11, label %if.else

if.then11:                                        ; preds = %if.then
  %tmp12 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx13 = getelementptr inbounds i64* %tmp12, i64 3 ; <i64*> [#uses=1]
  store i64 2, i64* %arrayidx13
  br label %if.end

if.else:                                          ; preds = %if.then
  %tmp14 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx15 = getelementptr inbounds i64* %tmp14, i64 24 ; <i64*> [#uses=1]
  store i64 9, i64* %arrayidx15
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then11
  %tmp16 = load i64** %A.addr                     ; <i64*> [#uses=1]
  %arrayidx17 = getelementptr inbounds i64* %tmp16, i64 9 ; <i64*> [#uses=1]
  store i64 2, i64* %arrayidx17
  br label %if.end18

if.end18:                                         ; preds = %if.end, %entry
  %tmp19 = load i64* %v                           ; <i64> [#uses=1]
  %tmp20 = load i64** %B.addr                     ; <i64*> [#uses=1]
  %arrayidx21 = getelementptr inbounds i64* %tmp20, i64 4 ; <i64*> [#uses=1]
  store i64 %tmp19, i64* %arrayidx21
  ret void
}
