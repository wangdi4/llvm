; RUN: opt -passes="hir-ssa-deconstruction,hir-opt-predicate,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Check that equal conditions be hoisted out of DO i1 loop.

;         BEGIN REGION { }
;               + DO i1 = 0, 99, 1   <DO_LOOP>
;               |   if (%n == 100)
;               |   {
;               |      + DO i2 = 0, 49, 1   <DO_LOOP>
;               |      |   %0 = (%a)[i2];
;               |      |   (%a)[i2] = i2 + %0;
;               |      + END LOOP
;               |   }
;               |   if (%n == 100)
;               |   {
;               |      + DO i2 = 0, 49, 1   <DO_LOOP>
;               |      |   %1 = (%a)[i2];
;               |      |   (%a)[i2] = i2 + %1;
;               |      + END LOOP
;               |   }
;               |   if (%n == 100)
;               |   {
;               |      + DO i2 = 0, 49, 1   <DO_LOOP>
;               |      |   %2 = (%a)[i2];
;               |      |   (%a)[i2] = i2 + %2;
;               |      + END LOOP
;               |   }
;               + END LOOP
;         END REGION

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        if (%n == 100)
; CHECK-NEXT:        {
; CHECK-NEXT:           + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-NEXT:           |   + DO i2 = 0, 49, 1   <DO_LOOP>
; CHECK-NEXT:           |   |   %0 = (%a)[i2];
; CHECK-NEXT:           |   |   (%a)[i2] = i2 + %0;
; CHECK-NEXT:           |   + END LOOP
; CHECK-NEXT:           |   
; CHECK-NEXT:           |   
; CHECK-NEXT:           |   + DO i2 = 0, 49, 1   <DO_LOOP>
; CHECK-NEXT:           |   |   %1 = (%a)[i2];
; CHECK-NEXT:           |   |   (%a)[i2] = i2 + %1;
; CHECK-NEXT:           |   + END LOOP
; CHECK-NEXT:           |   
; CHECK-NEXT:           |   
; CHECK-NEXT:           |   + DO i2 = 0, 49, 1   <DO_LOOP>
; CHECK-NEXT:           |   |   %2 = (%a)[i2];
; CHECK-NEXT:           |   |   (%a)[i2] = i2 + %2;
; CHECK-NEXT:           |   + END LOOP
; CHECK-NEXT:           + END LOOP
; CHECK-NEXT:        }
; CHECK-NEXT:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(i32 %n, ptr %a) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc31
  %i.04 = phi i32 [ 0, %entry ], [ %inc32, %for.inc31 ]
  %cmp1 = icmp eq i32 %n, 100
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  br label %for.body4

for.body4:                                        ; preds = %if.then, %for.inc
  %j.01 = phi i32 [ 0, %if.then ], [ %inc, %for.inc ]
  %idxprom = sext i32 %j.01 to i64
  %ptridx = getelementptr inbounds i32, ptr %a, i64 %idxprom
  %0 = load i32, ptr %ptridx, align 4
  %add = add nsw i32 %0, %j.01
  store i32 %add, ptr %ptridx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body4
  %inc = add nsw i32 %j.01, 1
  %cmp3 = icmp slt i32 %inc, 50
  br i1 %cmp3, label %for.body4, label %for.end

for.end:                                          ; preds = %for.inc
  br label %if.end

if.end:                                           ; preds = %for.end, %for.body
  %cmp5 = icmp eq i32 %n, 100
  br i1 %cmp5, label %if.then6, label %if.end17

if.then6:                                         ; preds = %if.end
  br label %for.body10

for.body10:                                       ; preds = %if.then6, %for.inc14
  %j7.02 = phi i32 [ 0, %if.then6 ], [ %inc15, %for.inc14 ]
  %idxprom11 = sext i32 %j7.02 to i64
  %ptridx12 = getelementptr inbounds i32, ptr %a, i64 %idxprom11
  %1 = load i32, ptr %ptridx12, align 4
  %add13 = add nsw i32 %1, %j7.02
  store i32 %add13, ptr %ptridx12, align 4
  br label %for.inc14

for.inc14:                                        ; preds = %for.body10
  %inc15 = add nsw i32 %j7.02, 1
  %cmp9 = icmp slt i32 %inc15, 50
  br i1 %cmp9, label %for.body10, label %for.end16

for.end16:                                        ; preds = %for.inc14
  br label %if.end17

if.end17:                                         ; preds = %for.end16, %if.end
  %cmp18 = icmp eq i32 %n, 100
  br i1 %cmp18, label %if.then19, label %if.end30

if.then19:                                        ; preds = %if.end17
  br label %for.body23

for.body23:                                       ; preds = %if.then19, %for.inc27
  %j20.03 = phi i32 [ 0, %if.then19 ], [ %inc28, %for.inc27 ]
  %idxprom24 = sext i32 %j20.03 to i64
  %ptridx25 = getelementptr inbounds i32, ptr %a, i64 %idxprom24
  %2 = load i32, ptr %ptridx25, align 4
  %add26 = add nsw i32 %2, %j20.03
  store i32 %add26, ptr %ptridx25, align 4
  br label %for.inc27

for.inc27:                                        ; preds = %for.body23
  %inc28 = add nsw i32 %j20.03, 1
  %cmp22 = icmp slt i32 %inc28, 50
  br i1 %cmp22, label %for.body23, label %for.end29

for.end29:                                        ; preds = %for.inc27
  br label %if.end30

if.end30:                                         ; preds = %for.end29, %if.end17
  br label %for.inc31

for.inc31:                                        ; preds = %if.end30
  %inc32 = add nsw i32 %i.04, 1
  %cmp = icmp slt i32 %inc32, 100
  br i1 %cmp, label %for.body, label %for.end33

for.end33:                                        ; preds = %for.inc31
  ret void
}

