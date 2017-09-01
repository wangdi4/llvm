; RUN: opt < %s -hir-ssa-deconstruction -hir-cost-model-throttling=0 | opt -analyze -hir-parser -hir-cost-model-throttling=0 | FileCheck %s
; RUN: opt < %s -hir-ssa-deconstruction -hir-cg -force-hir-cg -hir-cost-model-throttling=0 -S | FileCheck -check-prefix=CHECK-CG %s

; Check parsing output for the loop verifying that the switch is parsed correctly.

; CHECK: + DO i1 = 0, %n + -1, 1   <DO_LOOP>
; CHECK: |   switch(%c)
; CHECK: |   {
; CHECK: |   case 0:
; CHECK: |      %call = @printf(&((@.str)[0][0]));
; CHECK: |      break;
; CHECK: |   case 1:
; CHECK: |      break;
; CHECK: |   default:
; CHECK: |      %call3 = @printf(&((@.str2)[0][0]));
; CHECK: |      goto for.inc;
; CHECK: |   }
; CHECK: |   %call2 = @printf(&((@.str1)[0][0]));
; CHECK: |   for.inc:
; CHECK: + END LOOP


; Check cg for this switch.

;CHECK-CG: region.0:
;CHECK-CG: loop.[[LOOP_NUM:[0-9]+]]:
;CHECK-CG-NEXT: switch i32 %c, label %[[SWITCH_NAME:hir.sw.[0-9]+]].default [
;CHECK-CG-NEXT: i32 0, label %[[SWITCH_NAME]].case.0
;CHECK-CG-NEXT: i32 1, label %[[SWITCH_NAME]].case.1
;CHECK-CG-NEXT: ]

;CHECK-CG: [[SWITCH_NAME]].default:
;CHECK-CG: call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str2, i64 0, i64 0))
;CHECK-CG: br label %hir.for.inc

; Check loop backedge
;CHECK-CG: hir.for.inc:
;CHECK-CG: br i1 %condloop.[[LOOP_NUM]], label %loop.[[LOOP_NUM]], label %afterloop.[[LOOP_NUM]]

; case0 and case1 jump to a merge block before jumping to the backedge
;CHECK-CG: [[SWITCH_NAME]].case.0:
;CHECK-CG: call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str, i64 0, i64 0))
;CHECK-CG: br label %[[SWITCH_NAME]].end

;CHECK-CG: [[SWITCH_NAME]].case.1:
;CHECK-CG-NEXT: br label %[[SWITCH_NAME]].end

; Check the merge block of case0 and case1 leading to the backedge
;CHECK-CG: [[SWITCH_NAME]].end:
;CHECK-CG: call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str1, i64 0, i64 0))
;CHECK-CG: br label %hir.for.inc




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
