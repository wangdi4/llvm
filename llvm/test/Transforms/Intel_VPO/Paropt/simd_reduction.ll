; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED

; Original code:
; void foo()
; {
;   int l = 0;
; #pragma omp simd reduction(+:l)
;   for (int i = 0; i < 1000; ++i)
;     ++l;
; }

; CRITICAL: [[ZTT:%.+]] = icmp sle i32 0, %{{.+}}
; CRITICAL: br i1 [[ZTT]], label %[[PHB:[^,]+]], label %[[REXIT:[^,]+]]
; CRITICAL: [[PHB]]:
; CRITICAL: [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %[[RPRIV:[^,]+]], i32 0, i32 1), {{.*}} ]
; CRITICAL: br label %[[LOOPBODY:[^,]+]]
; CRITICAL: [[LOOPBODY]]:
; CRITICAL: load {{.*}}ptr %[[RPRIV]]
; CRITICAL: store {{.*}}ptr %[[RPRIV]]
; CRITICAL: br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LEXIT:[^,]+]]
; CRITICAL: [[LEXIT]]:
; CRITICAL: call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]
; CRITICAL: br label %[[LEXIT_SPLIT:[^,]+]]
; CRITICAL: [[LEXIT_SPLIT]]:
; CRITICAL: br label %[[LEXIT_SPLIT_SPLIT:[^,]+]]
; CRITICAL: [[LEXIT_SPLIT_SPLIT]]:
; CRITICAL: %[[V:.+]] = load i32, ptr %[[RPRIV]]
; CRITICAL: %[[OV:.+]] = load i32, ptr %l
; CRITICAL: %[[ADD:.+]] = add i32 %[[OV]], %[[V]]
; CRITICAL: store i32 %[[ADD]], ptr %l
; CRITICAL: br label %[[REXIT]]
; CRITICAL: [[REXIT]]:

; FASTRED: [[ZTT:%.+]] = icmp sle i32 0, %{{.+}}
; FASTRED: br i1 [[ZTT]], label %[[PHB:[^,]+]], label %[[REXIT:[^,]+]]
; FASTRED: [[PHB]]:
; FASTRED: [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %[[RPRIV:[^,]+]], i32 0, i32 1), {{.*}} ]
; FASTRED: br label %[[LOOPBODY:[^,]+]]
; FASTRED: [[LOOPBODY]]:
; FASTRED: load {{.*}}ptr %[[RPRIV]]
; FASTRED: store {{.*}}ptr %[[RPRIV]]
; FASTRED: br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LEXIT:[^,]+]]
; FASTRED: [[LEXIT]]:
; FASTRED: call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]
; FASTRED: %[[R:.+]] = load i32, ptr %[[RPRIV]]
; FASTRED: store i32 %[[R]], ptr %[[FRPRIV:.+]]
; FASTRED: br label %[[LEXIT_SPLIT:[^,]+]]
; FASTRED: [[LEXIT_SPLIT]]:
; FASTRED: br label %[[LEXIT_SPLIT_SPLIT:[^,]+]]
; FASTRED: [[LEXIT_SPLIT_SPLIT]]:
; FASTRED: %[[V:.+]] = load i32, ptr %[[FRPRIV]]
; FASTRED: %[[OV:.+]] = load i32, ptr %l
; FASTRED: %[[ADD:.+]] = add i32 %[[OV]], %[[V]]
; FASTRED: store i32 %[[ADD]], ptr %l
; FASTRED: br label %[[REXIT]]
; FASTRED: [[REXIT]]:

; ModuleID = 'simd.cpp'
source_filename = "simd.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() {
entry:
  %l = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %l, align 4
  store i32 999, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %l, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i, i32 0, i32 1, i32 1) ]

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
  %4 = load i32, ptr %l, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, ptr %l, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %5 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %5, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  ret void
}

declare token @llvm.directive.region.entry() 

declare void @llvm.directive.region.exit(token)
