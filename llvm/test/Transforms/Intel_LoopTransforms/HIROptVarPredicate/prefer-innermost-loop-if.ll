; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -disable-output <%s 2>&1 | FileCheck %s

; Test checks that the condition number <30> is chosen over condition number <3>
; since it is in the innermost loop.

;<0>          BEGIN REGION { }
;<51>               + DO i1 = 0, 19, 1   <DO_LOOP>
;<3>                |   if (i1 <u 4)
;<3>                |   {
;<10>               |      (@c)[0][i1][i1] = i1 + -1;
;<3>                |   }
;<52>               |
;<52>               |   + DO i2 = 0, 19, 1   <DO_LOOP>
;<19>               |   |   if (i2 >u 3)
;<19>               |   |   {
;<24>               |   |      (@a)[0][i1][i2] = i1;
;<19>               |   |   }
;<29>               |   |   (@c)[0][i1][i2] = i2;
;<30>               |   |   if (i1 != 3)
;<30>               |   |   {
;<35>               |   |      (@b)[0][i1][i2] = i2;
;<30>               |   |   }
;<52>               |   + END LOOP
;<51>               + END LOOP
;<0>          END REGION

; HIR After Optimization:
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:           |   if (i1 <u 4)
;                  |   {
;                  |      (@c)[0][i1][i1] = i1 + -1;
;                  |   }
;                  |
; CHECK:           |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:           |   |   (@c)[0][i1][i2] = i2;
; CHECK:           |   |   (@b)[0][i1][i2] = i2;
; CHECK:           |   + END LOOP
;                  |
;                  |
; CHECK:           |   + DO i2 = 0, 15, 1   <DO_LOOP>
; CHECK:           |   |   (@a)[0][i1][i2 + 4] = i1;
; CHECK:           |   |   (@c)[0][i1][i2 + 4] = i2 + 4;
; CHECK:           |   |   (@b)[0][i1][i2 + 4] = i2 + 4;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
;
; CHECK:           (@c)[0][3][3] = 2;
;
; CHECK:           + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK:           |   (@c)[0][3][i1] = i1;
; CHECK:           + END LOOP
;
;
; CHECK:           + DO i1 = 0, 15, 1   <DO_LOOP>
; CHECK:           |   (@a)[0][3][i1 + 4] = 3;
; CHECK:           |   (@c)[0][3][i1 + 4] = i1 + 4;
; CHECK:           + END LOOP
;
;
; CHECK:           + DO i1 = 0, 15, 1   <DO_LOOP>
; CHECK:           |   if (i1 + 4 <u 4)
;                  |   {
;                  |      (@c)[0][i1 + 4][i1 + 4] = i1 + 3;
;                  |   }
;                  |
; CHECK:           |   + DO i2 = 0, 3, 1   <DO_LOOP>
; CHECK:           |   |   (@c)[0][i1 + 4][i2] = i2;
; CHECK:           |   |   (@b)[0][i1 + 4][i2] = i2;
; CHECK:           |   + END LOOP
;                  |
;                  |
; CHECK:           |   + DO i2 = 0, 15, 1   <DO_LOOP>
; CHECK:           |   |   (@a)[0][i1 + 4][i2 + 4] = i1 + 4;
; CHECK:           |   |   (@c)[0][i1 + 4][i2 + 4] = i2 + 4;
; CHECK:           |   |   (@b)[0][i1 + 4][i2 + 4] = i2 + 4;
; CHECK:           |   + END LOOP
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = dso_local local_unnamed_addr global [20 x [20 x i32]] zeroinitializer, align 16
@a = dso_local local_unnamed_addr global [20 x [20 x i32]] zeroinitializer, align 16
@b = dso_local local_unnamed_addr global [20 x [20 x i32]] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind memory(write, argmem: none, inaccessiblemem: none) uwtable
define dso_local i32 @foo() local_unnamed_addr {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc25
  %indvars.iv47 = phi i64 [ 0, %entry ], [ %indvars.iv.next48, %for.inc25 ]
  %cmp1 = icmp ult i64 %indvars.iv47, 4
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds [20 x [20 x i32]], ptr @c, i64 0, i64 %indvars.iv47, i64 %indvars.iv47
  %0 = trunc i64 %indvars.iv47 to i32
  %1 = add nsw i32 %0, -1
  store i32 %1, ptr %arrayidx3, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %cmp18.not = icmp eq i64 %indvars.iv47, 3
  %2 = trunc i64 %indvars.iv47 to i32
  br label %for.body6

for.body6:                                        ; preds = %if.end, %for.inc
  %indvars.iv = phi i64 [ 0, %if.end ], [ %indvars.iv.next, %for.inc ]
  %cmp7 = icmp ugt i64 %indvars.iv, 3
  br i1 %cmp7, label %if.then8, label %if.end13

if.then8:                                         ; preds = %for.body6
  %arrayidx12 = getelementptr inbounds [20 x [20 x i32]], ptr @a, i64 0, i64 %indvars.iv47, i64 %indvars.iv
  store i32 %2, ptr %arrayidx12, align 4
  br label %if.end13

if.end13:                                         ; preds = %if.then8, %for.body6
  %arrayidx17 = getelementptr inbounds [20 x [20 x i32]], ptr @c, i64 0, i64 %indvars.iv47, i64 %indvars.iv
  %3 = trunc i64 %indvars.iv to i32
  store i32 %3, ptr %arrayidx17, align 4
  br i1 %cmp18.not, label %for.inc, label %if.then19

if.then19:                                        ; preds = %if.end13
  %arrayidx23 = getelementptr inbounds [20 x [20 x i32]], ptr @b, i64 0, i64 %indvars.iv47, i64 %indvars.iv
  store i32 %3, ptr %arrayidx23, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.end13, %if.then19
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 20
  br i1 %exitcond.not, label %for.inc25, label %for.body6

for.inc25:                                        ; preds = %for.inc
  %indvars.iv.next48 = add nuw nsw i64 %indvars.iv47, 1
  %exitcond50.not = icmp eq i64 %indvars.iv.next48, 20
  br i1 %exitcond50.not, label %for.end27, label %for.body

for.end27:                                        ; preds = %for.inc25
  ret i32 undef
}

