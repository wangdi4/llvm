; This test verifies that private-variables escaping into the unknown functions
; are not safe for data-layout transformations.

; RUN: opt -S -opaque-pointers -vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen %s 2>&1 | FileCheck %s

; HIR-run.
; RUN: opt -opaque-pointers -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa-hir -vplan-dump-soa-info\
; RUN: -disable-output  -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES:asserts

; CHECK-DAG: SOAUnsafe = arr_e.priv
; CHECK-DAG: SOAUnsafe = arr_e2.priv

; Source-file: test.c
; int arr_e[1024];
; int arr_e2[1024];
;
; extern int helper(int *elem);
;
; int foo(int n1) {
;   int index;
;
; #pragma omp simd private(arr_e, arr_e2)
;   for (index = 0; index < 1024; index++) {
;       arr_e[index] = helper(arr_e);
;       arr_e2[index] = helper(&arr_e2[index+3]);
;   }
;   return arr_e[0];
;
; }

; Compile-command: icx test.c -o out.ll -fiopenmp -O1 -S -emit-llvm  \
; -mllvm -print-before=vplan-vec -mllvm -print-module-scope

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr_e = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr_e2 = dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

define dso_local i32 @foo(i32 %n1) {
DIR.OMP.SIMD.211:
  %index.linear.iv = alloca i32, align 4
  %arr_e2.priv = alloca [1024 x i32], align 16
  %arr_e.priv = alloca [1024 x i32], align 16
  br label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.SIMD.211
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr_e.priv, i32 0, i32 1024), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr_e2.priv, i32 0, i32 1024), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %index.linear.iv, i32 0, i32 1, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i8 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i8 0) ]
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %1 = getelementptr inbounds [1024 x i32], ptr %arr_e.priv, i64 0, i64 0
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.1, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %call = call i32 @helper(ptr nonnull %1)
  %arrayidx = getelementptr inbounds [1024 x i32], ptr %arr_e.priv, i64 0, i64 %indvars.iv
  store i32 %call, ptr %arrayidx, align 4
  %2 = add nuw nsw i64 %indvars.iv, 3
  %arrayidx3 = getelementptr inbounds [1024 x i32], ptr %arr_e2.priv, i64 0, i64 %2
  %call4 = call i32 @helper(ptr nonnull %arrayidx3)
  %arrayidx6 = getelementptr inbounds [1024 x i32], ptr %arr_e2.priv, i64 0, i64 %indvars.iv
  store i32 %call4, ptr %arrayidx6, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond.not, label %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge, label %omp.inner.for.body

omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge: ; preds = %omp.inner.for.body
  store i32 1024, ptr %index.linear.iv, align 4
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.4.loopexit_crit_edge
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.221

DIR.OMP.END.SIMD.221:                             ; preds = %DIR.OMP.END.SIMD.2
  %3 = load i32, ptr getelementptr inbounds ([1024 x i32], ptr @arr_e, i64 0, i64 0), align 16
  ret i32 %3
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local i32 @helper(ptr)
