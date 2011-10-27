; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'file.s'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

define i32 @m33ain(i32 %x, i32 %y) nounwind {
entry:
  %tobool = icmp eq i32 %x, 0                     ; <i1> [#uses=1]
  br i1 %tobool, label %if.else, label %if.then

if.then:                                          ; preds = %entry
  tail call void @g(i32 %y) nounwind
  br label %UnifiedReturnBlock

if.else:                                          ; preds = %entry
  %tobool3 = icmp eq i32 %y, 0                    ; <i1> [#uses=1]
  br i1 %tobool3, label %if.else6, label %if.then4

if.then4:                                         ; preds = %if.else
  tail call void @g(i32 %x) nounwind
  br label %UnifiedReturnBlock

if.else6:                                         ; preds = %if.else
  %sub = add i32 %y, -5                           ; <i32> [#uses=1]
  tail call void @g(i32 %sub) nounwind
  br label %UnifiedReturnBlock

; CHECK: br label %UnifiedReturnBlock
; CHECK: phi-split-bb
; CHECK: UnifiedReturnBlock:
; CHECK: preds =
; CHECK: phi-split-bb

UnifiedReturnBlock:                               ; preds = %if.else6, %if.then4, %if.then
  %UnifiedRetVal = phi i32 [ 2, %if.then ], [ 1, %if.then4 ], [ 4, %if.else6 ] ; <i32> [#uses=1]
  ret i32 %UnifiedRetVal
}

declare void @g(i32)
