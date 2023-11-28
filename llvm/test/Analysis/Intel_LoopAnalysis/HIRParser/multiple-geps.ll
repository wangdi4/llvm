; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output -hir-details-dims 2>&1 | FileCheck %s

; NOTE: convert-to-subscript does not seem to be behaviing correctly for this case but it isn't important because this conversion doesn't happen in the actual compiler pipeline.
; opt < %s -passes="convert-to-subscript,hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output -hir-details-dims 2>&1 | FileCheck %s

; Check parsing output for the loop verifying that the load and store whose address is formed using multiple geps is parsed correctly.

; BEGIN REGION { }
;       + DO i1 = 0, %n + -1, 1
;       |   + DO i2 = 0, %n + -1, 1
;       |   |   %0 = (@B)[0][i2][i1];
;       |   |   (@A)[0][i1][i2] = %0;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK:      + DO i1 = 0, %n + -1
; CHECK-NEXT: |
; CHECK-NEXT: |   + DO i2 = 0, %n + -1
; CHECK-NEXT: |   |   %[[TMP:.*]] = (@B)[0:0:40000([100 x [100 x i32]]:0)][0:i2:400([100 x i32]:100)][0:i1:4(i32:100)]
; CHECK-NEXT: |   |   (@A)[0:0:40000([100 x [100 x i32]]:0)][0:i1:400([100 x i32]:100)][0:i2:4(i32:100)] = %[[TMP]]
; CHECK-NEXT: |   + END LOOP
; CHECK-NEXT: |
; CHECK-NEXT: + END LOOP


; ModuleID = 'multiple-gep.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = common global [100 x [100 x i32]] zeroinitializer, align 16
@A = common global [100 x [100 x i32]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n) {
entry:
  %cmp.3 = icmp slt i32 0, %n
  br i1 %cmp.3, label %for.body.lr.ph, label %for.end.12

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.inc.10
  %i.04 = phi i32 [ 0, %for.body.lr.ph ], [ %inc11, %for.inc.10 ]
  %cmp2.1 = icmp slt i32 0, %n
  br i1 %cmp2.1, label %for.body.3.lr.ph, label %for.end

for.body.3.lr.ph:                                 ; preds = %for.body
  br label %for.body.3

for.body.3:                                       ; preds = %for.body.3.lr.ph, %for.inc
  %j.02 = phi i32 [ 0, %for.body.3.lr.ph ], [ %inc, %for.inc ]
  %idxprom = sext i32 %i.04 to i64
  %idxprom4 = sext i32 %j.02 to i64
  %arrayidx = getelementptr inbounds [100 x [100 x i32]], ptr @B, i64 0, i64 %idxprom4
  %arrayidx5 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx, i64 0, i64 %idxprom
  %0 = load i32, ptr %arrayidx5, align 4
  %idxprom6 = sext i32 %j.02 to i64
  %idxprom7 = sext i32 %i.04 to i64
  %arrayidx8 = getelementptr inbounds [100 x [100 x i32]], ptr @A, i64 0, i64 %idxprom7
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx8, i64 0, i64 %idxprom6
  store i32 %0, ptr %arrayidx9, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body.3
  %inc = add nsw i32 %j.02, 1
  %cmp2 = icmp slt i32 %inc, %n
  br i1 %cmp2, label %for.body.3, label %for.cond.1.for.end_crit_edge

for.cond.1.for.end_crit_edge:                     ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond.1.for.end_crit_edge, %for.body
  br label %for.inc.10

for.inc.10:                                       ; preds = %for.end
  %inc11 = add nsw i32 %i.04, 1
  %cmp = icmp slt i32 %inc11, %n
  br i1 %cmp, label %for.body, label %for.cond.for.end.12_crit_edge

for.cond.for.end.12_crit_edge:                    ; preds = %for.inc.10
  br label %for.end.12

for.end.12:                                       ; preds = %for.cond.for.end.12_crit_edge, %entry
  ret void
}

