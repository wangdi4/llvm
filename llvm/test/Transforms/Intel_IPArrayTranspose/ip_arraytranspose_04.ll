; This test verifies that array transpose is not enabled because realloc
; call is not qualified as candidate.

; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;CHECK: Failed: No Candidate mallocs found

define i32 @main() #0 {
b0:
  %p1 = tail call i8* @malloc(i64 214400000)
  %p2 = tail call i8* @realloc(i8* %p1, i64 21440)
  %pinc = getelementptr inbounds i8, i8* %p1, i64 3200000
  %bc0 = bitcast i8* %pinc to double*
  br label %b1

b1:                                                ; preds = %b2, %b0
  %ph1 = phi i64 [ 0, %b0 ], [ %inc, %b2 ]
  %inc1 = add nuw nsw i64 %ph1, 19
  %gep1 = getelementptr inbounds double, double* %bc0, i64 %inc1
  %bc1 = bitcast double* %gep1 to i32*
  %ld = load i32, i32* %bc1, align 4
  %cmp0 = icmp eq i32 %ld, 0
  br i1 %cmp0, label %b2, label %b2

b2:                                               ; preds = %b1, %b1
  %inc = add nuw nsw i64 %ph1, 20
  %cmp = icmp ult i64 %ph1, 25999980
  br i1 %cmp, label %b1, label %b3

b3:                                               ; preds = %b2
  tail call void @free(i8* nonnull %p1)
  ret i32 0
}

declare dso_local noalias i8* @malloc(i64)

declare dso_local void @free(i8* nocapture)
declare dso_local i8* @realloc(i8* nocapture, i64)
attributes #0 = { norecurse }
