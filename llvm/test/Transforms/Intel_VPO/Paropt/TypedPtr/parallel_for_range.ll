; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck %s
;
; Check that paropt pass adds range metadata to the loop's LB/UB with signed IV
; that specifies positive range. With known loop bounds range's upper value
; is the maximum TC, otherwise it is the maximum signed value of the IV's type.
;
; void foo(int *P) {
; #pragma omp parallel for
;   for (int I = -2500; I < 2500; I += 5)
;     P[I] = I;
; }
;
; void bar(int *P, int N) {
; #pragma omp parallel for
;   for (int I = 0; I < N; ++I)
;     P[I] = I;
; }
;
; void goo(int *P) {
; #pragma omp parallel for schedule(static, 10)
;   for (int I = -1500; I < 1500; I += 5)
;     P[I] = I;
; }
;
; void baz(int *P) {
; #pragma omp parallel for
;   for (unsigned I = 2500; I < 5000; I += 5)
;     P[I] = I;
; }
;
; CHECK: define dso_local void @foo(i32* %P) {
; CHECK: call{{.+}}  @__kmpc_fork_call(%struct.ident_t* {{.+}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32**, i64, i32*)* [[FOO_OUTLINED:@.+]] to void (i32*, i32*, ...)*), 
; CHECK: }

; CHECK: define dso_local void @bar(i32* %P, i32 %N) {
; CHECK: call{{.+}}  @__kmpc_fork_call(%struct.ident_t* {{.+}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32**, i64, i32*)* [[BAR_OUTLINED:@.+]] to void (i32*, i32*, ...)*), 
; CHECK: }

; CHECK: define dso_local void @goo(i32* %P) {
; CHECK: call{{.+}}  @__kmpc_fork_call(%struct.ident_t* {{.+}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32**, i64, i32*)* [[GOO_OUTLINED:@.+]] to void (i32*, i32*, ...)*), 
; CHECK: }

; CHECK: define dso_local void @baz(i32* %P) {
; CHECK: call{{.+}}  @__kmpc_fork_call(%struct.ident_t* {{.+}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32**, i64, i32*)* [[BAZ_OUTLINED:@.+]] to void (i32*, i32*, ...)*),
; CHECK: }

; CHECK: define internal void [[FOO_OUTLINED]](i32* %{{.+}}, i32* %{{.+}}, i32** %{{.+}}, i64 %{{.+}}, i32* %{{.+}}) #{{[0-9]+}} {
; CHECK:   call void @__kmpc_for_static_init_4(%struct.ident_t* {{.+}}, i32 %{{.+}}, i32 34, i32* %{{.+}}, i32* [[FOO_LBA:%.+]], i32* [[FOO_UBA:%.+]], i32* %{{.+}}, i32 1, i32 1)
; CHECK:   %{{.+}} = load i32, i32* [[FOO_LBA]], align 4, !range [[FOO_RANGE:![0-9]+]]
; CHECK:   %{{.+}} = load i32, i32* [[FOO_UBA]], align 4, !range [[FOO_RANGE]]
; CHECK: }

; CHECK: define internal void [[BAR_OUTLINED]](i32* %{{.+}}, i32* %{{.+}}, i32** %{{.+}}, i64 %{{.+}}, i32* %{{.+}}) #{{[0-9]+}} {
; CHECK:   call void @__kmpc_for_static_init_4(%struct.ident_t* {{.+}}, i32 %{{.+}}, i32 34, i32* %{{.+}}, i32* [[BAR_LBA:%.+]], i32* [[BAR_UBA:%.+]], i32* %{{.+}}, i32 1, i32 1)
; CHECK:   %{{.+}} = load i32, i32* [[BAR_LBA]], align 4, !range [[BAR_RANGE:![0-9]+]]
; CHECK:   %{{.+}} = load i32, i32* [[BAR_UBA]], align 4, !range [[BAR_RANGE]]
; CHECK: }

; CHECK: define internal void [[GOO_OUTLINED]](i32* %{{.+}}, i32* %{{.+}}, i32** %{{.+}}, i64 %{{.+}}, i32* %{{.+}}) #{{[0-9]+}} {
; CHECK:   call void @__kmpc_for_static_init_4(%struct.ident_t* {{.+}}, i32 %{{.+}}, i32 33, i32* %{{.+}}, i32* [[GOO_LBA:%.+]], i32* [[GOO_UBA:%.+]], i32* %{{.+}}, i32 1, i32 10)
; CHECK:   %{{.+}} = load i32, i32* [[GOO_LBA]], align 4, !range [[GOO_RANGE:![0-9]+]]
; CHECK:   %{{.+}} = load i32, i32* [[GOO_UBA]], align 4, !range [[GOO_RANGE]]
; CHECK: }

