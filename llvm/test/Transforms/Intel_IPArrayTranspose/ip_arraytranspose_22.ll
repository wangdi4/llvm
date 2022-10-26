; This test verifies that there are no legality issues to enable array
; transpose. Stride for the memory references is 160. SCEV for the memory
; reference index looks like:
;   {{{(320152 + (160 * %lb3) + (16000 * %lb2) + (1600000 * %lb1)),+,1600000}
;    <%b1>,+,16000}<%b2>,+,160}<%b3>

; REQUIRES: asserts
; RUN: opt < %s -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -disable-output -passes='module(iparraytranspose)' -whole-program-assume -debug-only=iparraytranspose -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:    Passed: All validations

define i32 @main(i64 %lb1, i64 %lb2, i64 %lb3) #0 {
b0:
  %p1 = tail call i8* @malloc(i64 214400000)
  %pinc = getelementptr inbounds i8, i8* %p1, i64 320000
  %bc0 = bitcast i8* %pinc to double*
  br label %b1

b1:                                               ; preds = %b7, %b0
  %ph2 = phi i64 [ %lb1, %b0 ], [ %inc3, %b7 ]
  %mul4 = mul nuw nsw i64 %ph2, 10000
  br label %b2

b2:                                                ; preds = %b6, %b1
  %ph0 = phi i64 [ %lb2, %b1 ], [ %inc2, %b6 ]
  %mul5 = mul nuw nsw i64 %ph0, 100
  %add4 = add nuw nsw i64 %mul5, %mul4
  br label %b3

b3:                                               ; preds = %b5, %b2
  %ph1 = phi i64 [ %lb3, %b2 ], [ %inc1, %b5 ]
  br  label %b4

b4:                                               ; preds = %b3
  %inc0 = add nuw nsw i64 %add4, %ph1
  %mul = mul nuw nsw i64 %inc0, 20
  %add0 = add nuw nsw i64 %mul, 19
  %gep1 = getelementptr inbounds double, double* %bc0, i64 %add0
  %bc1 = bitcast double* %gep1 to i32*
  %ld = load i32, i32* %bc1, align 8
  %or1 = or i32 %ld, 1
  store i32 %or1, i32* %bc1, align 8
  br label %b5

b5:                                               ; preds = %b4, %b3
  %inc1 = add nuw nsw i64 %ph1, 1
  %cmp1 = icmp eq i64 %inc1, 100
  br i1 %cmp1, label %b6, label %b3

b6:                                               ; preds = %b5
  %inc2 = add nuw nsw i64 %ph0, 1
  %cmp2 = icmp eq i64 %inc2, 100
  br i1 %cmp2, label %b7, label %b2

b7:                                               ; preds = %b6
  %inc3 = add nuw nsw i64 %ph2, 1
  %cmp3 = icmp eq i64 %inc3, 130
  br i1 %cmp3, label %b8, label %b1

b8:
  tail call void @free(i8* nonnull %p1)
  ret i32 0
}

declare noalias i8* @malloc(i64) #1
declare void @free(i8* nocapture) #2
attributes #0 = { norecurse }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free") "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
