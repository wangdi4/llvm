; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
;
; This test checks that paropt transform pass adds range metadata to OpenMP API
; call with known result range.
;
; void test(int N, int B) {
;   omp_get_num_teams();             // [1, INTMAX)
;   omp_get_team_num();              // [0, INTMAX)
;   omp_get_num_threads();           // [1, INTMAX)
;   omp_get_thread_num();            // [0, INTMAX)
;
; #pragma omp teams distribute num_teams(400) thread_limit(300)
;   for (int I = 0; I < N; I += B) {
;     omp_get_num_teams();           // [1, 401)
;     omp_get_team_num();            // [0, 400)
;     omp_get_num_threads();         // [1, 301)
;     omp_get_thread_num();          // [0, 300)
;
; #pragma omp parallel for num_threads(200)
;     for (int J = I; J < I + B; ++J) {
;       omp_get_num_teams();         // [1, 401)
;       omp_get_team_num();          // [0, 400)
;       omp_get_num_threads();       // [1, 201)
;       omp_get_thread_num();        // [0, 200)
;
; #pragma omp target
;       {
;         omp_get_num_teams();       // [1, INTMAX)
;         omp_get_team_num();        // [0, INTMAX)
;         omp_get_num_threads();     // [1, INTMAX)
;         omp_get_thread_num();      // [0, INTMAX)
;
; #pragma omp parallel num_threads(100)
;         {
;           omp_get_num_teams();    // [1, INTMAX)
;           omp_get_team_num();     // [0, INTMAX)
;           omp_get_num_threads();  // [1, 101)
;           omp_get_thread_num();   // [0, 100)
;         }
;       }
;     }
;   }
; }
;
; CHECK: define void @test(i32 %N, i32 %B) {
; CHECK:   call i32 @omp_get_num_teams(), !range [[RANGE_1_MAX:![0-9]+]]
; CHECK:   call i32 @omp_get_team_num(), !range [[RANGE_0_MAX:![0-9]+]]
; CHECK:   call i32 @omp_get_num_threads(), !range [[RANGE_1_MAX]]
; CHECK:   call i32 @omp_get_thread_num(), !range [[RANGE_0_MAX]]
; CHECK:   call void {{.*}}@__kmpc_fork_teams({{.+}}, i32 2, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32*, i32*)* [[TEAMS:@.+]] to void (i32*, i32*, ...)*),
; CHECK: }
;
; CHECK: define internal void [[TARGET_PARALLEL:@.+]](i32* %{{.+}}, i32* %{{.+}}) #{{[0-9]+}} {
; CHECK:   call i32 @omp_get_num_teams(), !range [[RANGE_1_MAX]]
; CHECK:   call i32 @omp_get_team_num(), !range [[RANGE_0_MAX]]
; CHECK:   call i32 @omp_get_num_threads(), !range [[RANGE_1_101:![0-9]+]]
; CHECK:   call i32 @omp_get_thread_num(), !range [[RANGE_0_100:![0-9]+]]
; CHECK: }
;
; CHECK: define internal void [[TARGET:@__omp_offloading_.+]]() #{{[0-9]+}} {
; CHECK:   call i32 @omp_get_num_teams(), !range [[RANGE_1_MAX]]
; CHECK:   call i32 @omp_get_team_num(), !range [[RANGE_0_MAX]]
; CHECK:   call i32 @omp_get_num_threads(), !range [[RANGE_1_MAX]]
; CHECK:   call i32 @omp_get_thread_num(), !range [[RANGE_0_MAX]]
; CHECK:   call void {{.*}}@__kmpc_fork_call({{.+}}, i32 0, void (i32*, i32*, ...)* bitcast (void (i32*, i32*)* [[TARGET_PARALLEL]] to void (i32*, i32*, ...)*))
; CHECK: }

