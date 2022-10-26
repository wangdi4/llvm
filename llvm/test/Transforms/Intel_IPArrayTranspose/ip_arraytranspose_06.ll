; This test verifies that array transpose is not enabled because malloc
; sizes don't match.

; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: Failed: No Candidate mallocs found

define i32 @main() #0 {
b0:
  %p1 = tail call i8* @malloc(i64 214400000)
  %p2 = tail call i8* @malloc(i64 214400008)
  %pinc = getelementptr inbounds i8, i8* %p1, i64 3200000
  %bc0 = bitcast i8* %p1 to double*
  %pinc2 = getelementptr inbounds i8, i8* %p2, i64 3200000
  %bc2 = bitcast i8* %p2 to double*
  br label %b1

b1:                                                ; preds = %b2, %b0
  %ph1 = phi i64 [ 0, %b0 ], [ %inc, %b2 ]
  %inc1 = add nuw nsw i64 %ph1, 19
  %gep1 = getelementptr inbounds double, double* %bc0, i64 %inc1
  %bc1 = bitcast double* %gep1 to i32*
  %ld = load i32, i32* %bc1, align 4
  %gep3 = getelementptr inbounds double, double* %bc2, i64 %inc1
  %bc3 = bitcast double* %gep3 to i32*
  %ld3 = load i32, i32* %bc3, align 4
  %cmp0 = icmp eq i32 %ld, 0
  br i1 %cmp0, label %b2, label %b2

b2:                                               ; preds = %b1, %b1
  %inc = add nuw nsw i64 %ph1, 20
  %cmp = icmp ult i64 %ph1, 25999980
  br i1 %cmp, label %b1, label %b3

b3:                                               ; preds = %b2
  tail call void @free(i8* nonnull %p1)
  tail call void @free(i8* nonnull %p2)
  ret i32 0
}

declare dso_local noalias i8* @malloc(i64)
declare dso_local void @free(i8* nocapture)
attributes #0 = { norecurse }
