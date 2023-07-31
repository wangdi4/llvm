; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-prefetching" -hir-prefetching-loads-only -print-before=hir-prefetching -print-after=hir-prefetching 2>&1 < %s | FileCheck %s

; Verify that with load only prefetching, we only prefetch for loads A[i+500] 
; and C[B[i]] and ignore stores A[i1] and C[M[i]].

; Source code
;#pragma  prefetch A
;#pragma  prefetch C:2:30
;  for (i=0; i< N; i++) {
;    A[i1] = 0;
;    C[M[i]] = A[i+500] + C[B[i]];
;  }


; CHECK: Dump Before

; CHECK:      + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 9500>  <LEGAL_MAX_TC = 2147483647>
; CHECK-NEXT: |   (@A)[0][i1] = 0;
; CHECK-NEXT: |   %1 = (@A)[0][i1 + 500];
; CHECK-NEXT: |   %2 = (@B)[0][i1];
; CHECK-NEXT: |   %ld = (@C)[0][%2];
; CHECK-NEXT: |   %3 = (%M)[i1];
; CHECK-NEXT: |   (@C)[0][%3] = %1 + %ld;
; CHECK-NEXT: + END LOOP


; CHECK: Dump After

; CHECK:      + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 9500>  <LEGAL_MAX_TC = 2147483647>
; CHECK-NEXT: |   (@A)[0][i1] = 0;
; CHECK-NEXT: |   %1 = (@A)[0][i1 + 500];
; CHECK-NEXT: |   %2 = (@B)[0][i1];
; CHECK-NEXT: |   %ld = (@C)[0][%2];
; CHECK-NEXT: |   %3 = (%M)[i1];
; CHECK-NEXT: |   (@C)[0][%3] = %1 + %ld;
; CHECK-NEXT: |   if (i1 + 30 <=u zext.i32.i64(%N) + -1)
; CHECK-NEXT: |   {
; CHECK-NEXT: |      %Load = (@B)[0][i1 + 30];
; CHECK-NEXT: |      @llvm.prefetch.p0(&((i8*)(@C)[0][%Load]),  0,  1,  1);
; CHECK-NEXT: |   }
; CHECK-NEXT: |   @llvm.prefetch.p0(&((i8*)(@A)[0][i1 + 521]),  0,  3,  1);
; CHECK-NEXT: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local global [10000 x i32] zeroinitializer, align 16
@B = dso_local global [10000 x i32] zeroinitializer, align 16
@C = dso_local global [10000 x i32] zeroinitializer, align 16

define dso_local noalias ptr @sub(ptr nocapture readonly %M, i32 %N) {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.PREFETCH_LOOP"(), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @A), "QUAL.PRAGMA.HINT"(i32 -1), "QUAL.PRAGMA.DISTANCE"(i32 -1), "QUAL.PRAGMA.ENABLE"(i32 1), "QUAL.PRAGMA.VAR"(ptr @C), "QUAL.PRAGMA.HINT"(i32 2), "QUAL.PRAGMA.DISTANCE"(i32 30) ]
  %cmp11 = icmp sgt i32 %N, 0
  br i1 %cmp11, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  %wide.trip.count13 = zext i32 %N to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %iv.add = add nuw nsw i64 %indvars.iv, 500
  %arrayidx = getelementptr inbounds [10000 x i32], ptr @A, i64 0, i64 %indvars.iv
  store i32 0, ptr %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds [10000 x i32], ptr @A, i64 0, i64 %iv.add
  %1 = load i32, ptr %arrayidx1, align 4
  %arrayidx2 = getelementptr inbounds [10000 x i32], ptr @B, i64 0, i64 %indvars.iv
  %2 = load i32, ptr %arrayidx2, align 4
  %sext = sext i32 %2 to i64
  %arrayidx3 = getelementptr inbounds [10000 x i32], ptr @C, i64 0, i64 %sext
  %ld = load i32, ptr %arrayidx3, align 4
  %add = add nsw i32 %1, %ld
  %ptridx = getelementptr inbounds i32, ptr %M, i64 %indvars.iv
  %3 = load i32, ptr %ptridx, align 4
  %idxprom4 = sext i32 %3 to i64
  %arrayidx5 = getelementptr inbounds [10000 x i32], ptr @C, i64 0, i64 %idxprom4
  store i32 %add, ptr %arrayidx5, align 4
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count13
  br i1 %exitcond.not, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.PREFETCH_LOOP"() ]
  ret ptr undef
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

