; REQUIRES: asserts
; RUN: opt -passes "function(print<array-use>)" -o /dev/null < %s 2>&1 | FileCheck %s
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that the array use analysis is capable of recognizing that not all the
; values of the array will be subsequently used.

; CHECK: Classifying array use for: @do_something
; CHECK:   store i64 %indvars.iv54, [[PTR:.*]] %arrayidx, align 4
; CHECK:   -->  array at   %array = alloca [10000 x i64], align 16 (size 10000) [0, 9999]
; CHECK:        only using [0, 4999]
; CHECK:   %val = load i64, [[PTR]] %arrayidx5, align 4
; CHECK:   -->  array at   %array = alloca [10000 x i64], align 16 (size 10000) [0, 4999]
; CHECK:        only using [0, 4999]


define dso_local void @do_something() local_unnamed_addr {
entry:
  %array = alloca [10000 x i64], align 16
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv54 = phi i64 [ 0, %entry ], [ %indvars.iv.next55, %for.body ]
  %arrayidx = getelementptr inbounds [10000 x i64], [10000 x i64]* %array, i64 0, i64 %indvars.iv54
  store i64 %indvars.iv54, i64* %arrayidx, align 4
  %indvars.iv.next55 = add nuw nsw i64 %indvars.iv54, 1
  %exitcond56 = icmp eq i64 %indvars.iv.next55, 10000
  br i1 %exitcond56, label %barrier, label %for.body

barrier:
  call void @optbarrier()
  br label %for.body3

for.body3:                                        ; preds = %barrier, %for.body3
  %indvars.iv51 = phi i64 [ %indvars.iv.next52, %for.body3 ], [ 0, %barrier ]
  %arrayidx5 = getelementptr inbounds [10000 x i64], [10000 x i64]* %array, i64 0, i64 %indvars.iv51
  %val = load i64, i64* %arrayidx5, align 4
  call void @use_value(i64 %val)
  %indvars.iv.next52 = add nuw nsw i64 %indvars.iv51, 1
  %exitcond53 = icmp eq i64 %indvars.iv.next52, 5000
  br i1 %exitcond53, label %for.end28, label %for.body3

for.end28:                                        ; preds = %for.body23
  ret void
}

declare void @optbarrier()
declare void @use_value(i64)