; CHECK: define internal void [[PARALLEL_FOR:@.+]](i32* %{{.+}}, i32* %{{.+}}, i32* %{{.+}}, i64 %{{.+}}, i32* %{{.+}}) #{{[0-9]+}} {
; CHECK:   call i32 @omp_get_num_teams(), !range [[RANGE_1_401:![0-9]+]]
; CHECK:   call i32 @omp_get_team_num(), !range [[RANGE_0_400:![0-9]+]]
; CHECK:   call i32 @omp_get_num_threads(), !range [[RANGE_1_201:![0-9]+]]
; CHECK:   call i32 @omp_get_thread_num(), !range [[RANGE_0_200:![0-9]+]]
; CHECK:   call i32 @__tgt_target({{.+}}, i8* [[TARGET]].region_id,
; CHECK: }
;
; CHECK: define internal void [[TEAMS]](i32* %{{.+}}, i32* %{{.+}}, i32* %{{.+}}, i32* %{{.+}}) #{{[0-9]+}} {
; CHECK:   call i32 @omp_get_num_teams(), !range [[RANGE_1_401]]
; CHECK:   call i32 @omp_get_team_num(), !range [[RANGE_0_400]]
; CHECK:   call i32 @omp_get_num_threads(), !range [[RANGE_1_301:![0-9]+]]
; CHECK:   call i32 @omp_get_thread_num(), !range [[RANGE_0_300:![0-9]+]]
; CHECK:   call void {{.+}}@__kmpc_fork_call({{.+}}, i32 3, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i32*, i64, i32*)* [[PARALLEL_FOR]] to void (i32*, i32*, ...)*),
; CHECK: }
;
; CHECK-DAG: [[RANGE_1_MAX]] = !{i32 1, i32 2147483647}
; CHECK-DAG: [[RANGE_0_MAX]] = !{i32 0, i32 2147483647}
; CHECK-DAG: [[RANGE_1_101]] = !{i32 1, i32 101}
; CHECK-DAG: [[RANGE_0_100]] = !{i32 0, i32 100}
; CHECK-DAG: [[RANGE_1_201]] = !{i32 1, i32 201}
; CHECK-DAG: [[RANGE_0_200]] = !{i32 0, i32 200}
; CHECK-DAG: [[RANGE_1_301]] = !{i32 1, i32 301}
; CHECK-DAG: [[RANGE_0_300]] = !{i32 0, i32 300}
; CHECK-DAG: [[RANGE_1_401]] = !{i32 1, i32 401}
; CHECK-DAG: [[RANGE_0_400]] = !{i32 0, i32 400}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

define void @test(i32 %N, i32 %B) {
entry:
  %N.addr = alloca i32, align 4
  %B.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.capture_expr.4 = alloca i32, align 4
  %.capture_expr.5 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  %tmp12 = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.omp.iv21 = alloca i32, align 4
  %.omp.lb22 = alloca i32, align 4
  %.omp.ub23 = alloca i32, align 4
  %J = alloca i32, align 4
  store i32 %N, i32* %N.addr, align 4
  store i32 %B, i32* %B.addr, align 4
  %call = call i32 @omp_get_num_teams()
  %call1 = call i32 @omp_get_team_num()
  %call2 = call i32 @omp_get_num_threads()
  %call3 = call i32 @omp_get_thread_num()
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.NUM_TEAMS"(i32 400),
    "QUAL.OMP.THREAD_LIMIT"(i32 300),
    "QUAL.OMP.SHARED"(i32* %N.addr),
    "QUAL.OMP.SHARED"(i32* %B.addr),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.5),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %I),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.3),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.4),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.2),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv21),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb22),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub23),
    "QUAL.OMP.PRIVATE"(i32* %J),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1),
    "QUAL.OMP.PRIVATE"(i32* %tmp),
    "QUAL.OMP.PRIVATE"(i32* %tmp12) ]

  %1 = load i32, i32* %N.addr, align 4
  store i32 %1, i32* %.capture_expr.3, align 4
  %2 = load i32, i32* %B.addr, align 4
  store i32 %2, i32* %.capture_expr.4, align 4
  %3 = load i32, i32* %.capture_expr.3, align 4
  %4 = load i32, i32* %.capture_expr.4, align 4
  %sub = sub nsw i32 0, %4
  %add = add nsw i32 %sub, 1
  %sub4 = sub nsw i32 %3, %add
  %5 = load i32, i32* %.capture_expr.4, align 4
  %div = sdiv i32 %sub4, %5
  %sub5 = sub nsw i32 %div, 1
  store i32 %sub5, i32* %.capture_expr.5, align 4
  %6 = load i32, i32* %.capture_expr.3, align 4
  %cmp = icmp slt i32 0, %6
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end48

