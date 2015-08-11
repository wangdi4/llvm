; RUN: opt < %s -loop-simplify -hir-de-ssa | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the compare instruction is parsed correctly.
; CHECK: DO i1 = 0, %n + -1
; CHECK-NEXT: %small.030.out = %small.030
; CHECK-NEXT: %0 = (%A)[i1]
; CHECK-NEXT: %1 = (%B)[i1]
; CHECK-NEXT: %cmp3 = %0 < %1
; CHECK-NEXT: %tobool = %small.030.out != 0
; CHECK-NEXT: %small.030 = zext.i1.i8(%cmp3)  ||  zext.i1.i8(%tobool)
; CHECK-NEXT: if (%small.030 == 0)
; CHECK: (%B)[i1] = %0
; CHECK: (%A)[i1] = %1
; CHECK: END LOOP



; ModuleID = 'cmp1.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @_Z3fooPiS_i(i32* nocapture %A, i32* nocapture %B, i32 %n) {
entry:
  %cmp.28 = icmp sgt i32 %n, 0
  br i1 %cmp.28, label %for.body, label %for.end

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc ], [ 0, %entry ]
  %small.030 = phi i8 [ %frombool, %for.inc ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx2, align 4
  %cmp3 = icmp slt i32 %0, %1
  %tobool = icmp ne i8 %small.030, 0
  %cmp326 = zext i1 %cmp3 to i8
  %tobool27 = zext i1 %tobool to i8
  %frombool = or i8 %cmp326, %tobool27
  %tobool4 = icmp eq i8 %frombool, 0
  br i1 %tobool4, label %if.else, label %if.then

if.then:                                          ; preds = %for.body
  store i32 %1, i32* %arrayidx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  store i32 %0, i32* %arrayidx2, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %lftr.wideiv, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc, %entry
  ret void
}

