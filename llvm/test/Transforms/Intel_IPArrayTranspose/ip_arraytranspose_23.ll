; This test verifies that the transformations are done correctly for memory
; references and pointer increments are updated.
;
; SCEV before:  {(152 + %in1)<nsw>,+,160}<nsw><%b1>
;
; SCEV after: {(203688000 + %in1),+,8}<nw><%b1>
;
; Stride: 160
; Max Elem Size: 4
; MallocSize: 214400000
; TransposedNumRows: 1340000  TransposedNumCols: 40
;
; Original constant: 320000 + 152
;
; Transposed constant: ((320154 / 4) / 40 + ((320154 / 4) % 4) * 1340000 ) * 4


; RUN: opt < %s -ip-array-transpose-heuristic=false -iparraytranspose -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -ip-array-transpose-heuristic=false -passes='module(iparraytranspose)' -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK:  define i32 @main()
; CHECK:  %pinc = getelementptr inbounds i8, i8* %p1, i64 0
; CHECK-NOT:  %pinc = getelementptr inbounds i8, i8* %p1, i64 320000

; CHECK:  define void @foo(i8* %in1)
; CHECK:  b1:
; CHECK:  [[I0:%[a-z0-9.]+]] = phi i64 [ [[I1:%[a-z0-9.]+]], %b2 ], [ 0, %b0 ]
; CHECK:  [[S0:%[0-9]+]] = shl nuw nsw i64 [[I0]], 2
; CHECK:  [[A0:%[0-9]+]] = add i64 [[S0]], 203688000
; CHECK:  [[G0:%[a-z0-9]+]] = getelementptr i8, i8* %in1, i64 [[A0]]
; CHECK:  [[B0:%[0-9]+]] = bitcast i8* [[G0]] to i32*
; CHECK:  %ld = load i32, i32* [[B0]], align 4
; CHECK:  b2:
; CHECK:  [[I1]] = add i64 [[I0]], 1


define i32 @main() #0 {
b0:
  %p1 = tail call i8* @malloc(i64 214400000)
  %pinc = getelementptr inbounds i8, i8* %p1, i64 320000
  tail call void @foo(i8* %pinc)
  br label %b1

b1:                                               ; preds = %b0
  tail call void @free(i8* nonnull %p1)
  ret i32 0
}

define void @foo(i8* %in1) {
b0:
  %bc0 = bitcast i8* %in1 to double*
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

b3:
  ret void
}

declare noalias i8* @malloc(i64)
declare void @free(i8* nocapture)
attributes #0 = { norecurse }
