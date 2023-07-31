;
; Test to check that CM detects trunc i64 to../add/zext ... to i64 pattern and
; assigne cost of the 'and' instrcution for trunc and 0 for zext.
;
; RUN: opt -disable-output -passes=vplan-vec -vplan-cost-model-print-analysis-for-vf=4  %s | FileCheck %s
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @main(ptr %a, i12 %k) local_unnamed_addr {
;
; CHECK:      Cost 2 for i12 [[VP_TRUNC:%.*]] = trunc i64 [[VP_INDVARS_IV:%.*]] to i12
; CHECK-NEXT: Cost 1 for i12 [[VP_ADD12:%.*]] = add i12 [[VP_TRUNC]] i12 [[K0:%.*]]
; CHECK-NEXT: Cost 0 for i64 [[VP_ZEXT:%.*]] = zext i12 [[VP_ADD12]] to i64
;
entry:
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.1
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %trunc = trunc i64 %indvars.iv to i12
  %add12 = add i12 %trunc, %k
  %zext = zext i12 %add12 to i64
  %gep = getelementptr i32, ptr %a, i64 %zext
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %omp.loop.exit
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
