; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplify-cfg)' -S %s | FileCheck %s
;
; Check that paropt pass adds "llvm.loop.intel.loopcount_maximum" matadata to
; the parallel loop with known loop bounds.
;
; void foo(int *P) {
; #pragma omp parallel for
;   for (int I = -2500; I < 2500; I += 5)
;     P[I] = I;
; }
;
;
; CHECK: define dso_local void @foo(i32* %P) {
; CHECK: call{{.+}}  @__kmpc_fork_call(%struct.ident_t* {{.+}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32**, i64, i32*)* [[OUTLINED_FUNC:@.+]] to void (i32*, i32*, ...)*), 
; CHECK: }

; CHECK: define internal void [[OUTLINED_FUNC]](i32* %{{.+}}, i32* %{{.+}}, i32** %{{.+}}, i64 %{{.+}}, i32* %{{.+}}) #{{[0-9]+}} {
; CHECK:   call void @__kmpc_for_static_init_4(%struct.ident_t* {{.+}}, i32 %{{.+}}, i32 34, i32* %{{.+}}, i32* [[LBA:%.+]], i32* [[UBA:%.+]], i32* %{{.+}}, i32 1, i32 1)
; CHECK:   [[LB:%.+]] = load i32, i32* [[LBA]]
; CHECK:   [[UB:%.+]] = load i32, i32* [[UBA]]
; CHECK:   [[ZTT:%.+]] = icmp sle i32 [[LB]], [[UB]]
; CHECK:   br i1 [[ZTT]], label %[[LOOP_BODY:.+]], label %[[LOOP_EXIT:.+]]
;
; CHECK: [[LOOP_BODY]]:
; CHECK:   [[IV:%.+]] = phi i32 [ [[ADD:%.+]], %[[LOOP_BODY]] ], [ [[LB]], %[[LOOP_PH:.+]] ]
; CHECK:   [[ADD]] = add nsw i32 [[IV]], 1
; CHECK:   [[CMP:%.+]] = icmp sle i32 [[ADD]], [[UB]]
; CHECK:   br i1 [[CMP]], label %[[LOOP_BODY]], label %[[LOOP_EXIT]], !llvm.loop [[LOOP_MD:![0-9]+]]
;
; CHECK: [[LOOP_EXIT]]:
; CHECK:   call void @__kmpc_for_static_fini(%struct.ident_t* {{.+}}, i32 %{{.+}})
; CHECK: }

; CHECK: [[LOOP_MD]] = distinct !{[[LOOP_MD]], !{{[0-9]+}}, [[LOOP_CM:![0-9]+]]}
; CHECK: [[LOOP_CM]] = !{!"llvm.loop.intel.loopcount_maximum", i32 1000}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(i32* %P) {
entry:
  %P.addr = alloca i32*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32* %P, i32** %P.addr, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 999, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(i32** %P.addr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %I) ]
  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 5
  %add = add nsw i32 -2500, %mul
  store i32 %add, i32* %I, align 4
  %5 = load i32, i32* %I, align 4
  %6 = load i32*, i32** %P.addr, align 8
  %7 = load i32, i32* %I, align 4
  %idxprom = sext i32 %7 to i64
  %ptridx = getelementptr inbounds i32, i32* %6, i64 %idxprom
  store i32 %5, i32* %ptridx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
