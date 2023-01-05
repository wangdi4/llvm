; This test verifies that the transformations are done correctly for memory
; references when sext is involved.
;
;SCEV before: {(-12800000 + (160 * (sext i32 %lb to i64))<nsw> + %in1),+,160}<nsw><%b1>
;
;SCEV after: {((8 * (sext i32 %lb to i64))<nsw> + %in1),+,8}<nw><%b1>

; RUN: opt < %s -ip-array-transpose-heuristic=false -passes='module(iparraytranspose)' -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -ip-array-transpose-heuristic=false -passes='module(iparraytranspose)' -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck --check-prefix=CHECK-OP %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check updated memory references are generated.
; CHECK: b00:
; CHECK:  [[S0:%[0-9]+]] = shl nsw i64 %s0, 3
; CHECK: b1:
; CHECK: [[I0:%[a-z0-9]+]] = phi i64 [ [[I1:%[a-z0-9.]+]], %b1 ], [ 0, %b00 ]
; CHECK: [[S1:%[0-9]+]] = shl i64 [[I0]], 3
; CHECK: [[A0:%[0-9]+]] = add i64 [[S0]], [[S1]]
; CHECK: [[G0:%[a-z0-9]+]] = getelementptr i8, i8* %in1, i64 [[A0]]
; CHECK: [[B0:%[0-9]+]] = bitcast i8* [[G0]] to double*
; CHECK: %ld = load double, double* [[B0]], align 8
; CHECK: [[I1]] = add i64 [[I0]], 1


; Check updated memory references are generated.
; CHECK-OP: [[B0:%bc[0-9]+]] = bitcast ptr %in1 to ptr
; CHECK-OP: b00:
; CHECK-OP:  [[S0:%[0-9]+]] = shl nsw i64 %s0, 3
; CHECK-OP: b1:
; CHECK-OP: [[I0:%[a-z0-9]+]] = phi i64 [ [[I1:%[a-z0-9.]+]], %b1 ], [ 0, %b00 ]
; CHECK-OP: [[S1:%[0-9]+]] = shl i64 [[I0]], 3
; CHECK-OP: [[A0:%[0-9]+]] = add i64 [[S0]], [[S1]]
; CHECK-OP: [[G0:%[a-z0-9]+]] = getelementptr i8, ptr [[B0]], i64 [[A0]]
; CHECK-OP: %ld = load double, ptr [[G0]], align 8
; CHECK-OP: [[I1]] = add i64 [[I0]], 1

define i32 @main() #0 {
b0:
  %p1 = tail call i8* @malloc(i64 1689600000)
  %pinc = getelementptr inbounds i8, i8* %p1, i64 12800000
  tail call void @foo(i8* %pinc)
  br label %b1

b1:                                               ; preds = %b0
  tail call void @free(i8* nonnull %p1)
  ret i32 0
}

define void @foo(i8* %in1) {
b0:
  %a0 = alloca i32, align 4
  %a1 = alloca i32, align 4
  %bc0 = bitcast i8* %in1 to double*
  %lb = load i32, i32* %a0, align 4
  %ub = load i32, i32* %a1, align 4
  %ic = icmp sgt i32 %lb, %ub
  br i1 %ic, label %b3, label %b00

b00:
  %s0 = sext i32 %lb to i64
  %ub1 = add i32 %ub, 1
  br label %b1

b1:                                                ; preds = %b1, %b00
  %ph1 = phi i64 [ %s0, %b00 ], [ %inc, %b1 ]
  %m1 = mul nsw i64 %ph1, 20
  %inc1 = add nuw nsw i64 %m1, -1600000
  %gep1 = getelementptr inbounds double, double* %bc0, i64 %inc1
  %ld = load double, double* %gep1, align 8
  %inc = add nuw nsw i64 %ph1, 1
  %tr = trunc i64 %inc to i32
  %cmp = icmp eq i32 %tr, %ub1
  br i1 %cmp, label %b1, label %b3

b3:
  ret void
}

declare noalias i8* @malloc(i64) #1
declare void @free(i8* nocapture) #2
attributes #0 = { norecurse }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free") "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
