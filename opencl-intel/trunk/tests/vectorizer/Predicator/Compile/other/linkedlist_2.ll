; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = '/tmp/webcompile/_22246_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-linux-gnu"

%struct.LLIST = type { i32, %struct.LLIST* }

@.str = private constant [14 x i8] c"list is empty\00", align 1 ; <[14 x i8]*> [#uses=1]
@.str1 = private constant [16 x i8] c"print %p %p %d\0A\00", align 1 ; <[16 x i8]*> [#uses=1]

; CHECK: @list_print

define void @list_print(%struct.LLIST* %n) nounwind {
entry:
  %0 = icmp eq %struct.LLIST* %n, null            ; <i1> [#uses=1]
  br i1 %0, label %bb2.preheader, label %bb1

bb2.preheader:                                    ; preds = %entry
  %1 = tail call i32 @puts(i8* getelementptr inbounds ([14 x i8]* @.str, i64 0, i64 0)) nounwind ; <i32> [#uses=0]
  ret void

bb1:                                              ; preds = %bb1, %entry
  %n_addr.04 = phi %struct.LLIST* [ %7, %bb1 ], [ %n, %entry ] ; <%struct.LLIST*> [#uses=3]
  %2 = getelementptr inbounds %struct.LLIST* %n_addr.04, i64 0, i32 0 ; <i32*> [#uses=1]
  %3 = load i32* %2, align 8                      ; <i32> [#uses=1]
  %4 = getelementptr inbounds %struct.LLIST* %n_addr.04, i64 0, i32 1 ; <%struct.LLIST**> [#uses=2]
  %5 = load %struct.LLIST** %4, align 8           ; <%struct.LLIST*> [#uses=1]
  %6 = tail call i32 (i8*, ...)* @printf(i8* noalias getelementptr inbounds ([16 x i8]* @.str1, i64 0, i64 0), %struct.LLIST* %n_addr.04, %struct.LLIST* %5, i32 %3) nounwind ; <i32> [#uses=0]
  %7 = load %struct.LLIST** %4, align 8           ; <%struct.LLIST*> [#uses=2]
  %8 = icmp eq %struct.LLIST* %7, null            ; <i1> [#uses=1]
  br i1 %8, label %return, label %bb1

return:                                           ; preds = %bb1
  ret void
}

declare i32 @puts(i8* nocapture) nounwind

declare i32 @printf(i8* nocapture, ...) nounwind

