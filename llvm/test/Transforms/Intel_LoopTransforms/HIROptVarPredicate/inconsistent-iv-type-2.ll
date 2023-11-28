; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-opt-var-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output -S < %s 2>&1 | FileCheck %s

; In the example below DO i2 IV type is i32 and "if (-1 * i2 < %0)" %0 is i64.

; Check that result loop UB does not contain unnecessary casts: "-1 * smax(0, (1 + (-1 * %0))) + 7".

;  BEGIN REGION { }
;        + DO i1 = 0, 67, 1   <DO_LOOP>
;        |   if (-1 * i1 + 69 == 9)
;        |   {
;        |      (%jh6)[0] = 9;
;        |
;        |      + DO i2 = 0, 7, 1   <DO_LOOP>
;        |      |   if (-1 * i2 < %0)
;        |      |   {
;        |      |      %1 = (%arrayidx)[0];
;        |      |      %2 = (%b)[0];
;        |      |      (%b)[0] = -1 * %1 + %2;
;        |      |      %3 = (%ia)[0];
;        |      |      (%ia)[0] = ((%0 + %3) * %3);
;        |      |      %4 = (%arrayidx19)[0];
;        |      |      %5 = (%arrayidx21)[0];
;        |      |      (%arrayidx21)[0] = -1 * %4 + %5;
;        |      |   }
;        |      + END LOOP
;        |
;        |      (%jh6)[0] = 1;
;        |   }
;        + END LOOP
;  END REGION

; CHECK:      BEGIN REGION { modified }
; CHECK-NEXT:       (%jh6)[0] = 9;
;
; CHECK:            + DO i1 = 0, -1 * smax(0, (1 + (-1 * %0))) + 7, 1   <DO_LOOP>
; CHECK-NEXT:       |   %1 = (%arrayidx)[0];
; CHECK-NEXT:       |   %2 = (%b)[0];
; CHECK-NEXT:       |   (%b)[0] = -1 * %1 + %2;
; CHECK-NEXT:       |   %3 = (%ia)[0];
; CHECK-NEXT:       |   (%ia)[0] = ((%0 + %3) * %3);
; CHECK-NEXT:       |   %4 = (%arrayidx19)[0];
; CHECK-NEXT:       |   %5 = (%arrayidx21)[0];
; CHECK-NEXT:       |   (%arrayidx21)[0] = -1 * %4 + %5;
; CHECK-NEXT:       + END LOOP
;
; CHECK:            (%jh6)[0] = 1;
; CHECK-NEXT: END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @main.for.body(ptr %jh6, ptr %arrayidx, ptr %b, ptr %ia, i32 %0, ptr %arrayidx19, ptr %arrayidx21) #0 {
newFuncRoot:
  %n = sext i32 %0 to i64
  br label %for.body

for.end29.exitStub:                               ; preds = %for.inc27
  ret void

for.body:                                         ; preds = %newFuncRoot, %for.inc27
  %indvars.iv64 = phi i64 [ 69, %newFuncRoot ], [ %indvars.iv.next65, %for.inc27 ]
  %indvars.iv62 = phi i32 [ 67, %newFuncRoot ], [ %indvars.iv.next63, %for.inc27 ]
  %indvars.iv58 = phi i32 [ 59, %newFuncRoot ], [ %indvars.iv.next59, %for.inc27 ]
  %cmp3 = icmp eq i64 %indvars.iv64, 9
  br i1 %cmp3, label %for.cond4.preheader, label %for.inc27

for.cond4.preheader:                              ; preds = %for.body
  store i32 9, ptr %jh6, align 4
  br label %for.body6

for.body6:                                        ; preds = %for.inc24, %for.cond4.preheader
  %indvars.iv60 = phi i32 [ %indvars.iv58, %for.cond4.preheader ], [ %indvars.iv.next61, %for.inc24 ]
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.inc24 ]
  %cmp7 = icmp slt i64 %indvars.iv, %n
  br i1 %cmp7, label %if.end, label %for.inc24

if.end:                                           ; preds = %for.body6
  %1 = load i32, ptr %arrayidx, align 4
  %2 = load i32, ptr %b, align 4
  %sub11 = sub i32 %2, %1
  store i32 %sub11, ptr %b, align 4
  %3 = load i32, ptr %ia, align 4
  %add = add i32 %3, %0
  %mul = mul i32 %add, %3
  store i32 %mul, ptr %ia, align 4
  %4 = load i32, ptr %arrayidx19, align 4
  %5 = load i32, ptr %arrayidx21, align 4
  %sub22 = sub i32 %5, %4
  store i32 %sub22, ptr %arrayidx21, align 4
  br label %for.inc24

for.inc24:                                        ; preds = %if.end, %for.body6
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %indvars.iv.next61 = add nsw i32 %indvars.iv60, 1
  %exitcond = icmp eq i32 %indvars.iv.next61, %indvars.iv62
  br i1 %exitcond, label %for.inc27.loopexit, label %for.body6

for.inc27.loopexit:                               ; preds = %for.inc24
  store i32 1, ptr %jh6, align 4
  br label %for.inc27

for.inc27:                                        ; preds = %for.inc27.loopexit, %for.body
  %indvars.iv.next65 = add nsw i64 %indvars.iv64, -1
  %cmp = icmp ugt i64 %indvars.iv.next65, 1
  %indvars.iv.next59 = add nsw i32 %indvars.iv58, -1
  %indvars.iv.next63 = add nsw i32 %indvars.iv62, -1
  br i1 %cmp, label %for.body, label %for.end29.exitStub
}

