; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
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
  %arrayidx = getelementptr inbounds i64* %A, i64 122
  store i64 6, i64* %arrayidx, align 8
  %tmp4 = load i64* %A, align 8
  %tobool = icmp eq i64 %tmp4, 0
  br i1 %tobool, label %if.end18, label %if.then

if.then:                                          ; preds = %entry
  %arrayidx6 = getelementptr inbounds i64* %A, i64 12
  store i64 2, i64* %arrayidx6, align 8
  %arrayidx8 = getelementptr inbounds i64* %A, i64 1
  %tmp9 = load i64* %arrayidx8, align 8
  %tobool10 = icmp eq i64 %tmp9, 0
  br i1 %tobool10, label %if.else, label %if.then11

if.then11:                                        ; preds = %if.then
  %arrayidx13 = getelementptr inbounds i64* %A, i64 3
  store i64 2, i64* %arrayidx13, align 8
  br label %if.end

if.else:                                          ; preds = %if.then
  %arrayidx15 = getelementptr inbounds i64* %A, i64 24
  store i64 9, i64* %arrayidx15, align 8
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then11
  %arrayidx17 = getelementptr inbounds i64* %A, i64 9
  store i64 2, i64* %arrayidx17, align 8
  br label %if.end18

if.end18:                                         ; preds = %entry, %if.end
  ret void
}
