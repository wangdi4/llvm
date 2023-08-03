; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck --check-prefixes=CHECK,CHECKPA %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck --check-prefixes=CHECK,CHECKPA %s
; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck --check-prefixes=CHECK,CHECKIV %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck --check-prefixes=CHECK,CHECKIV %s
; end INTEL_CUSTOMIZATION
;
; This test checks that paropt pass correctly adds parallel access metadata to
; nested loops with non-monotonic property (such as a loop with nonmonotonic
; schedule modifier or a plain simd loops with no safelen).
;
; void test1(float *P, unsigned N) {
; #pragma omp for schedule(nonmonotonic: auto)
;   for (int I = 0 ; I < N; I++) {
;     P[I * 100] = 5.0;
;
; #pragma omp simd
;     for (int J = 0 ; J < 100; J++)
;       P[I * 100 + J] += J;
;   }
; }

; CHECK-LABEL: define void @test1
; CHECK: call i32 @__kmpc_dispatch_next_4u(
; CHECK: store float 5.000000e+00{{.*}}, !llvm.access.group ![[AG1:[0-9]+]]
; CHECK: br i1 %{{.*}}, label %[[LOOP_PH:.+]], label %[[LOOP_INC:.+]]
;
; CHECK: [[LOOP_PH]]:
; CHECK:   [[T1:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
; CHECK:   br label %[[LOOP:.+]]
;
; CHECK: [[LOOP]]:
; CHECK:   store float %{{.+}}{{.*}}, !llvm.access.group ![[AGM:[0-9]+]]
; CHECK:   br i1 %{{.+}}, label %[[LOOP]], label %[[LOOP_END:.+]], !llvm.loop ![[ID2:[0-9]+]]
;
; CHECK: [[LOOP_END]]:
; CHECK:   call void @llvm.directive.region.exit(token [[T1]]) [ "DIR.OMP.END.SIMD"() ]
; CHECK:   br label %[[LOOP_INC]]
;
; CHECK: [[LOOP_INC]]
; CHECK:   br i1 %{{.+}}, label %{{.+}}, label %{{.+}}, !llvm.loop ![[ID1:[0-9]+]]

; CHECKPA-DAG: ![[AG1]] = distinct !{}
; CHECKPA-DAG: ![[AGM]] = !{![[AG1]], ![[AG2:[0-9]+]]}
; CHECKPA-DAG: ![[AG2]] = distinct !{}
; CHECKPA-DAG: ![[PA2:[0-9]+]] = !{!"llvm.loop.parallel_accesses", ![[AG2]]}
; CHECKPA-DAG: ![[ID2]] = distinct !{![[ID2]]{{.*}}, ![[PA2]]{{[,\}]}}
; CHECKPA-DAG: ![[PA1:[0-9]+]] = !{!"llvm.loop.parallel_accesses", ![[AG1]]}
; CHECKPA-DAG: ![[ID1]] = distinct !{![[ID1]]{{.*}}, ![[PA1]]{{[,\}]}}

; INTEL_CUSTOMIZATION
; CHECKIV-DAG: ![[IV1:[0-9]+]] = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
; CHECKIV-DAG: ![[ID1]] = distinct !{![[ID1]]{{.*}}, ![[IV1]]{{[,\}]}}
; CHECKIV-DAG: ![[ID2]] = distinct !{![[ID2]]{{.*}}, ![[IV1]]{{[,\}]}}
; end INTEL_CUSTOMIZATION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @test1(ptr noundef %P, i32 noundef %N) {
entry:
  %P.addr = alloca ptr, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  %tmp7 = alloca i32, align 4
  %.omp.iv8 = alloca i32, align 4
  %.omp.ub9 = alloca i32, align 4
  %J = alloca i32, align 4
  store ptr %P, ptr %P.addr, align 8
  store i32 %N, ptr %N.addr, align 4
  %0 = load i32, ptr %N.addr, align 4
  store i32 %0, ptr %.capture_expr.0, align 4
  %1 = load i32, ptr %.capture_expr.0, align 4
  %sub = sub i32 %1, 0
  %sub1 = sub i32 %sub, 1
  %add = add i32 %sub1, 1
  %div = udiv i32 %add, 1
  %sub2 = sub i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.1, align 4
  %2 = load i32, ptr %.capture_expr.0, align 4
  %cmp = icmp ult i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, ptr %.omp.lb, align 4
  %3 = load i32, ptr %.capture_expr.1, align 4
  store i32 %3, ptr %.omp.ub, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.SCHEDULE.AUTO:NONMONOTONIC"(i32 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %I, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv8, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub9, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %J, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp7, i32 0, i32 1) ]

  %5 = load i32, ptr %.omp.lb, align 4
  store i32 %5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc22, %omp.precond.then
  %6 = load i32, ptr %.omp.iv, align 4
  %7 = load i32, ptr %.omp.ub, align 4
  %add3 = add i32 %7, 1
  %cmp4 = icmp ult i32 %6, %add3
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end24

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, ptr %.omp.iv, align 4
  %mul = mul i32 %8, 1
  %add5 = add i32 0, %mul
  store i32 %add5, ptr %I, align 4
  %9 = load ptr, ptr %P.addr, align 8
  %10 = load i32, ptr %I, align 4
  %mul6 = mul nsw i32 %10, 100
  %idxprom = sext i32 %mul6 to i64
  %arrayidx = getelementptr inbounds float, ptr %9, i64 %idxprom
  store float 5.000000e+00, ptr %arrayidx, align 4
  store i32 99, ptr %.omp.ub9, align 4
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv8, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub9, i32 0),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %J, i32 0, i32 1, i32 1) ]

  store i32 0, ptr %.omp.iv8, align 4
  br label %omp.inner.for.cond10

omp.inner.for.cond10:                             ; preds = %omp.inner.for.inc, %omp.inner.for.body
  %12 = load i32, ptr %.omp.iv8, align 4
  %13 = load i32, ptr %.omp.ub9, align 4
  %cmp11 = icmp sle i32 %12, %13
  br i1 %cmp11, label %omp.inner.for.body12, label %omp.inner.for.end

omp.inner.for.body12:                             ; preds = %omp.inner.for.cond10
  %14 = load i32, ptr %.omp.iv8, align 4
  %mul13 = mul nsw i32 %14, 1
  %add14 = add nsw i32 0, %mul13
  store i32 %add14, ptr %J, align 4
  %15 = load i32, ptr %J, align 4
  %conv = sitofp i32 %15 to float
  %16 = load ptr, ptr %P.addr, align 8
  %17 = load i32, ptr %I, align 4
  %mul15 = mul nsw i32 %17, 100
  %18 = load i32, ptr %J, align 4
  %add16 = add nsw i32 %mul15, %18
  %idxprom17 = sext i32 %add16 to i64
  %arrayidx18 = getelementptr inbounds float, ptr %16, i64 %idxprom17
  %19 = load float, ptr %arrayidx18, align 4
  %add19 = fadd fast float %19, %conv
  store float %add19, ptr %arrayidx18, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body12
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %20 = load i32, ptr %.omp.iv8, align 4
  %add20 = add nsw i32 %20, 1
  store i32 %add20, ptr %.omp.iv8, align 4
  br label %omp.inner.for.cond10, !llvm.loop !5

omp.inner.for.end:                                ; preds = %omp.inner.for.cond10
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.SIMD"() ]

  br label %omp.body.continue21

omp.body.continue21:                              ; preds = %omp.loop.exit
  br label %omp.inner.for.inc22

omp.inner.for.inc22:                              ; preds = %omp.body.continue21
  %21 = load i32, ptr %.omp.iv, align 4
  %add23 = add nuw i32 %21, 1
  store i32 %add23, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end24:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit25

omp.loop.exit25:                                  ; preds = %omp.inner.for.end24
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.LOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit25, %entry
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.vectorize.enable", i1 true}