; CHECK: define internal void [[BAZ_OUTLINED]](i32* %{{.+}}, i32* %{{.+}}, i32** %{{.+}}, i64 %{{.+}}, i32* %{{.+}}) #{{[0-9]+}} {
; CHECK:   call void @__kmpc_for_static_init_4u(%struct.ident_t* {{.+}}, i32 %{{.+}}, i32 34, i32* %{{.+}}, i32* [[BAZ_LBA:%.+]], i32* [[BAZ_UBA:%.+]], i32* %{{.+}}, i32 1, i32 1)
; CHECK:   %{{.+}} = load i32, i32* [[BAZ_LBA]], align 4, !range [[BAZ_RANGE:![0-9]+]]
; CHECK:   %{{.+}} = load i32, i32* [[BAZ_UBA]], align 4, !range [[BAZ_RANGE]]
; CHECK: }

; CHECK: [[FOO_RANGE]] = !{i32 0, i32 1001}
; CHECK: [[BAR_RANGE]] = !{i32 0, i32 2147483647}
; CHECK: [[GOO_RANGE]] = !{i32 0, i32 601}
; CHECK: [[BAZ_RANGE]] = !{i32 0, i32 501}

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

define dso_local void @bar(i32* %P, i32 %N) {
entry:
  %P.addr = alloca i32*, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32* %P, i32** %P.addr, align 8
  store i32 %N, i32* %N.addr, align 4
  %0 = load i32, i32* %N.addr, align 4
  store i32 %0, i32* %.capture_expr.0, align 4
  %1 = load i32, i32* %.capture_expr.0, align 4
  %sub = sub nsw i32 %1, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4
  %2 = load i32, i32* %.capture_expr.0, align 4
  %cmp = icmp slt i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4
  %3 = load i32, i32* %.capture_expr.1, align 4
  store i32 %3, i32* %.omp.ub, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(i32** %P.addr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %I) ]
  %5 = load i32, i32* %.omp.lb, align 4
  store i32 %5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %6 = load i32, i32* %.omp.iv, align 4
  %7 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %6, %7
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add4 = add nsw i32 0, %mul
  store i32 %add4, i32* %I, align 4
  %9 = load i32, i32* %I, align 4
  %10 = load i32*, i32** %P.addr, align 8
  %11 = load i32, i32* %I, align 4
  %idxprom = sext i32 %11 to i64
  %ptridx = getelementptr inbounds i32, i32* %10, i64 %idxprom
  store i32 %9, i32* %ptridx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, i32* %.omp.iv, align 4
  %add5 = add nsw i32 %12, 1
  store i32 %add5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

define dso_local void @goo(i32* %P) {
entry:
  %P.addr = alloca i32*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32* %P, i32** %P.addr, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 599, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 10), "QUAL.OMP.SHARED"(i32** %P.addr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %I) ]
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
  %add = add nsw i32 -1500, %mul
  store i32 %add, i32* %I, align 4
  %5 = load i32, i32* %I, align 4
  %6 = load i32*, i32** %P.addr, align 8
  %7 = load i32, i32* %I, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds i32, i32* %6, i64 %idxprom
  store i32 %5, i32* %arrayidx, align 4
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

define dso_local void @baz(i32* %P) {
entry:
  %P.addr = alloca i32*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32* %P, i32** %P.addr, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 499, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(i32** %P.addr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %I) ]
  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %add = add i32 %3, 1
  %cmp = icmp ult i32 %2, %add
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul i32 %4, 5
  %add1 = add i32 2500, %mul
  store i32 %add1, i32* %I, align 4
  %5 = load i32, i32* %I, align 4
  %6 = load i32*, i32** %P.addr, align 8
  %7 = load i32, i32* %I, align 4
  %idxprom = zext i32 %7 to i64
  %arrayidx = getelementptr inbounds i32, i32* %6, i64 %idxprom
  store i32 %5, i32* %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32* %.omp.iv, align 4
  %add2 = add nuw i32 %8, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)
