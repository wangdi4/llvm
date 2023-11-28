; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck %s

; Test src:
;
; #define LOOP() for (int i = 0; i < 1000; ++i)
; void for_ordered(int *x) {
; #pragma omp for ordered
;   LOOP();
; }

; Verify correctness of CFG and run-time calls.

; CHECK-LABEL: @for_ordered

; CHECK: call i32 @__kmpc_global_thread_num(

; ZTT block:
; CHECK: %[[ZTT1:.+]] = icmp sle i32 %[[LB:[^,]+]], %[[UB:[^,]+]]
; CHECK: br i1 %[[ZTT1]], label %[[PHB:[^,]+]], label %[[FOREND:[^,]+]]

; PHB:
; CHECK: [[PHB]]:
; 'schedule' 66 is kmp_ord_static
; CHECK: call void @__kmpc_dispatch_init_4({{.*}}, i32 %[[TID:[^,]+]], i32 66, i32 %[[DISPLB:[^,]+]], i32 %[[DISPUB:[^,]+]], i32 1, i32 1)
; CHECK: br label %[[DISPNEXT:[^,]+]]

; DISPNEXT:
; CHECK: [[DISPNEXT]]:
; CHECK: %[[NEXT:.+]] = call i32 @__kmpc_dispatch_next_4({{.*}}, i32 %[[TID]], ptr %[[PISLAST:[^,]+]], ptr %[[PLB:[^,]+]], ptr %[[PUB:[^,]+]], ptr %[[PST:[^,]+]])
; CHECK: %[[NEXTP:.+]] = icmp ne i32 %[[NEXT]], 0
; CHECK: br i1 %[[NEXTP]], label %[[DISPB:[^,]+]], label %[[REGIONEXIT:[^,]+]]

; DISPB:
; CHECK: [[DISPB]]:
; CHECK: %[[LBNEW:.+]] = load i32, ptr %[[PLB]]
; CHECK: %[[UBNEW:.+]] = load i32, ptr %[[PUB]]
; CHECK: %[[ZTT2:.+]] = icmp sle i32 %[[LBNEW]], %[[UBNEW]]
; CHECK: br i1 %[[ZTT2]], label %[[LOOPBODY:[^,]+]], label %[[REGIONEXIT:[^,]+]]

; LOOPBODY:
; CHECK: [[LOOPBODY]]:
; CHECK: call void @__kmpc_dispatch_fini_4({{.*}}, i32 %[[TID]])
; CHECK: br i1 {{.*}}, label %[[LOOPBODY]], label %[[DISPNEXT:[^,]+]]

; REGIONEXIT:
; CHECK: [[REGIONEXIT]]:
; CHECK: br label %[[FOREND]]

; FOREND:
; CHECK: [[FOREND]]:
; CHECK: call void @__kmpc_barrier

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @for_ordered(ptr noundef %x) #0 {
entry:
  %x.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %x, ptr %x.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 999, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.ORDERED"(i32 0),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]
  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
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
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }


