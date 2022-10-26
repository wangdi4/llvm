; This test verifies that the transformations are done correctly for memory
; references and pointer increments are updated.
;
; SCEV before: {{{(320152 + %p1),+,1600000}<nw><%b1>,+,16000}<nw><%b2>,+,160}<nw><%b3>
; SCEV after: {{{(203688000 + %p1),+,80000}<nw><%b1>,+,800}<nw><%b2>,+,8}<nw><%b3>

; RUN: opt < %s -ip-array-transpose-heuristic=false -passes='module(iparraytranspose)' -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -ip-array-transpose-heuristic=false -passes='module(iparraytranspose)' -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck --check-prefix=CHECK-OP %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Make sure pointer increment is cleared.
; CHECK: %pinc = getelementptr inbounds i8, i8* %p1, i64 0
; CHECK-NOT: %pinc = getelementptr inbounds i8, i8* %p1, i64 320000

; Check updated memory references are generated.
; CHECK:  [[M0:%[0-9]+]]  = mul nuw nsw i64 %ph2, 40000
; CHECK:  [[A0:%[0-9]+]] = add nuw i64 [[M0]], 203688000
; CHECK:  [[M1:%[0-9]+]] = mul nuw nsw i64 %ph0, 400
; CHECK:  [[A1:%[0-9]+]] = add i64 [[A0]], [[M1]]
; CHECK:  [[S0:%[0-9]+]] = shl nuw nsw i64 %ph1, 2
; CHECK:  [[A2:%[0-9]+]]  = add i64 [[A1]], [[S0]]
; CHECK:  [[G0:%[a-z0-9]+]] = getelementptr i8, i8* %p1, i64 [[A2]]
; CHECK:  [[B0:%[0-9]+]] = bitcast i8* [[G0]] to i32*
; CHECK:  %ld = load i32, i32* [[B0]], align 8
; CHECK:  [[B1:%[0-9]+]] = bitcast i8* %scevgep to i32*
; CHECK:  store i32 %or1, i32* [[B1]], align 8

; Make sure pointer increment is cleared.
; CHECK-OP: %pinc = getelementptr inbounds i8, ptr %p1, i64 0
; CHECK-OP-NOT: %pinc = getelementptr inbounds i8, ptr %p1, i64 320000

; Check updated memory references are generated.
; CHECK-OP:  [[M0:%[0-9]+]]  = mul nuw nsw i64 %ph2, 40000
; CHECK-OP:  [[A0:%[0-9]+]] = add nuw i64 [[M0]], 203688000
; CHECK-OP:  [[M1:%[0-9]+]] = mul nuw nsw i64 %ph0, 400
; CHECK-OP:  [[A1:%[0-9]+]] = add i64 [[A0]], [[M1]]
; CHECK-OP:  [[S0:%[0-9]+]] = shl nuw nsw i64 %ph1, 2
; CHECK-OP:  [[A2:%[0-9]+]]  = add i64 [[A1]], [[S0]]
; CHECK-OP:  [[G0:%[a-z0-9]+]] = getelementptr i8, ptr %p1, i64 [[A2]]
; CHECK-OP:  %ld = load i32, ptr [[G0]], align 8
; CHECK-OP:  store i32 %or1, ptr [[G0]], align 8

define i32 @main() #0 {
b0:
  %p1 = tail call i8* @malloc(i64 214400000)
  %pinc = getelementptr inbounds i8, i8* %p1, i64 320000
  %bc0 = bitcast i8* %pinc to double*
  br label %b1

b1:                                               ; preds = %b7, %b0
  %ph2 = phi i64 [ 0, %b0 ], [ %inc3, %b7 ]
  %mul4 = mul nuw nsw i64 %ph2, 10000
  br label %b2

b2:                                                ; preds = %b6, %b1
  %ph0 = phi i64 [ 0, %b1 ], [ %inc2, %b6 ]
  %mul5 = mul nuw nsw i64 %ph0, 100
  %add4 = add nuw nsw i64 %mul5, %mul4
  br label %b3

b3:                                               ; preds = %b5, %b2
  %ph1 = phi i64 [ 0, %b2 ], [ %inc1, %b5 ]
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
