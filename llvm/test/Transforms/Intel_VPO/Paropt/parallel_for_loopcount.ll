; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck %s
;
; Check that paropt pass adds "llvm.loop.intel.loopcount_maximum" and
; "llvm.loop.intel.loopcount_average" metadata to the parallel loop with known
; loop bounds and/or schedule chunk.
;
; void foo(int *P) {
; #pragma omp parallel for
;   for (int I = -2500; I < 2500; I += 5)
;     P[I] = I;
; }
;
; void bar(int *P) {
; #pragma omp parallel for schedule(static, 25)
;   for (int I = -2500; I < 2500; I += 5)
;     P[I] = I;
; }
;
; CHECK: define dso_local void @foo(i32* %P) {
; CHECK: call{{.+}}  @__kmpc_fork_call(%struct.ident_t* {{.+}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32**, i64, i32*)* [[FOO_OUTLINED_FUNC:@.+]] to void (i32*, i32*, ...)*), 
; CHECK: }

; CHECK: define dso_local void @bar(i32* %P) {
; CHECK: call{{.+}}  @__kmpc_fork_call(%struct.ident_t* {{.+}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32**, i64, i32*)* [[BAR_OUTLINED_FUNC:@.+]] to void (i32*, i32*, ...)*), 
; CHECK: }

; CHECK: define internal void [[FOO_OUTLINED_FUNC]](i32* %{{.+}}, i32* %{{.+}}, i32** %{{.+}}, i64 %{{.+}}, i32* %{{.+}}) #{{[0-9]+}} {
; CHECK:   call void @__kmpc_for_static_init_4(%struct.ident_t* {{.+}}, i32 %{{.+}}, i32 34, i32* %{{.+}}, i32* [[FOO_LBA:%.+]], i32* [[FOO_UBA:%.+]], i32* %{{.+}}, i32 1, i32 1)
; CHECK:   [[FOO_LB:%.+]] = load i32, i32* [[FOO_LBA]]
; CHECK:   [[FOO_UB:%.+]] = load i32, i32* [[FOO_UBA]]
; CHECK:   [[FOO_ZTT:%.+]] = icmp sle i32 [[FOO_LB]], [[FOO_UB]]
; CHECK:   br i1 [[FOO_ZTT]], label %[[FOO_LOOP_BODY:.+]], label %[[FOO_LOOP_EXIT:.+]]
;
; CHECK: [[FOO_LOOP_BODY]]:
; CHECK:   [[FOO_IV:%.+]] = phi i32 [ [[FOO_ADD:%.+]], %[[FOO_LOOP_BODY]] ], [ [[FOO_LB]], %[[FOO_LOOP_PH:.+]] ]
; CHECK:   [[FOO_ADD]] = add nsw i32 [[FOO_IV]], 1
; CHECK:   [[FOO_CMP:%.+]] = icmp sle i32 [[FOO_ADD]], [[FOO_UB]]
; CHECK:   br i1 [[FOO_CMP]], label %[[FOO_LOOP_BODY]], label %[[FOO_LOOP_EXIT]], !llvm.loop [[FOO_LOOP_MD:![0-9]+]]
;
; CHECK: [[FOO_LOOP_EXIT]]:
; CHECK:   call void @__kmpc_for_static_fini(%struct.ident_t* {{.+}}, i32 %{{.+}})
; CHECK: }

; CHECK: define internal void [[BAR_OUTLINED_FUNC]](i32* %{{.+}}, i32* %{{.+}}, i32** %{{.+}}, i64 %{{.+}}, i32* %{{.+}}) #{{[0-9]+}} {
; CHECK:   call void @__kmpc_for_static_init_4(%struct.ident_t* {{.+}}, i32 %{{.+}}, i32 33, i32* %{{.+}}, i32* [[BAR_LBA:%.+]], i32* [[BAR_UBA:%.+]], i32* %{{.+}}, i32 1, i32 25)
; CHECK:   br label %[[BAR_DISPATCH_HEADER:.+]]
;
; CHECK: [[BAR_DISPATCH_HEADER]]:
; CHECK:  br i1 %{{.+}}, label %[[BAR_DISPATCH_BODY:.+]], label %{{.+}}
;
; CHECK: [[BAR_DISPATCH_BODY]]:
; CHECK:   [[BAR_LB:%.+]] = load i32, i32* [[BAR_LBA]]
; CHECK:   [[BAR_UB:%.+]] = load i32, i32* [[BAR_UBA]]
; CHECK:   [[BAR_ZTT:%.+]] = icmp sle i32 [[BAR_LB]], [[BAR_UB]]
; CHECK:   br i1 [[BAR_ZTT]], label %[[BAR_LOOP_BODY:.+]], label %[[BAR_LOOP_EXIT:.+]]
;
; CHECK: [[BAR_LOOP_BODY]]:
; CHECK:   [[BAR_IV:%.+]] = phi i32 [ [[BAR_ADD:%.+]], %[[BAR_LOOP_BODY]] ], [ [[BAR_LB]], %[[BAR_DISPATCH_BODY]] ]
; CHECK:   [[BAR_ADD]] = add nsw i32 [[BAR_IV]], 1
; CHECK:   [[BAR_CMP:%.+]] = icmp sle i32 [[BAR_ADD]], [[BAR_UB]]
; CHECK:   br i1 [[BAR_CMP]], label %[[BAR_LOOP_BODY]], label %[[BAR_DISPATCH_INC:.+]], !llvm.loop [[BAR_LOOP_MD:![0-9]+]]
;
; CHECK: [[BAR_DISPATCH_INC]]:
; CHECK:   br label %[[BAR_DISPATCH_HEADER]]

; CHECK: [[BAR_LOOP_EXIT]]:
; CHECK:   call void @__kmpc_for_static_fini(%struct.ident_t* {{.+}}, i32 %{{.+}})
; CHECK: }
;
; CHECK-DAG: [[FOO_LOOP_CM:![0-9]+]] = !{!"llvm.loop.intel.loopcount_maximum", i32 1000}
; CHECK-DAG: [[FOO_LOOP_MD]] = distinct !{[[FOO_LOOP_MD]]{{.*}}, [[FOO_LOOP_CM]]{{[,\}]}}
;
; CHECK-DAG: [[BAR_LOOP_CM:![0-9]+]] = !{!"llvm.loop.intel.loopcount_maximum", i32 25}
; CHECK-DAG: [[BAR_LOOP_CA:![0-9]+]] = !{!"llvm.loop.intel.loopcount_average", i32 25}
; CHECK-DAG: [[BAR_LOOP_MD]] = distinct !{[[BAR_LOOP_MD]]{{.*}}, [[BAR_LOOP_CM]], [[BAR_LOOP_CA]]{{[,\}]}}

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

define dso_local void @bar(i32* %P) {
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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 25), "QUAL.OMP.SHARED"(i32** %P.addr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %I) ]
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
