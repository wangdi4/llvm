; CMPLRLLVM-23423: This test verifies that ArrayTranspose is triggered when
; zext is involved in unoptimized SCEV expression for memory references.
;
;SCEV before: ({(32216 + (160 * (zext i32 %lb0 to i64))<nuw><nsw>)<nuw><nsw>,+,160}<nuw><nsw><%b1> + %inn)<nsw>
;
;SCEV after: ({(592001608 + (8 * (zext i32 %lb0 to i64))<nuw><nsw>)<nuw><nsw>,+,8}<nw><%b1> + %inn)

; RUN: opt < %s -ip-array-transpose-heuristic=false -passes='module(iparraytranspose)' -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -ip-array-transpose-heuristic=false -passes='module(iparraytranspose)' -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -S 2>&1 | FileCheck --check-prefix=CHECK-OP %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check updated memory references are generated.
; CHECK: b00:
; CHECK: [[S0:%[0-9]+]] = shl nuw nsw i64 %lb, 3
; CHECK: [[A0:%[0-9]+]] = add nuw nsw i64 [[S0]], 592001608
; CHECK: b1:
; CHECK: [[I0:%[a-z0-9]+]] = phi i64 [ [[I1:%[a-z0-9.]+]], %b1 ], [ 0, %b00 ]
; CHECK: [[S1:%[0-9]+]] = shl i64 [[I0]], 3
; CHECK: [[A1:%[0-9]+]] = add i64 [[A0]], [[S1]]
; CHECK: [[G0:%[a-z0-9]+]]  = getelementptr i8, i8* %inn, i64 [[A1]]
; CHECK: [[B0:%[0-9]+]] = bitcast i8* [[G0]] to i64*
; CHECK: store i64 0, i64* [[B0]], align 8
; CHECK: [[I1]] = add i64 [[I0]], 1

; Check updated memory references are generated.
; CHECK-OP: b00:
; CHECK-OP: [[S0:%[0-9]+]] = shl nuw nsw i64 %lb, 3
; CHECK-OP: [[A0:%[0-9]+]] = add nuw nsw i64 [[S0]], 592001608
; CHECK-OP: b1:
; CHECK-OP: [[I0:%[a-z0-9]+]] = phi i64 [ [[I1:%[a-z0-9.]+]], %b1 ], [ 0, %b00 ]
; CHECK-OP: [[S1:%[0-9]+]] = shl i64 [[I0]], 3
; CHECK-OP: [[A1:%[0-9]+]] = add i64 [[A0]], [[S1]]
; CHECK-OP: [[G0:%[a-z0-9]+]]  = getelementptr i8, ptr %inn, i64 [[A1]]
; CHECK-OP: store i64 0, ptr [[G0]], align 8
; CHECK-OP: [[I1]] = add i64 [[I0]], 1

define i32 @main() #0 {
b0:
  %a00 = alloca i8*, align 4
  %p1 = tail call i8* @malloc(i64 1689600000)
  %pinc = getelementptr inbounds i8, i8* %p1, i64 12800000
  store i8* %pinc, i8** %a00
  tail call void @foo(i8** %a00)
  br label %b1

b1:                                               ; preds = %b0
  tail call void @free(i8* nonnull %p1)
  ret i32 0
}

define void @foo(i8** %in1) {
b0:
  %a0 = alloca i32, align 4
  %a1 = alloca i32, align 4
  %lb0 = load i32, i32* %a0, align 4
  %ub0 = load i32, i32* %a1, align 4
  %ic1 = icmp ugt i32 %lb0, %ub0
  br i1 %ic1, label %b2, label %b00

b00:
  %lb = zext i32 %lb0 to i64
  %ub1 = add i32 %ub0, 1
  %ub = zext i32 %ub1 to i64
  br label %b1

b1:                                                ; preds = %b1, %b00
  %ph1 = phi i64 [ %lb, %b00 ], [ %inc, %b1 ]
  %m1 = mul nsw i64 %ph1, 20
  %add1 = add nuw nsw i64 %m1, 4027
  %inn = load i8*, i8** %in1, align 4
  %bc1 = bitcast i8* %inn to double*
  %gep2 = getelementptr inbounds double, double* %bc1, i64 %add1
  %bc2 = bitcast double* %gep2 to i64*
  store i64 0, i64* %bc2, align 8
  %inc = add nuw nsw i64 %ph1, 1
  %ic2 = icmp eq i64 %inc, %ub
  br i1 %ic2, label %b2, label %b1

b2:
  ret void
}

declare noalias i8* @malloc(i64) #1
declare void @free(i8* nocapture) #2
attributes #0 = { norecurse }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free") "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
