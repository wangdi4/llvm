; RUN: opt < %s -hir-ssa-deconstruction -hir-cost-model-throttling=0 | opt -analyze -enable-new-pm=0 -hir-loop-statistics -hir-cost-model-throttling=0 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-loop-statistics>" -hir-cost-model-throttling=0 -disable-output 2>&1 | FileCheck %s

; HIR-
;  + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;  |   switch(%c)
;  |   {
;  |   case 0:
;  |      %call = @printf(&((@.str)[0][0]));
;  |      goto sw.bb1;
;  |   case 1:
;  |      sw.bb1:
;  |      %call2 = @printf(&((@.str1)[0][0]));
;  |      break;
;  |   default:
;  |      %call3 = @printf(&((@.str2)[0][0]));
;  |      break;
;  |   }
;  + END LOOP

; Check that loop statistics are calculated correctly.
; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK:    Number of switches: 1
; CHECK:    Number of forward gotos: 1
; CHECK:    Number of forward goto target labels: 1
; CHECK:    Number of user calls: 3
; CHECK: + END LOOP


; ModuleID = 'loop-switch.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [5 x i8] c"zero\00", align 1
@.str1 = private unnamed_addr constant [4 x i8] c"one\00", align 1
@.str2 = private unnamed_addr constant [8 x i8] c"default\00", align 1

define void @foo(i32 %c, i32 %n) {
entry:
  %cmp5 = icmp sgt i32 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %i.06 = phi i32 [ %inc, %for.inc ], [ 0, %for.body.preheader ]
  switch i32 %c, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb1
  ]

sw.bb:                                            ; preds = %for.body
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str, i64 0, i64 0))
  br label %sw.bb1

sw.bb1:                                           ; preds = %sw.bb, %for.body
  %call2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str1, i64 0, i64 0))
  br label %for.inc

sw.default:                                       ; preds = %for.body
  %call3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str2, i64 0, i64 0))
  br label %for.inc

for.inc:                                          ; preds = %sw.default, %sw.bb1
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare i32 @printf(i8* nocapture readonly, ...)