omp.precond.then:                                 ; preds = %entry
  store i32 0, i32* %.omp.lb, align 4
  %7 = load i32, i32* %.capture_expr.5, align 4
  store i32 %7, i32* %.omp.ub, align 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %I),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.2),
    "QUAL.OMP.PRIVATE"(i32* %.omp.iv21),
    "QUAL.OMP.PRIVATE"(i32* %.omp.lb22),
    "QUAL.OMP.PRIVATE"(i32* %.omp.ub23),
    "QUAL.OMP.PRIVATE"(i32* %J),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0),
    "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1),
    "QUAL.OMP.PRIVATE"(i32* %tmp12) ]

  %9 = load i32, i32* %.omp.lb, align 4
  store i32 %9, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc44, %omp.precond.then
  %10 = load i32, i32* %.omp.iv, align 4
  %11 = load i32, i32* %.omp.ub, align 4
  %cmp6 = icmp sle i32 %10, %11
  br i1 %cmp6, label %omp.inner.for.body, label %omp.inner.for.end46

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, i32* %.omp.iv, align 4
  %13 = load i32, i32* %.capture_expr.4, align 4
  %mul = mul nsw i32 %12, %13
  %add7 = add nsw i32 0, %mul
  store i32 %add7, i32* %I, align 4
  %call8 = call i32 @omp_get_num_teams()
  %call9 = call i32 @omp_get_team_num()
  %call10 = call i32 @omp_get_num_threads()
  %call11 = call i32 @omp_get_thread_num()
  %14 = load i32, i32* %I, align 4
  store i32 %14, i32* %.capture_expr.0, align 4
  %15 = load i32, i32* %I, align 4
  %16 = load i32, i32* %B.addr, align 4
  %add13 = add nsw i32 %15, %16
  store i32 %add13, i32* %.capture_expr.1, align 4
  %17 = load i32, i32* %.capture_expr.1, align 4
  %18 = load i32, i32* %.capture_expr.0, align 4
  %sub14 = sub i32 %17, %18
  %sub15 = sub i32 %sub14, 1
  %add16 = add i32 %sub15, 1
  %div17 = udiv i32 %add16, 1
  %sub18 = sub i32 %div17, 1
  store i32 %sub18, i32* %.capture_expr.2, align 4
  %19 = load i32, i32* %.capture_expr.0, align 4
  %20 = load i32, i32* %.capture_expr.1, align 4
  %cmp19 = icmp slt i32 %19, %20
  br i1 %cmp19, label %omp.precond.then20, label %omp.precond.end

omp.precond.then20:                               ; preds = %omp.inner.for.body
  store i32 0, i32* %.omp.lb22, align 4
  %21 = load i32, i32* %.capture_expr.2, align 4
  store i32 %21, i32* %.omp.ub23, align 4
  %22 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.NUM_THREADS"(i32 200),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv21),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb22),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub23),
    "QUAL.OMP.PRIVATE"(i32* %J),
    "QUAL.OMP.SHARED"(i32* %.capture_expr.0) ]

  %23 = load i32, i32* %.omp.lb22, align 4
  store i32 %23, i32* %.omp.iv21, align 4
  br label %omp.inner.for.cond24

omp.inner.for.cond24:                             ; preds = %omp.inner.for.inc, %omp.precond.then20
  %24 = load i32, i32* %.omp.iv21, align 4
  %25 = load i32, i32* %.omp.ub23, align 4
  %add25 = add i32 %25, 1
  %cmp26 = icmp ult i32 %24, %add25
  br i1 %cmp26, label %omp.inner.for.body27, label %omp.inner.for.end

omp.inner.for.body27:                             ; preds = %omp.inner.for.cond24
  %26 = load i32, i32* %.capture_expr.0, align 4
  %27 = load i32, i32* %.omp.iv21, align 4
  %mul28 = mul i32 %27, 1
  %add29 = add i32 %26, %mul28
  store i32 %add29, i32* %J, align 4
  %call30 = call i32 @omp_get_num_teams()
  %call31 = call i32 @omp_get_team_num()
  %call32 = call i32 @omp_get_num_threads()
  %call33 = call i32 @omp_get_thread_num()
  %28 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]

  %call34 = call i32 @omp_get_num_teams()
  %call35 = call i32 @omp_get_team_num()
  %call36 = call i32 @omp_get_num_threads()
  %call37 = call i32 @omp_get_thread_num()
  %29 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.NUM_THREADS"(i32 100) ]

  %call38 = call i32 @omp_get_num_teams()
  %call39 = call i32 @omp_get_team_num()
  %call40 = call i32 @omp_get_num_threads()
  %call41 = call i32 @omp_get_thread_num()
  call void @llvm.directive.region.exit(token %29) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %28) [ "DIR.OMP.END.TARGET"() ]
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body27
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %30 = load i32, i32* %.omp.iv21, align 4
  %add42 = add nuw i32 %30, 1
  store i32 %add42, i32* %.omp.iv21, align 4
  br label %omp.inner.for.cond24

omp.inner.for.end:                                ; preds = %omp.inner.for.cond24
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %22) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %omp.inner.for.body
  br label %omp.body.continue43

omp.body.continue43:                              ; preds = %omp.precond.end
  br label %omp.inner.for.inc44

omp.inner.for.inc44:                              ; preds = %omp.body.continue43
  %31 = load i32, i32* %.omp.iv, align 4
  %add45 = add nsw i32 %31, 1
  store i32 %add45, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end46:                              ; preds = %omp.inner.for.cond
  br label %omp.loop.exit47

omp.loop.exit47:                                  ; preds = %omp.inner.for.end46
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.DISTRIBUTE"() ]
  br label %omp.precond.end48

omp.precond.end48:                                ; preds = %omp.loop.exit47, %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]
  ret void
}

declare dso_local i32 @omp_get_num_teams()
declare dso_local i32 @omp_get_team_num()
declare dso_local i32 @omp_get_num_threads()
declare dso_local i32 @omp_get_thread_num()
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 51, i32 -694787535, !"_Z4test", i32 23, i32 0, i32 0}
