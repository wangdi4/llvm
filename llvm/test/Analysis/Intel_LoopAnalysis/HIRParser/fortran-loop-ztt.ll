; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-details 2>&1 | FileCheck %s

; Verify that ztt of inner loop is recognized for a function marked with fortran language attribute.

; CHECK: DO i64 i1 = 0, sext.i32.i64(%N1) + -1, 1   <DO_LOOP>
; CHECK: Ztt: if (%N2 >= 1)
; CHECK: DO i64 i2 = 0, sext.i32.i64(%N2) + -1, 1   <DO_LOOP>


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @sub_(ptr %A, i32 %N1, i32 %N2) "intel-lang"="fortran" {
alloca_0:
  %rel = icmp slt i32 %N1, 1
  br i1 %rel, label %bb1, label %bb4.preheader

bb4.preheader:                                    ; preds = %alloca_0
  %rel2 = icmp slt i32 %N2, 1
  %0 = add nuw nsw i32 %N2, 1
  %1 = add nuw nsw i32 %N1, 1
  %wide.trip.count55 = sext i32 %1 to i64
  %wide.trip.count = sext i32 %0 to i64
  br label %bb4

bb4:                                              ; preds = %bb4.preheader, %bb9
  %indvars.iv53 = phi i64 [ 1, %bb4.preheader ], [ %indvars.iv.next54, %bb9 ]
  br i1 %rel2, label %bb9, label %bb8.preheader

bb8.preheader:                                    ; preds = %bb4
  br label %bb8

bb8:                                              ; preds = %bb8.preheader, %bb8
  %indvars.iv = phi i64 [ 1, %bb8.preheader ], [ %indvars.iv.next, %bb8 ]
  store i32 5, ptr %A, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb9.loopexit, label %bb8

bb9.loopexit:                                     ; preds = %bb8
  br label %bb9

bb9:                                              ; preds = %bb9.loopexit, %bb4
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond56 = icmp eq i64 %indvars.iv.next54, %wide.trip.count55
  br i1 %exitcond56, label %bb1.loopexit, label %bb4

bb1.loopexit:                                     ; preds = %bb9
  br label %bb1

bb1:                                              ; preds = %bb1.loopexit, %alloca_0
  ret void
}

