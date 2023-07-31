; This test verifies that private-variables escaping out through a write
; to an output argument are unsafe for data-layout transformation.

; RUN: opt -disable-output -passes=vplan-vec -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen %s 2>&1 | FileCheck %s

; HIR-run.
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-enable-soa-hir -vplan-dump-soa-info -vplan-enable-hir-private-arrays\
; RUN: -disable-output  -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES:asserts

; Source-file: test.c
; int foo(int n1, int **out) {
;  int index;
;
; #pragma omp simd private(arr_ne, arr_e)
;  for (index = 0; index < 1024; index++) {
;    if (index % 2 == 0)
;      arr_ne[index] = index + n1;
;    else
;      arr_ne[index] = index - n1;
;
;    if ((index + n1) > 1024)
;      *out = &arr_e[n1];
;   }
;   return arr_e[n1];
; }


; Compile-command: icx test.c -o out.ll -fiopenmp -O1 -S  \
; -mllvm -disable-vplan-codegen -mllvm -vplan-entities-dump \
; -mllvm -vplan-use-entity-instr -emit-llvm

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr_ne = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr_e = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind uwtable
define dso_local i32 @foo(i32 %n1, ptr nocapture %out) local_unnamed_addr {
; CHECK-LABEL:  SOA profitability:
; CHECK-NEXT:  SOASafe = [[VP_INDEX_LPRIV:%.*]] (index.lpriv) Profitable = 1
; CHECK-NEXT:  SOAUnsafe = [[VP_ARR_E_PRIV:%.*]] (arr_e.priv)
; CHECK-NEXT:  SOASafe = [[VP_ARR_NE_PRIV:%.*]] (arr_ne.priv) Profitable = 0
; CHECK-NEXT:  SOA profitability:
; CHECK-NEXT:  SOASafe = [[VP0:%.*]] Profitable = 1
; CHECK-NEXT:  SOAUnsafe = [[VP1:%.*]]
; CHECK-NEXT:  SOASafe = [[VP2:%.*]] Profitable = 0
omp.inner.for.body.lr.ph:
  %arr_e.priv = alloca [1024 x i32], align 4
  %arr_ne.priv = alloca [1024 x i32], align 4
  %index.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
%0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr_ne.priv, i32 0, i32 1024), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr_e.priv, i32 0, i32 1024), "QUAL.OMP.PRIVATE:TYPED"(ptr %index.lpriv, i32 0, i32 1)]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %idxprom8 = sext i32 %n1 to i64
  %arrayidx9 = getelementptr inbounds [1024 x i32], ptr %arr_e.priv, i64 0, i64 %idxprom8
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %DIR.OMP.SIMD.2 ]
  %rem1925 = and i64 %indvars.iv, 1
  %cmp1 = icmp eq i64 %rem1925, 0
  %1 = trunc i64 %indvars.iv to i32
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %omp.inner.for.body
  %add2 = add nsw i32 %1, %n1
  %arrayidx = getelementptr inbounds [1024 x i32], ptr %arr_ne.priv, i64 0, i64 %indvars.iv
  store i32 %add2, ptr %arrayidx, align 4
  br label %if.end

if.else:                                          ; preds = %omp.inner.for.body
  %sub = sub nsw i32 %1, %n1
  %arrayidx4 = getelementptr inbounds [1024 x i32], ptr %arr_ne.priv, i64 0, i64 %indvars.iv
  store i32 %sub, ptr %arrayidx4, align 4
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %2 = trunc i64 %indvars.iv to i32
  %add5 = add nsw i32 %2, %n1
  %cmp6 = icmp sgt i32 %add5, 1024
  br i1 %cmp6, label %if.then7, label %omp.inner.for.inc

if.then7:                                         ; preds = %if.end
  store ptr %arrayidx9, ptr %out, align 8
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then7, %if.end
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  store i32 1023, ptr %index.lpriv, align 4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  %idxprom12 = sext i32 %n1 to i64
  %arrayidx13 = getelementptr inbounds [1024 x i32], ptr @arr_e, i64 0, i64 %idxprom12
  %3 = load i32, ptr %arrayidx13, align 4
  ret i32 %3
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
