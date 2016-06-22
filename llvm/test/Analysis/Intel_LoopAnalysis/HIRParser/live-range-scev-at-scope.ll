; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Src code-

; for (a=0; a<n; a++)
;  for (b=0; b<n; b++)
;   for (c=0; c<n; c++)
;    for (d=0; d<n; d++)
;     for (e=0; e<n; e++)
;      for (f=0; f<n; f++)
;       x++;


; Check parsing output for the loop verifying that %2 is parsed as a blob outside i4 loop. The SCEV of %2 is an AddRec in terms of %x.267. Propagating this information outside the loop leads to live range violation as %x.267 is updated in i3 loop.

; CHECK:      + DO i1 = 0, %cond84 + -1, 1   <DO_LOOP>
; CHECK-NEXT: |   %x.171 = %x.075;
; CHECK-NEXT: |   + DO i2 = 0, %cond84 + -1, 1   <DO_LOOP>
; CHECK-NEXT: |   |   %x.267 = %x.171;
; CHECK-NEXT: |   |   + DO i3 = 0, %cond84 + -1, 1   <DO_LOOP>
; CHECK-NEXT: |   |   |   + DO i4 = 0, %cond84 + -1, 1   <DO_LOOP>
; CHECK-NEXT: |   |   |   |   %2 = (%cond84 * %cond84)  +  (%cond84 * %cond84) * i4 + %x.267;
; CHECK-NEXT: |   |   |   + END LOOP
; CHECK-NEXT: |   |   |   %x.267 = %2;
; CHECK-NEXT: |   |   + END LOOP
; CHECK-NEXT: |   |   %x.171 = %2;
; CHECK-NEXT: |   + END LOOP
; CHECK-NEXT: |   %x.075 = %2;
; CHECK-NEXT: + END LOOP



;Module Before HIR; ModuleID = 'nestedloop.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main(i32 %argc, i8** nocapture readonly %argv) {
entry:
  %cmp = icmp eq i32 %argc, 2
  br i1 %cmp, label %cond.end, label %for.cond2.preheader.lr.ph

cond.end:                                         ; preds = %entry
  %arrayidx = getelementptr inbounds i8*, i8** %argv, i64 1
  %0 = load i8*, i8** %arrayidx, align 8
  %call.i = tail call i64 @strtol(i8* nocapture nonnull %0, i8** null, i32 10) 
  %conv.i = trunc i64 %call.i to i32
  %cmp173 = icmp sgt i32 %conv.i, 0
  br i1 %cmp173, label %for.cond2.preheader.lr.ph, label %for.end32

for.cond2.preheader.lr.ph:                        ; preds = %entry, %cond.end
  %cond84 = phi i32 [ %conv.i, %cond.end ], [ 46, %entry ]
  %1 = mul i32 %cond84, %cond84
  br label %for.cond5.preheader.preheader

for.cond5.preheader.preheader:                    ; preds = %for.cond2.preheader.lr.ph, %for.inc30
  %x.075 = phi i32 [ 0, %for.cond2.preheader.lr.ph ], [ %2, %for.inc30 ]
  %a.074 = phi i32 [ 0, %for.cond2.preheader.lr.ph ], [ %inc31, %for.inc30 ]
  br label %for.cond8.preheader.preheader

for.cond8.preheader.preheader:                    ; preds = %for.cond5.preheader.preheader, %for.inc27
  %x.171 = phi i32 [ %2, %for.inc27 ], [ %x.075, %for.cond5.preheader.preheader ]
  %b.070 = phi i32 [ %inc28, %for.inc27 ], [ 0, %for.cond5.preheader.preheader ]
  br label %for.cond11.preheader.preheader

for.cond11.preheader.preheader:                   ; preds = %for.cond8.preheader.preheader, %for.inc24
  %x.267 = phi i32 [ %2, %for.inc24 ], [ %x.171, %for.cond8.preheader.preheader ]
  %c.066 = phi i32 [ %inc25, %for.inc24 ], [ 0, %for.cond8.preheader.preheader ]
  br label %for.inc21

for.inc21:                                        ; preds = %for.inc21, %for.cond11.preheader.preheader
  %x.363 = phi i32 [ %2, %for.inc21 ], [ %x.267, %for.cond11.preheader.preheader ]
  %d.062 = phi i32 [ %inc22, %for.inc21 ], [ 0, %for.cond11.preheader.preheader ]
  %2 = add i32 %1, %x.363
  %inc22 = add nuw nsw i32 %d.062, 1
  %exitcond78 = icmp eq i32 %inc22, %cond84
  br i1 %exitcond78, label %for.inc24, label %for.inc21

for.inc24:                                        ; preds = %for.inc21
  %inc25 = add nuw nsw i32 %c.066, 1
  %exitcond79 = icmp eq i32 %inc25, %cond84
  br i1 %exitcond79, label %for.inc27, label %for.cond11.preheader.preheader

for.inc27:                                        ; preds = %for.inc24
  %inc28 = add nuw nsw i32 %b.070, 1
  %exitcond80 = icmp eq i32 %inc28, %cond84
  br i1 %exitcond80, label %for.inc30, label %for.cond8.preheader.preheader

for.inc30:                                        ; preds = %for.inc27
  %inc31 = add nuw nsw i32 %a.074, 1
  %exitcond81 = icmp eq i32 %inc31, %cond84
  br i1 %exitcond81, label %for.end32.loopexit, label %for.cond5.preheader.preheader

for.end32.loopexit:                               ; preds = %for.inc30
  br label %for.end32

for.end32:                                        ; preds = %for.end32.loopexit, %cond.end
  %x.0.lcssa = phi i32 [ 0, %cond.end ], [ %2, %for.end32.loopexit ]
  %call33 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i64 0, i64 0), i32 %x.0.lcssa)
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) 

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) 

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) 

; Function Attrs: nounwind
declare i64 @strtol(i8* readonly, i8** nocapture, i32) 

