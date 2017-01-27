; This is an example of hoisting 3 HLIfs where 1st and 3rd statements are proper linear and 2nd contains i1.

; RUN: opt -hir-ssa-deconstruction -hir-opt-predicate -print-after=hir-opt-predicate -S < %s 2>&1 | FileCheck %s

; Input HIR:
; BEGIN REGION { }
;   + DO i1 = 0, 999, 1   <DO_LOOP>
;   |   + DO i2 = 0, 999, 1   <DO_LOOP>
;   |   |   if (%x < 50)
;   |   |   {
;   |   |      (@a)[0][i1] = i2;
;   |   |   }
;   |   |   if (i1 < 50)
;   |   |   {
;   |   |      (@b)[0][i1] = i2;
;   |   |   }
;   |   |   if (%y > 0)
;   |   |   {
;   |   |      %2 = (@a)[0][i2];
;   |   |      (@a)[0][i2] = %y + %2;
;   |   |   }
;   |   + END LOOP
;   + END LOOP
; END REGION

;CHECK:      BEGIN REGION { modified }
;CHECK:      if (%x < 50)
;CHECK-NEXT: {
;CHECK-NEXT:   if (%y > 0)
;CHECK-NEXT:   {
;CHECK-NEXT:      + DO i1 = 0, 999, 1   <DO_LOOP>
;CHECK-NEXT:      |   if (i1 < 50)
;CHECK-NEXT:      |   {
;CHECK-NEXT:      |      + DO i2 = 0, 999, 1   <DO_LOOP>
;CHECK-NEXT:      |      |   (@a)[0][i1] = i2;
;CHECK-NEXT:      |      |   (@b)[0][i1] = i2;
;CHECK-NEXT:      |      |   %2 = (@a)[0][i2];
;CHECK-NEXT:      |      |   (@a)[0][i2] = %y + %2;
;CHECK-NEXT:      |      + END LOOP
;CHECK-NEXT:      |   }
;CHECK-NEXT:      |   else
;CHECK-NEXT:      |   {
;CHECK-NEXT:      |      + DO i2 = 0, 999, 1   <DO_LOOP>
;CHECK-NEXT:      |      |   (@a)[0][i1] = i2;
;CHECK-NEXT:      |      |   %2 = (@a)[0][i2];
;CHECK-NEXT:      |      |   (@a)[0][i2] = %y + %2;
;CHECK-NEXT:      |      + END LOOP
;CHECK-NEXT:      |   }
;CHECK-NEXT:      + END LOOP
;CHECK-NEXT:   }
;CHECK-NEXT:   else
;CHECK-NEXT:   {
;CHECK-NEXT:      + DO i1 = 0, 999, 1   <DO_LOOP>
;CHECK-NEXT:      |   if (i1 < 50)
;CHECK-NEXT:      |   {
;CHECK-NEXT:      |      + DO i2 = 0, 999, 1   <DO_LOOP>
;CHECK-NEXT:      |      |   (@a)[0][i1] = i2;
;CHECK-NEXT:      |      |   (@b)[0][i1] = i2;
;CHECK-NEXT:      |      + END LOOP
;CHECK-NEXT:      |   }
;CHECK-NEXT:      |   else
;CHECK-NEXT:      |   {
;CHECK-NEXT:      |      + DO i2 = 0, 999, 1   <DO_LOOP>
;CHECK-NEXT:      |      |   (@a)[0][i1] = i2;
;CHECK-NEXT:      |      + END LOOP
;CHECK-NEXT:      |   }
;CHECK-NEXT:      + END LOOP
;CHECK-NEXT:   }
;CHECK-NEXT: }
;CHECK-NEXT: else
;CHECK-NEXT: {
;CHECK-NEXT:   if (%y > 0)
;CHECK-NEXT:   {
;CHECK-NEXT:      + DO i1 = 0, 999, 1   <DO_LOOP>
;CHECK-NEXT:      |   if (i1 < 50)
;CHECK-NEXT:      |   {
;CHECK-NEXT:      |      + DO i2 = 0, 999, 1   <DO_LOOP>
;CHECK-NEXT:      |      |   (@b)[0][i1] = i2;
;CHECK-NEXT:      |      |   %2 = (@a)[0][i2];
;CHECK-NEXT:      |      |   (@a)[0][i2] = %y + %2;
;CHECK-NEXT:      |      + END LOOP
;CHECK-NEXT:      |   }
;CHECK-NEXT:      |   else
;CHECK-NEXT:      |   {
;CHECK-NEXT:      |      + DO i2 = 0, 999, 1   <DO_LOOP>
;CHECK-NEXT:      |      |   %2 = (@a)[0][i2];
;CHECK-NEXT:      |      |   (@a)[0][i2] = %y + %2;
;CHECK-NEXT:      |      + END LOOP
;CHECK-NEXT:      |   }
;CHECK-NEXT:      + END LOOP
;CHECK-NEXT:   }
;CHECK-NEXT:   else
;CHECK-NEXT:   {
;CHECK-NEXT:      + DO i1 = 0, 999, 1   <DO_LOOP>
;CHECK-NEXT:      |   if (i1 < 50)
;CHECK-NEXT:      |   {
;CHECK-NEXT:      |      + DO i2 = 0, 999, 1   <DO_LOOP>
;CHECK-NEXT:      |      |   (@b)[0][i1] = i2;
;CHECK-NEXT:      |      + END LOOP
;CHECK-NEXT:      |   }
;CHECK-NEXT:      + END LOOP
;CHECK-NEXT:   }
;CHECK-NEXT: }
;CHECK-NEXT: END REGION


;Module Before HIR; ModuleID = 'msm.c'
source_filename = "msm.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16
@b = common local_unnamed_addr global [1000 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i8* nocapture readonly %p, i8* nocapture readnone %q, i32 %x, i32 %y) local_unnamed_addr #0 {
entry:
  %cmp4 = icmp slt i32 %x, 50
  %cmp10 = icmp sgt i32 %y, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc15, %entry
  %indvars.iv32 = phi i64 [ 0, %entry ], [ %indvars.iv.next33, %for.inc15 ]
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %indvars.iv32
  %cmp5 = icmp slt i64 %indvars.iv32, 50
  %arrayidx8 = getelementptr inbounds [1000 x i32], [1000 x i32]* @b, i64 0, i64 %indvars.iv32
  br label %for.body3

for.body3:                                        ; preds = %for.inc, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.inc ]
  br i1 %cmp4, label %if.then, label %if.end

if.then:                                          ; preds = %for.body3
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body3
  br i1 %cmp5, label %if.then6, label %if.end9

if.then6:                                         ; preds = %if.end
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx8, align 4
  br label %if.end9

if.end9:                                          ; preds = %if.then6, %if.end
  br i1 %cmp10, label %if.then11, label %for.inc

if.then11:                                        ; preds = %if.end9
  %arrayidx13 = getelementptr inbounds [1000 x i32], [1000 x i32]* @a, i64 0, i64 %indvars.iv
  %2 = load i32, i32* %arrayidx13, align 4
  %add = add nsw i32 %2, %y
  store i32 %add, i32* %arrayidx13, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.end9, %if.then11
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.inc15, label %for.body3

for.inc15:                                        ; preds = %for.inc
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond34 = icmp eq i64 %indvars.iv.next33, 1000
  br i1 %exitcond34, label %for.end17, label %for.cond1.preheader

for.end17:                                        ; preds = %for.inc15
  %3 = load i8, i8* %p, align 1
  %conv = sext i8 %3 to i32
  ret i32 %conv
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

