; This test verifies that array transpose is not enabled because %pinc,
; which is single indexed GEP, is in loop.

; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:    Inst is in unexpected routine or loop
; CHECK:    Failed: Unable to collect MemRefs


; Function Attrs: norecurse
define i32 @main() #0 {
b0:
  %p1 = tail call i8* @malloc(i64 214400000)
  %bc0 = bitcast i8* %p1 to double*
  br label %b1

b1:                                                ; preds = %b2, %b0
  %ph1 = phi i64 [ 0, %b0 ], [ %inc, %b2 ]
  %pinc = getelementptr inbounds i8, i8* %p1, i64 3200000
  %inc1 = add nuw nsw i64 %ph1, 19
  %bc2 = bitcast i8* %pinc to double*
  %gep1 = getelementptr inbounds double, double* %bc2, i64 %inc1
  %bc1 = bitcast double* %gep1 to i32*
  %ld = load i32, i32* %bc1, align 4
  %gep2 = getelementptr inbounds i8, i8* %pinc, i64 8
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

declare dso_local noalias i8* @malloc(i64) #1

declare dso_local void @free(i8* nocapture) #2

attributes #0 = { norecurse }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free") "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
