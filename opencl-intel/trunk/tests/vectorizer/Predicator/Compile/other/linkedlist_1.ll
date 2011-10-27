; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = '/tmp/webcompile/_22246_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-linux-gnu"

%struct.LLIST = type { i32, %struct.LLIST* }

@.str = private constant [14 x i8] c"list is empty\00", align 1 ; <[14 x i8]*> [#uses=1]
@.str1 = private constant [16 x i8] c"print %p %p %d\0A\00", align 1 ; <[16 x i8]*> [#uses=1]

; CHECK: @list_search

define %struct.LLIST** @list_search(%struct.LLIST** %n, i32 %i) nounwind readonly {
entry:
  %0 = icmp eq %struct.LLIST** %n, null           ; <i1> [#uses=1]
  br i1 %0, label %bb6, label %bb4

bb1:                                              ; preds = %bb4
  %1 = getelementptr inbounds %struct.LLIST* %5, i64 0, i32 0 ; <i32*> [#uses=1]
  %2 = load i32* %1, align 8                      ; <i32> [#uses=1]
  %3 = icmp eq i32 %2, %i                         ; <i1> [#uses=1]
  br i1 %3, label %bb6, label %bb3

bb3:                                              ; preds = %bb1
  %4 = getelementptr inbounds %struct.LLIST* %5, i64 0, i32 1 ; <%struct.LLIST**> [#uses=1]
  br label %bb4

bb4:                                              ; preds = %bb3, %entry
  %n_addr.0 = phi %struct.LLIST** [ %4, %bb3 ], [ %n, %entry ] ; <%struct.LLIST**> [#uses=2]
  %5 = load %struct.LLIST** %n_addr.0, align 8    ; <%struct.LLIST*> [#uses=3]
  %6 = icmp eq %struct.LLIST* %5, null            ; <i1> [#uses=1]
  br i1 %6, label %bb6, label %bb1

bb6:                                              ; preds = %bb4, %bb1, %entry
  %.0 = phi %struct.LLIST** [ null, %entry ], [ null, %bb4 ], [ %n_addr.0, %bb1 ] ; <%struct.LLIST**> [#uses=1]
  ret %struct.LLIST** %.0
}

