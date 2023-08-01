; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details -hir-framework-debug=parser -disable-output  2>&1 | FileCheck %s

; Verify that we mark %retval.0.i as liveout of the i2 loop.

; + DO i1 = 0, 3, 1   <DO_LOOP>
; |   + DO i2 = 0, 3, 1   <DO_MULTI_EXIT_LOOP>
; |   |   %retval.0.i = 0;
; |   |   if (%0 == 0)
; |   |   {
; |   |      goto thorough_check.exit;
; |   |   }
; |   |   %retval.0.i = 1;
; |   + END LOOP
; |
; |   thorough_check.exit:
; |   %add4 = %add4  +  %retval.0.i;
; + END LOOP


; CHECK: + LiveIn symbases: [[ADD4:.*]], [[ZERO:.*]]
; CHECK: + LiveOut symbases: [[ADD4]]
; CHECK: + DO i8 i1 = 0, 3, 1

; CHECK: |   + LiveIn symbases: [[ZERO]]
; CHECK: |   + LiveOut symbases: [[RETVAL:[0-9]+]]
; CHECK: |   + DO i32 i2 = 0, 3, 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@n = common local_unnamed_addr global i32 0, align 4
@sum = common local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind uwtable
define void @foo() {
entry:
  %0 = load i32, ptr @n, align 4
  %tobool.i = icmp eq i32 %0, 0
  %sum.promoted = load i32, ptr @sum, align 4
  br label %for.body

for.body:                                         ; preds = %entry, %thorough_check.exit
  %add4 = phi i32 [ %sum.promoted, %entry ], [ %add, %thorough_check.exit ]
  %k.03 = phi i8 [ 3, %entry ], [ %dec, %thorough_check.exit ]
  br label %for.body.i

for.cond.i:                                       ; preds = %for.body.i
  %cmp.i = icmp ult i32 %inc.i, 4
  br i1 %cmp.i, label %for.body.i, label %thorough_check.exit

for.body.i:                                       ; preds = %for.cond.i, %for.body
  %i.03.i = phi i32 [ 0, %for.body ], [ %inc.i, %for.cond.i ]
  %inc.i = add nuw nsw i32 %i.03.i, 1
  br i1 %tobool.i, label %thorough_check.exit, label %for.cond.i

thorough_check.exit:                              ; preds = %for.cond.i, %for.body.i
  %retval.0.i = phi i32 [ 0, %for.body.i ], [ 1, %for.cond.i ]
  %add = add nsw i32 %add4, %retval.0.i
  %dec = add nsw i8 %k.03, -1
  %cmp = icmp eq i8 %k.03, 0
  br i1 %cmp, label %for.end, label %for.body

for.end:                                          ; preds = %thorough_check.exit
  %add.lcssa = phi i32 [ %add, %thorough_check.exit ]
  store i32 %add.lcssa, ptr @sum, align 4
  ret void
}

