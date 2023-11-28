; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>,hir-last-value-computation,print<hir>,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that side effect in post-exit is captured and i2 loop will not be removed.

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   + DO i2 = 0, 99, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;       |   |   |   %x.0 = i2;
;       |   |   |   if (i1 == 5)
;       |   |   |   {
;       |   |   |      %x.0 = i1;
;       |   |   |   }
;       |   |   + END LOOP
;       |   |      (@A)[0][0] = %x.0;
;       |   + END LOOP
;       + END LOOP
; END REGION

; Skip HIR After first opt-pred
; CHECK-LABEL: Function: foo

; After Last Value Computation
; CHECK-LABEL: Function: foo
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   if (i1 == 5)
; CHECK:           |   {
; CHECK:           |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |      |   + DO i3 = 0, %n + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:           |      |   |   %x.0 = i2;
; CHECK:           |      |   + END LOOP
; CHECK:           |      |      %x.0 = i1;
; CHECK:           |      |      (@A)[0][0] = %x.0;
; CHECK:           |      + END LOOP
; CHECK:           |   }
; CHECK:           |   else
; CHECK:           |   {
; CHECK:           |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |      |   if (0 < %n)
; CHECK:           |      |   {
; CHECK:           |      |      %x.0 = i2;
; CHECK:           |      |      (@A)[0][0] = %x.0;
; CHECK:           |      |   }
; CHECK:           |      + END LOOP
; CHECK:           |   }
; CHECK:           + END LOOP
; CHECK:     END REGION

; We need to look at the second opt-pred output.
; CHECK-LABEL: Function: foo
; CHECK:     BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:           |   if (i1 == 5)
; CHECK:           |   {
; CHECK:           |      + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |      |   if (0 < %n)
; CHECK:           |      |   {
; CHECK:           |      |      %x.0 = i1;
; CHECK:           |      |      (@A)[0][0] = %x.0;
; CHECK:           |      |   }
; CHECK:           |      + END LOOP
; CHECK:           |   }
; CHECK:           |   else
; CHECK:           |   {
; CHECK:           |      if (0 < %n)
; CHECK:           |      {
; CHECK:           |         + DO i2 = 0, 99, 1   <DO_LOOP>
; CHECK:           |         |   %x.0 = i2;
; CHECK:           |         |   (@A)[0][0] = %x.0;
; CHECK:           |         + END LOOP
; CHECK:           |      }
; CHECK:           |   }
; CHECK:           + END LOOP
; CHECK:     END REGION

source_filename = "1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local global [100 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %cmp51 = icmp slt i32 0, %n
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc11
  %i.04 = phi i32 [ 0, %entry ], [ %inc12, %for.inc11 ]
  %cmp7 = icmp eq i32 %i.04, 5
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.inc8
  %k.03 = phi i32 [ 0, %for.body ], [ %inc9, %for.inc8 ]
  br i1 %cmp51, label %for.body6.lr.ph, label %for.end

for.body6.lr.ph:                                  ; preds = %for.body3
  br label %for.body6

for.body6:                                        ; preds = %for.body6.lr.ph, %for.inc
  %j.02 = phi i32 [ 0, %for.body6.lr.ph ], [ %inc, %for.inc ]
  br i1 %cmp7, label %if.then, label %if.end

if.then:                                          ; preds = %for.body6
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body6
  %x.0 = phi i32 [ %i.04, %if.then ], [ %k.03, %for.body6 ]
  br label %for.inc

for.inc:                                          ; preds = %if.end
  %inc = add nsw i32 %j.02, 1
  %cmp5 = icmp slt i32 %inc, %n
  br i1 %cmp5, label %for.body6, label %for.cond4.for.end_crit_edge

for.cond4.for.end_crit_edge:                      ; preds = %for.inc
  %x.0.lcssa = phi i32 [ %x.0, %for.inc ]
  store i32 %x.0.lcssa, ptr @A, align 16
  br label %for.end

for.end:                                          ; preds = %for.cond4.for.end_crit_edge, %for.body3
  br label %for.inc8

for.inc8:                                         ; preds = %for.end
  %inc9 = add nsw i32 %k.03, 1
  %cmp2 = icmp slt i32 %inc9, 100
  br i1 %cmp2, label %for.body3, label %for.end10

for.end10:                                        ; preds = %for.inc8
  br label %for.inc11

for.inc11:                                        ; preds = %for.end10
  %inc12 = add nsw i32 %i.04, 1
  %cmp = icmp slt i32 %inc12, 100
  br i1 %cmp, label %for.body, label %for.end13

for.end13:                                        ; preds = %for.inc11
  ret void
}
