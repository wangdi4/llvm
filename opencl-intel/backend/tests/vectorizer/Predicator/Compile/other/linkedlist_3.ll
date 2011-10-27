; RUN: llvm-as %s -o %t.bc
; RUN: opt  -std-compile-opts -inline-threshold=4096 -inline -lowerswitch -mergereturn -loopsimplify -phicanon -predicate -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = '/tmp/webcompile/_22246_0.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-linux-gnu"

%struct.LLIST = type { i32, %struct.LLIST* }

@.str = private constant [14 x i8] c"list is empty\00", align 1 ; <[14 x i8]*> [#uses=1]
@.str1 = private constant [16 x i8] c"print %p %p %d\0A\00", align 1 ; <[16 x i8]*> [#uses=1]

; CHECK: @list_remove

define void @list_remove(%struct.LLIST** %p) nounwind {
entry:
  %0 = icmp eq %struct.LLIST** %p, null           ; <i1> [#uses=1]
  br i1 %0, label %return, label %bb

bb:                                               ; preds = %entry
  %1 = load %struct.LLIST** %p, align 8           ; <%struct.LLIST*> [#uses=3]
  %2 = icmp eq %struct.LLIST* %1, null            ; <i1> [#uses=1]
  br i1 %2, label %return, label %bb1

bb1:                                              ; preds = %bb
  %3 = getelementptr inbounds %struct.LLIST* %1, i64 0, i32 1 ; <%struct.LLIST**> [#uses=1]
  %4 = load %struct.LLIST** %3, align 8           ; <%struct.LLIST*> [#uses=1]
  store %struct.LLIST* %4, %struct.LLIST** %p, align 8
  %5 = bitcast %struct.LLIST* %1 to i8*           ; <i8*> [#uses=1]
  tail call void @free(i8* %5) nounwind
  ret void

return:                                           ; preds = %bb, %entry
  ret void
}

declare void @free(i8* nocapture) nounwind

