; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Check delinearize conditions for 2d case with respect to lower bound (-2).

; BEGIN REGION { }
;       + DO i1 = 0, %UB1, 1   <DO_LOOP>
;       |   + DO i2 = 0, %UB2, 1   <DO_LOOP>
;       |   |   + DO i3 = 0, %UB3, 1   <DO_LOOP>
;       |   |   |   (%p)[(%d2 * %d1) * i1 + %d2 * i2 + i3] = (%q)[i3];
;       |   |   |   (%p)[(%d2 * %d1) * i1 + %d2 * i2 + i3 + %d2 - 2] = (%q)[i3];
;       |   |   + END LOOP
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK: %mv.test = &((%q)[%UB3]) >=u &((%p)[0][0][0]);
; CHECK: %mv.test3 = &((%p)[%UB1][%UB2 + 1][%UB3 + -2]) >=u &((%q)[0]);
; CHECK: %mv.and = %mv.test  &  %mv.test3;

; For each inner dimension check that the index span (Maximum index - Minimum index)
; is less then the next higher dimension size. In this case-
;
;      d1     d2
; (%p)[i1][i2    ][i3    ]
; (%p)[i1][i2 + 1][i3 - 2]
;
; Maximum indices for inner dimensions:
; [][UB2 + 1][UB3]
;
; Minimum indices for inner dimensions:
; [][0][-2]

; CHECK: if
; CHECK-DAG: %d2 > 1
; CHECK-DAG: %UB3 + 2 < %d2
; CHECK-DAG: %d1 > 1
; CHECK-DAG: %UB2 + 1 < %d1
; CHECK-DAG: %mv.and == 0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr nocapture %p, ptr nocapture readonly %q, i64 %d2, i64 %d1, i64 %UB1, i64 %UB2, i64 %UB3) {
entry:
  %mul = mul i64 %d1, %d2
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %entry
  %k.049 = phi i64 [ 0, %entry ], [ %inc25, %for.cond.cleanup3 ]
  %mul9 = mul i64 %mul, %k.049
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3
  ret void

for.cond5.preheader:                              ; preds = %for.cond.cleanup7, %for.cond1.preheader
  %j.048 = phi i64 [ 0, %for.cond1.preheader ], [ %add16, %for.cond.cleanup7 ]
  %mul10 = mul i64 %j.048, %d2
  %add16 = add nuw nsw i64 %j.048, 1
  %mul17_ = mul i64 %add16, %d2
  %mul17 = add i64 %mul17_, -2
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %inc25 = add nuw nsw i64 %k.049, 1
  %exitcond53 = icmp ugt i64 %inc25, %UB1
  br i1 %exitcond53, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.body8
  %exitcond52 = icmp ugt i64 %add16, %UB2
  br i1 %exitcond52, label %for.cond.cleanup3, label %for.cond5.preheader

for.body8:                                        ; preds = %for.body8, %for.cond5.preheader
  %i.047 = phi i64 [ 0, %for.cond5.preheader ], [ %inc, %for.body8 ]
  %arrayidx = getelementptr inbounds i32, ptr %q, i64 %i.047
  %0 = load i32, ptr %arrayidx, align 4
  %add = add i64 %i.047, %mul9
  %add11 = add i64 %add, %mul10
  %arrayidx12 = getelementptr inbounds i32, ptr %p, i64 %add11
  store i32 %0, ptr %arrayidx12, align 4
  %1 = load i32, ptr %arrayidx, align 4
  %add19 = add i64 %add, %mul17
  %arrayidx20 = getelementptr inbounds i32, ptr %p, i64 %add19
  store i32 %1, ptr %arrayidx20, align 4
  %inc = add nuw nsw i64 %i.047, 1
  %exitcond = icmp ugt i64 %inc, %UB3
  br i1 %exitcond, label %for.cond.cleanup7, label %for.body8
}

