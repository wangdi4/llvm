; RUN: opt -passes="instcombine" -S < %s | FileCheck %s

; In the loop below, there are 2 IVs:
;  %t4.0 = phi ptr [ %arrayidx, %entry ], [ %arrayIdx47, %loop.20.loop.20_crit_edge ]
;  %i1.i64.0 = phi i64 [ 0, %entry ], [ %nextivloop.20, %loop.20.loop.20_crit_edge ]

; t4.0 is a derived IV of pointer type that has an incoming unrelated value,
; array %a.
; After the 1st iteration, t4.0 is changed to point to array %f.
; The basic IV of the loop is i1.i64.0, which is used in a GEP to index the
; array %f.
; BasicAA incorrectly returns "NoAlias" when queried between
; "t4.0" and "GEP %f %t2" (where %t2 = %i1.i64.0 * 2).
; The BasicAA phi analysis examines the 2 incoming values of the t4 phi.
; %arrayidx does not alias because it points to %a.
; %arrayIdx47 points to %f, but it does not alias because it is the previous
; iteration value of %t4. This cannot alias the current iteration value.
; BasicAA should not compare previous and current values using GEP distance
; analysis.

; CHECK: gepload = load i32, ptr %arrayIdx
; CHECK: store{{.*}}t4.0
; CHECK: gepload30 = load i32, ptr %arrayIdx

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [10 x i8] c"res = %u\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local ptr @foo(ptr %f) local_unnamed_addr  {
entry:
  %a = alloca [100 x i32], align 16
  %0 = bitcast ptr %f to ptr
  %arrayidx = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 0
  br label %loop.20

loop.20:                                          ; preds = %loop.20.loop.20_crit_edge, %entry
  %t4.0 = phi ptr [ %arrayidx, %entry ], [ %arrayIdx47, %loop.20.loop.20_crit_edge ]
  %i1.i64.0 = phi i64 [ 0, %entry ], [ %nextivloop.20, %loop.20.loop.20_crit_edge ]
  %t2 = mul i64 2, %i1.i64.0
  %arrayIdx = getelementptr inbounds [100 x i32], ptr %f, i64 0, i64 %t2
  %gepload = load i32, ptr %arrayIdx, align 4
  store i32 %gepload, ptr %t4.0, align 4
  %gepload30 = load i32, ptr %arrayIdx, align 4
  %arrayIdx27 = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 0
  store i32 %gepload30, ptr %arrayIdx27, align 4
  %t6 = add i64 %t2, 2
  %arrayIdx47 = getelementptr inbounds [100 x i32], ptr %f, i64 0, i64 %t6
  %nextivloop.20 = add nuw nsw i64 %i1.i64.0, 1
  %condloop.20 = icmp ne i64 %i1.i64.0, 34
  br i1 %condloop.20, label %loop.20.loop.20_crit_edge, label %afterloop.20

loop.20.loop.20_crit_edge:                        ; preds = %loop.20
  br label %loop.20

afterloop.20:                                     ; preds = %loop.20
  ret ptr %a
}
