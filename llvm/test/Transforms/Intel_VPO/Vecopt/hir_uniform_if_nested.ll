; LIT test to check generation of uniform nested ifs
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
;
; Incoming HIR looks like the following:
;
;    DO i1 = 0, 99, 1   <DO_LOOP>
;      if (%n1 != 0)
;      {
;         (%lp1)[i1] = i1;
;         if (%n2 == 0)
;         {
;            (%lp4)[i1] = i1;
;         }
;         else
;         {
;            (%lp2)[i1] = i1;
;            if (%n3 != 0)
;            {
;               (%lp3)[i1] = i1;
;            }
;         }
;      }
;    END LOOP
;
; CHECK:      + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT: |   if (%n1 != 0)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      (<4 x i64>*)(%lp1)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT: |      if (%n2 == 0)
; CHECK-NEXT: |      {
; CHECK-NEXT: |         (<4 x i64>*)(%lp4)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT: |      }
; CHECK-NEXT: |      else
; CHECK-NEXT: |      {
; CHECK-NEXT: |         (<4 x i64>*)(%lp2)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT: |         if (%n3 != 0)
; CHECK-NEXT: |         {
; CHECK-NEXT: |            (<4 x i64>*)(%lp3)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT: |         }
; CHECK-NEXT: |      }
; CHECK-NEXT: |   }
; CHECK-NEXT: + END LOOP
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind uwtable writeonly
define dso_local void @foo(i64* noalias nocapture %lp1, i64* noalias nocapture %lp2, i64* noalias nocapture %lp3, i64* noalias nocapture %lp4, i64 %n1, i64 %n2, i64 %n3, i64 %n4) local_unnamed_addr #0 {
entry:
  %tobool.not = icmp eq i64 %n1, 0
  %tobool1.not = icmp eq i64 %n2, 0
  %tobool4.not = icmp eq i64 %n3, 0
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %l1.020 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i64, i64* %lp1, i64 %l1.020
  store i64 %l1.020, i64* %arrayidx, align 8
  br i1 %tobool1.not, label %if.else, label %if.then2

if.then2:                                         ; preds = %if.then
  %arrayidx3 = getelementptr inbounds i64, i64* %lp2, i64 %l1.020
  store i64 %l1.020, i64* %arrayidx3, align 8
  br i1 %tobool4.not, label %for.inc, label %if.then5

if.then5:                                         ; preds = %if.then2
  %arrayidx6 = getelementptr inbounds i64, i64* %lp3, i64 %l1.020
  store i64 %l1.020, i64* %arrayidx6, align 8
  br label %for.inc

if.else:                                          ; preds = %if.then
  %arrayidx7 = getelementptr inbounds i64, i64* %lp4, i64 %l1.020
  store i64 %l1.020, i64* %arrayidx7, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then2, %if.then5, %if.else
  %inc = add nuw nsw i64 %l1.020, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}
