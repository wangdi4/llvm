; RUN: opt < %s -loop-simplify -hir-de-ssa | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the gep operator argument of printf is parsed correctly.
; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: %0 = (@A)[i1]
; CHECK-NEXT: %call = @printf(&((@.str)[0]),  %0)
; CHECK-NEXT: END LOOP


; ModuleID = 'gep-operator-ddref.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@A = common global [100 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n) {
entry:
  %cmp.4 = icmp sgt i32 %n, 0
  br i1 %cmp.4, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i64 0, i64 0), i32 %0) #1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) 


