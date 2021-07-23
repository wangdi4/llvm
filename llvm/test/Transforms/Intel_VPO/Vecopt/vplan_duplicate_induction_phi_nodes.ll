; Check if VPLoopEntities framework is able to handle duplicates of induction PHIs.

; RUN: opt -loopopt=0 -vplan-vec -vpo-vplan-build-stress-test -vplan-print-after-plain-cfg -vplan-entities-dump -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -loopopt=0 -passes="vplan-vec" -vpo-vplan-build-stress-test -vplan-print-after-plain-cfg -vplan-entities-dump -disable-output < %s 2>&1 | FileCheck %s

; Note : We should potentially stop supporting duplicate induction PHIs right
; from legality. Check CMPLRLLVM-18412.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Case 1 - Duplicate PHI is completely identical to original. So duplicate PHI should not be imported and all its uses
; within loop is replaced with original.

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* nocapture %a, i32* nocapture %b, i32* nocapture readonly %c, i32 %N) local_unnamed_addr {
; CHECK-LABEL:  VPlan after importing plain CFG:
; CHECK:        Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 2147483647 BinOp: i64 [[VP_INDVARS_IV_NEXT:%.*]] = add i64 [[VP_INDVARS_IV:%.*]] i64 1 need close form
; CHECK-NEXT:    Linked values: i64 [[VP_INDVARS_IV]], i64 [[VP_INDVARS_IV_NEXT]],
; CHECK:          i64 [[VP_INDVARS_IV]] = phi  [ i64 0, [[BB1:.*]] ],  [ i64 [[VP_INDVARS_IV_NEXT]], [[BB0:.*]] ]
; CHECK-NEXT:     i64 [[VP_INDVARS_IV_2:%.*]] = phi  [ i64 0, [[BB1]] ],  [ i64 [[VP_INDVARS_IV_NEXT]], [[BB0]] ]
; Use was replaced from %indvars.iv.2 to just %indvars.iv
; CHECK-NEXT:     i64 [[VP_REM2026:%.*]] = and i64 [[VP_INDVARS_IV]] i64 1
;
entry:
  %cmp = icmp sgt i32 %N, 0
  br i1 %cmp, label %DIR.OMP.SIMD.121, label %omp.precond.end

DIR.OMP.SIMD.121:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:
  %1 = add i32 %N, -1
  %wide.trip.count = sext i32 %N to i64
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %indvars.iv.2 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %rem2026 = and i64 %indvars.iv.2, 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:
  ret void
}

; Case 2 - Duplicate PHI is partially identical to original. So it should not be imported as an entity, and left behind
; for regular vectorization.

define dso_local void @foo1(i32* nocapture %a, i32* nocapture %b, i32* nocapture readonly %c, i32 %N) local_unnamed_addr {
; CHECK-LABEL:  VPlan after importing plain CFG:
; CHECK:        Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 1 Step: i64 1 StartVal: i64 1 EndVal: ? BinOp: i64 [[VP_INDVARS_IV_NEXT:%.*]] = add i64 [[VP_INDVARS_IV:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP_INDVARS_IV]], i64 [[VP_INDVARS_IV_NEXT]],
; CHECK:          i64 [[VP_INDVARS_IV]] = phi  [ i64 1, [[BB1:.*]] ],  [ i64 [[VP_INDVARS_IV_NEXT]], [[BB0:.*]] ]
; CHECK-NEXT:     i64 [[VP_INDVARS_IV_2:%.*]] = phi  [ i64 0, [[BB1]] ],  [ i64 [[VP_INDVARS_IV]], [[BB0]] ]
; No replacement happened
; CHECK-NEXT:     i64 [[VP_REM2026:%.*]] = and i64 [[VP_INDVARS_IV_2]] i64 1
;
entry:
  %cmp = icmp sgt i32 %N, 0
  br i1 %cmp, label %DIR.OMP.SIMD.121, label %omp.precond.end

DIR.OMP.SIMD.121:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:
  %1 = add i32 %N, -1
  %wide.trip.count = sext i32 %N to i64
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 1, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %indvars.iv.2 = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv, %omp.inner.for.body ]
  %rem2026 = and i64 %indvars.iv.2, 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
