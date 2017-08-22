;Simple CG for switch
;RUN:  opt -hir-cg -S -force-hir-cg -hir-cost-model-throttling=0 %s | FileCheck %s


;          BEGIN REGION { }
;<21>         + DO i1 = 0, %n + -1, 1   <DO_LOOP>
;<2>          |   switch(%c)
;<2>          |   {
;<2>          |   case 0:
;<16>         |      %call = @printf(&((@.str)[0][0]));
;<2>          |      break;
;<2>          |   case 1:
;<19>         |      %call2 = @printf(&((@.str1)[0][0]));
;<2>          |      break;
;<2>          |   default:
;<7>          |      %call3 = @printf(&((@.str2)[0][0]));
;<2>          |      break;
;<2>          |   }
;<21>         + END LOOP
;          END REGION

;CHECK: region.0:
;CHECK: loop.[[LOOP_NUM:[0-9]+]]:
;CHECK-NEXT: switch i32 %c, label %[[SWITCH_NAME:hir.sw.[0-9]+]].default [
;CHECK-NEXT: i32 0, label %[[SWITCH_NAME]].case.0
;CHECK-NEXT: i32 1, label %[[SWITCH_NAME]].case.1
;CHECK-NEXT: ]

;CHECK: [[SWITCH_NAME]].default:
;CHECK: call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str2, i64 0, i64 0))
;CHECK: br label %[[SWITCH_NAME]].end

;CHECK: [[SWITCH_NAME]].case.0:
;CHECK: call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str, i64 0, i64 0))
;CHECK: br label %[[SWITCH_NAME]].end

;CHECK: [[SWITCH_NAME]].case.1:
;CHECK: call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str1, i64 0, i64 0))
;CHECK: br label %[[SWITCH_NAME]].end

;CHECK: [[SWITCH_NAME]].end:
;CHECK: br i1 %condloop.[[LOOP_NUM]], label %loop.[[LOOP_NUM]], label %afterloop.[[LOOP_NUM]]

; ModuleID = '<stdin>'
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
  br label %for.inc

sw.bb1:                                           ; preds = %for.body
  %call2 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str1, i64 0, i64 0))
  br label %for.inc

sw.default:                                       ; preds = %for.body
  %call3 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str2, i64 0, i64 0))
  br label %for.inc

for.inc:                                          ; preds = %sw.default, %sw.bb1, %sw.bb
  %inc = add nuw nsw i32 %i.06, 1
  %exitcond = icmp eq i32 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

declare i32 @printf(i8* nocapture readonly, ...)
