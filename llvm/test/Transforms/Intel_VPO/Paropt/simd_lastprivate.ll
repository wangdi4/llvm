; RUN: opt  -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; void foo(int *a)
; {
;   int i, l = 0;
; #pragma omp simd lastprivate(l)
;   for (i = 0; i < 10000; ++i) {
;     l = a[i];
;     ++l;
;   }
; }

; CHECK: [[ZTT:%.+]] = icmp sle i32 0, %{{.+}}
; CHECK: br i1 [[ZTT]], label %[[PHB:[^,]+]], label %[[REXIT:[^,]+]]
; CHECK: [[PHB]]:
; CHECK: [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),{{.*}}"QUAL.OMP.LASTPRIVATE:TYPED"(ptr %[[LPRIV:[^,]+]], i32 0, i32 1){{.*}} ]
; CHECK: br label %[[LOOPBODY:[^,]+]]
; CHECK: [[LOOPBODY]]:
; CHECK: store {{.*}}ptr %[[LPRIV]]
; CHECK: load {{.*}}ptr %[[LPRIV]]
; CHECK: store {{.*}}ptr %[[LPRIV]]
; CHECK: br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LEXIT:[^,]+]]
; CHECK: [[LEXIT]]:
; CHECK: call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]
; CHECK: %[[V:.+]] = load i32, ptr %[[LPRIV]]
; CHECK: store i32 %[[V]], ptr %l
; CHECK: br label %[[REXIT]]
; CHECK: [[REXIT]]:

; ModuleID = 'simd_lastprivate.c'
source_filename = "simd_lastprivate.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr noundef %a) {
entry:
  %a.addr = alloca ptr, align 8
  %i = alloca i32, align 4
  %l = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store ptr %a, ptr %a.addr, align 8
  store i32 0, ptr %l, align 4
  store i32 9999, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %l, i32 0, i32 1),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

  store i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %1 = load i32, ptr %.omp.iv, align 4
  %2 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %1, %2
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %3 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %3, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %4 = load ptr, ptr %a.addr, align 8
  %5 = load i32, ptr %i, align 4
  %idxprom = sext i32 %5 to i64
  %arrayidx = getelementptr inbounds i32, ptr %4, i64 %idxprom
  %6 = load i32, ptr %arrayidx, align 4
  store i32 %6, ptr %l, align 4
  %7 = load i32, ptr %l, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, ptr %l, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  %9 = load i32, ptr %i, align 4
  %add2 = add nsw i32 %9, 1
  store i32 %add2, ptr %i, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
