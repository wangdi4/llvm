; This test verifies that private-variables escaping into the unknown functions
; are not safe for data-layout transformations.

; RUN: opt -S -vplan-vec -vplan-enable-masked-variant=0 -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen %s 2>&1 | FileCheck %s

; HIR-run.
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-enable-masked-variant-hir=0 -vplan-enable-soa-hir -vplan-dump-soa-info\
; RUN: -disable-output  -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES:asserts

; CHECK-DAG: SOASafe = arr.priv
; CHECK-DAG: SOASafe = arr_ne.priv
;; This next one privates escapes into the unknown function and is unsafe.
; CHECK-DAG: SOAUnsafe = arr_e.priv
; CHECK-DAG: SOASafe = index.lpriv

; Source-file: test.c
;int arr[1024];
;int arr_e[1024];
;int arr_ne[1024];
;extern int helper(int *elem);
;int foo(int n1) {
;  int index;
;
;#pragma omp simd private(arr, arr_e, arr_ne)
;  for (index = 0; index < 1024; index++) {
;    if (arr[index] > 0) {
;      arr[index + n1] = index + n1 * n1 + 3;
;    }
;
;    if (index % 2 == 0)
;      arr_ne[index] = index + n1;
;    else
;      arr_ne[index] = index - n1;
;
;    if (index % n1 == 1)
;      arr_e[index] = index - n1;
;    else
;      arr_e[index] = helper(arr_e);
;  }
;  return arr_e[0];
;}

; Compile-command: icx test.c -o out.ll -fiopenmp -O1 -S  \
; -mllvm -disable-vplan-codegen -mllvm -vplan-entities-dump \
; -mllvm -vplan-use-entity-instr -emit-llvm

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr_e = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@arr_ne = common dso_local local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

define dso_local void @foo(i32 %n1) local_unnamed_addr {
omp.inner.for.body.lr.ph:
  %arr_ne.priv = alloca [1024 x i32], align 4
  %arr_e.priv = alloca [1024 x i32], align 4
  %arr.priv = alloca [1024 x i32], align 4
  %index.lpriv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"([1024 x i32]* %arr.priv), "QUAL.OMP.PRIVATE"([1024 x i32]* %arr_e.priv), "QUAL.OMP.PRIVATE"([1024 x i32]* %arr_ne.priv), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.PRIVATE"(i32* %index.lpriv)]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %mul2 = mul nsw i32 %n1, %n1
  %add3 = add nuw i32 %mul2, 3
  %1 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_e.priv, i64 0, i64 0
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ %indvars.iv.next, %omp.inner.for.inc ], [ 0, %DIR.OMP.SIMD.2 ]
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %index.lpriv, align 4
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr.priv, i64 0, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx, align 4
  %cmp1 = icmp sgt i32 %3, 0
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %omp.inner.for.body
  %4 = trunc i64 %indvars.iv to i32
  %add4 = add i32 %add3, %4
  %5 = trunc i64 %indvars.iv to i32
  %add5 = add nsw i32 %5, %n1
  %idxprom6 = sext i32 %add5 to i64
  %arrayidx7 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr.priv, i64 0, i64 %idxprom6
  store i32 %add4, i32* %arrayidx7, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %omp.inner.for.body
  %rem3339 = and i64 %indvars.iv, 1
  %cmp8 = icmp eq i64 %rem3339, 0
  %6 = trunc i64 %indvars.iv to i32
  br i1 %cmp8, label %if.then9, label %if.else

if.then9:                                         ; preds = %if.end
  %add10 = add nsw i32 %6, %n1
  %arrayidx12 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_ne.priv, i64 0, i64 %indvars.iv
  store i32 %add10, i32* %arrayidx12, align 4
  br label %if.end15

if.else:                                          ; preds = %if.end
  %sub = sub nsw i32 %6, %n1
  %arrayidx14 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_ne.priv, i64 0, i64 %indvars.iv
  store i32 %sub, i32* %arrayidx14, align 4
  br label %if.end15

if.end15:                                         ; preds = %if.else, %if.then9
  %7 = trunc i64 %indvars.iv to i32
  %rem16 = srem i32 %7, %n1
  %cmp17 = icmp eq i32 %rem16, 1
  br i1 %cmp17, label %if.then18, label %if.else22

if.then18:                                        ; preds = %if.end15
  %8 = trunc i64 %indvars.iv to i32
  %sub19 = sub nsw i32 %8, %n1
  %arrayidx21 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_e.priv, i64 0, i64 %indvars.iv
  store i32 %sub19, i32* %arrayidx21, align 4
  br label %omp.inner.for.inc

if.else22:                                        ; preds = %if.end15
  %call = call i32 @helper(i32* nonnull %1)
  %9 = load i32, i32* %index.lpriv, align 4
  %idxprom23 = sext i32 %9 to i64
  %arrayidx24 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_e.priv, i64 0, i64 %idxprom23
  store i32 %call, i32* %arrayidx24, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.else22, %if.then18
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  ret void
}

; This test checks that we correctly identify SOA-unsafe variables on account of
; unsafe load/store instructions (loaded type size != alloca element size).
define void @test_unsafe_addrspacecast() {
; CHECK:        SOA profitability
; CHECK-NEXT:  SOASafe = arr_ne.priv
; CHECK-NEXT:  SOAUnsafe = arr_e.priv
;
  %arr_e.priv = alloca [1024 x i32], align 4
  %arr_ne.priv = alloca [1024 x i32], align 4
  br label %simd.begin.region
simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"([1024 x i32]* %arr_e.priv, [1024 x i32]* %arr_ne.priv) ]
  br label %simd.loop
simd.loop:
  %index = phi i64 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.end ]
  %uni.gep1 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_e.priv, i64 0, i64 0
  %uni.gep2 = getelementptr inbounds [1024 x i32], [1024 x i32]* %arr_ne.priv, i64 0, i64 0
  %Y = addrspacecast i32* %uni.gep1 to i64 addrspace(2)*
  %Z = addrspacecast i32* %uni.gep2 to i64 addrspace(2)*
  %ld = load i32, i32* %uni.gep1
  %ld1 = load i64, i64 addrspace(2)* %Y
  br label %simd.loop.end
simd.loop.end:
  %indvar = add nuw i64 %index, 1
  %vl.cond = icmp ult i64 %indvar, 1024
  br i1 %vl.cond, label %simd.end.region, label %simd.loop
simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %for.end
for.end:
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare dso_local i32 @helper(i32*) local_unnamed_addr
