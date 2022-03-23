; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; void foo(int *a)
; {
;   int i, l = 0;
; #pragma omp parallel
; #pragma omp simd lastprivate(l)
;   for (i = 0; i < 10000; ++i) {
;     l = a[i];
;     ++l;
;   }
; }

; CHECK: [[ZTT:%.+]] = icmp sle i32 0, %{{.+}}
; CHECK: br i1 [[ZTT]], label %[[PHB:[^,]+]], label %[[REXIT:[^,]+]]
; CHECK: [[PHB]]:
; CHECK: [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),{{.*}}"QUAL.OMP.LASTPRIVATE"(i32* %[[LPRIV:[^,]+]]){{.*}} ]
; CHECK: br label %[[LOOPBODY:[^,]+]]
; CHECK: [[LOOPBODY]]:
; CHECK: store {{.*}}i32* %[[LPRIV]]
; CHECK: load {{.*}}i32* %[[LPRIV]]
; CHECK: store {{.*}}i32* %[[LPRIV]]
; CHECK: br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LEXIT:[^,]+]]
; CHECK: [[LEXIT]]:
; CHECK: call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]
; CHECK: %[[V:.+]] = load i32, i32* %[[LPRIV]]
; CHECK: store i32 %[[V]], i32* %l
; CHECK: br label %[[REXIT]]

; ModuleID = 'par_simd_lastprivate.c'
source_filename = "par_simd_lastprivate.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo(i32* %a) #0 {
entry:
  %a.addr = alloca i32*, align 8
  %i = alloca i32, align 4
  %l = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32* %a, i32** %a.addr, align 8
  store i32 0, i32* %l, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.SHARED"(i32* %l), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.SHARED"(i32** %a.addr), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  store i32 9999, i32* %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE"(i32* %l), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
  store i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %5 = load i32*, i32** %a.addr, align 8
  %6 = load i32, i32* %i, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds i32, i32* %5, i64 %idxprom
  %7 = load i32, i32* %arrayidx, align 4
  store i32 %7, i32* %l, align 4
  %8 = load i32, i32* %l, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, i32* %l, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %9, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind uwtable "may-have-openmp-directive"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
