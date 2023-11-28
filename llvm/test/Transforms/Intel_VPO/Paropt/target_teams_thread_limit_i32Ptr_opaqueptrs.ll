; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s --check-prefixes=HST,ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s --check-prefixes=HST,ALL
;
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S %s | FileCheck %s --check-prefixes=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s --check-prefixes=ALL
;
; Check that paropt nullify the thread_limit clause value before generating the outlined function so that it doesn't become a live-in in the outlined function.
;
; Test src:
;
;void foo()
;{
;  int n = 4, m = 2;
;  #pragma omp target teams thread_limit(n)
;  { }
;  #pragma omp target teams loop thread_limit(n)
;  for(int i = 0; i < 1024;  i++)
;  { }
;  #pragma omp target teams distribute thread_limit(n)
;  for(int i = 0; i < 1024;  i++)
;  { }
;  #pragma omp target teams distribute simd thread_limit(n)
;  for(int i = 0; i < 1024;  i++)
;  { }
;  #pragma omp target teams distribute parallel for thread_limit(n)
;  for(int i = 0; i < 1024;  i++)
;  { }
;  #pragma omp target teams distribute parallel for simd thread_limit(n)
;  for(int i = 0; i < 1024;  i++)
;  { }
;  #pragma omp target thread_limit(m)
;  #pragma omp teams thread_limit(n)
;  { }
;}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

; HST: [[THREADLIMIT0:%[0-9]*]] = load i32, ptr %.thread_limit.0, align 4
; HST: call i32 @__tgt_target_teams(i64 %{{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 {{.*}}, ptr null, ptr null, ptr null, ptr null, i32 0, i32 [[THREADLIMIT0]])

; HST: [[THREADLIMIT1:%[0-9]*]] = load i32, ptr %.thread_limit.1, align 4
; HST: call i32 @__tgt_target_teams(i64 %{{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 {{.*}}, ptr null, ptr null, ptr null, ptr null, i32 0, i32 [[THREADLIMIT1]])

; HST: [[THREADLIMIT2:%[0-9]*]] = load i32, ptr %.thread_limit.2, align 4
; HST: call i32 @__tgt_target_teams(i64 %{{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 {{.*}}, ptr null, ptr null, ptr null, ptr null, i32 0, i32 [[THREADLIMIT2]])

; HST: [[THREADLIMIT3:%[0-9]*]] = load i32, ptr %.thread_limit.3, align 4
; HST: call i32 @__tgt_target_teams(i64 %{{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 {{.*}}, ptr null, ptr null, ptr null, ptr null, i32 0, i32 [[THREADLIMIT3]])

; HST: [[THREADLIMIT4:%[0-9]*]] = load i32, ptr %.thread_limit.4, align 4
; HST: call i32 @__tgt_target_teams(i64 %{{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 {{.*}}, ptr null, ptr null, ptr null, ptr null, i32 0, i32 [[THREADLIMIT4]])

; HST: [[THREADLIMIT5:%[0-9]*]] = load i32, ptr %.thread_limit.5, align 4
; HST: call i32 @__tgt_target_teams(i64 %{{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 {{.*}}, ptr null, ptr null, ptr null, ptr null, i32 0, i32 [[THREADLIMIT5]])

; HST: [[THREADLIMIT6:%[0-9]*]] = load i32, ptr %.thread_limit.6, align 4
; HST: call i32 @__tgt_target_teams(i64 %{{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 {{.*}}, ptr {{.*}}, ptr {{.*}}, ptr {{.*}}, ptr {{.*}}, i32 0, i32 [[THREADLIMIT6]])

; ALL: define internal void @foo.DIR.OMP.TEAMS{{.*}}(ptr %tid, ptr %bid)
; ALL: define internal void @foo.DIR.OMP.TEAMS{{.*}}(ptr %tid, ptr %bid)
; ALL: define internal void @foo.DIR.OMP.TEAMS{{.*}}(ptr %tid, ptr %bid)
; ALL: define internal void @foo.DIR.OMP.TEAMS{{.*}}(ptr %tid, ptr %bid)
; ALL: define internal void @foo.DIR.OMP.DISTRIBUTE.PARLOOP{{.*}}(ptr %tid, ptr %bid, i64 %.omp.lb{{.*}}, ptr %.omp.ub{{.*}})
; ALL: define internal void @foo.DIR.OMP.TEAMS{{.*}}(ptr %tid, ptr %bid)
; ALL: define internal void @foo.DIR.OMP.DISTRIBUTE.PARLOOP{{.*}}(ptr %tid, ptr %bid, i64 %.omp.lb{{.*}}, ptr %.omp.ub{{.*}})
; ALL: define internal void @foo.DIR.OMP.TEAMS{{.*}}(ptr %tid, ptr %bid)
; ALL: define internal void @foo.DIR.OMP.TEAMS{{.*}}(ptr %tid, ptr %bid)

define dso_local void @foo() {
entry:
  %n = alloca i32, align 4
  %m = alloca i32, align 4
  %.thread_limit.0 = alloca i32, align 4
  %.thread_limit.1 = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %.thread_limit.2 = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %i9 = alloca i32, align 4
  %.thread_limit.3 = alloca i32, align 4
  %tmp17 = alloca i32, align 4
  %.omp.iv18 = alloca i32, align 4
  %.omp.lb19 = alloca i32, align 4
  %.omp.ub20 = alloca i32, align 4
  %i24 = alloca i32, align 4
  %.thread_limit.4 = alloca i32, align 4
  %tmp32 = alloca i32, align 4
  %.omp.iv33 = alloca i32, align 4
  %.omp.lb34 = alloca i32, align 4
  %.omp.ub35 = alloca i32, align 4
  %i39 = alloca i32, align 4
  %.thread_limit.5 = alloca i32, align 4
  %tmp47 = alloca i32, align 4
  %.omp.iv48 = alloca i32, align 4
  %.omp.lb49 = alloca i32, align 4
  %.omp.ub50 = alloca i32, align 4
  %i54 = alloca i32, align 4
  %.thread_limit.6 = alloca i32, align 4
  store i32 4, ptr %n, align 4
  store i32 2, ptr %m, align 4
  %0 = load i32, ptr %n, align 4
  store i32 %0, ptr %.thread_limit.0, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %.thread_limit.0, i32 0) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]

  %3 = load i32, ptr %n, align 4
  store i32 %3, ptr %.thread_limit.1, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %.thread_limit.1, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 1023, ptr %.omp.ub, align 4
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %7 = load i32, ptr %.omp.lb, align 4
  store i32 %7, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %8 = load i32, ptr %.omp.iv, align 4
  %9 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %8, %9
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %10, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %11 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %11, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TARGET"() ]

  %12 = load i32, ptr %n, align 4
  store i32 %12, ptr %.thread_limit.2, align 4
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %.thread_limit.2, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i9, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1) ]

  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i9, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb4, align 4
  store i32 1023, ptr %.omp.ub5, align 4
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv3, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub5, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i9, i32 0, i32 1) ]

  %16 = load i32, ptr %.omp.lb4, align 4
  store i32 %16, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc13, %omp.loop.exit
  %17 = load i32, ptr %.omp.iv3, align 4
  %18 = load i32, ptr %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %17, %18
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end15

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %19 = load i32, ptr %.omp.iv3, align 4
  %mul10 = mul nsw i32 %19, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, ptr %i9, align 4
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %20 = load i32, ptr %.omp.iv3, align 4
  %add14 = add nsw i32 %20, 1
  store i32 %add14, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.DISTRIBUTE"() ]

  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.TARGET"() ]

  %21 = load i32, ptr %n, align 4
  store i32 %21, ptr %.thread_limit.3, align 4
  %22 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %.thread_limit.3, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv18, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb19, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub20, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i24, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp17, i32 0, i32 1) ]

  %23 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv18, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb19, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub20, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i24, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp17, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb19, align 4
  store i32 1023, ptr %.omp.ub20, align 4
  %24 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv18, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb19, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub20, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i24, i32 0, i32 1) ]

  %25 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i24, i32 0, i32 1, i32 1) ]

  %26 = load i32, ptr %.omp.lb19, align 4
  store i32 %26, ptr %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.cond21:                             ; preds = %omp.inner.for.inc28, %omp.loop.exit16
  %27 = load i32, ptr %.omp.iv18, align 4
  %28 = load i32, ptr %.omp.ub20, align 4
  %cmp22 = icmp sle i32 %27, %28
  br i1 %cmp22, label %omp.inner.for.body23, label %omp.inner.for.end30

omp.inner.for.body23:                             ; preds = %omp.inner.for.cond21
  %29 = load i32, ptr %.omp.iv18, align 4
  %mul25 = mul nsw i32 %29, 1
  %add26 = add nsw i32 0, %mul25
  store i32 %add26, ptr %i24, align 4
  br label %omp.body.continue27

omp.body.continue27:                              ; preds = %omp.inner.for.body23
  br label %omp.inner.for.inc28

omp.inner.for.inc28:                              ; preds = %omp.body.continue27
  %30 = load i32, ptr %.omp.iv18, align 4
  %add29 = add nsw i32 %30, 1
  store i32 %add29, ptr %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.end30:                              ; preds = %omp.inner.for.cond21
  br label %omp.loop.exit31

omp.loop.exit31:                                  ; preds = %omp.inner.for.end30
  call void @llvm.directive.region.exit(token %25) [ "DIR.OMP.END.SIMD"() ]

  call void @llvm.directive.region.exit(token %24) [ "DIR.OMP.END.DISTRIBUTE"() ]

  call void @llvm.directive.region.exit(token %23) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %22) [ "DIR.OMP.END.TARGET"() ]

  %31 = load i32, ptr %n, align 4
  store i32 %31, ptr %.thread_limit.4, align 4
  %32 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 4),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %.thread_limit.4, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv33, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb34, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub35, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i39, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp32, i32 0, i32 1) ]

  %33 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv33, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb34, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub35, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i39, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp32, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb34, align 4
  store i32 1023, ptr %.omp.ub35, align 4
  %34 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv33, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb34, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub35, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i39, i32 0, i32 1) ]

  %35 = load i32, ptr %.omp.lb34, align 4
  store i32 %35, ptr %.omp.iv33, align 4
  br label %omp.inner.for.cond36

omp.inner.for.cond36:                             ; preds = %omp.inner.for.inc43, %omp.loop.exit31
  %36 = load i32, ptr %.omp.iv33, align 4
  %37 = load i32, ptr %.omp.ub35, align 4
  %cmp37 = icmp sle i32 %36, %37
  br i1 %cmp37, label %omp.inner.for.body38, label %omp.inner.for.end45

omp.inner.for.body38:                             ; preds = %omp.inner.for.cond36
  %38 = load i32, ptr %.omp.iv33, align 4
  %mul40 = mul nsw i32 %38, 1
  %add41 = add nsw i32 0, %mul40
  store i32 %add41, ptr %i39, align 4
  br label %omp.body.continue42

omp.body.continue42:                              ; preds = %omp.inner.for.body38
  br label %omp.inner.for.inc43

omp.inner.for.inc43:                              ; preds = %omp.body.continue42
  %39 = load i32, ptr %.omp.iv33, align 4
  %add44 = add nsw i32 %39, 1
  store i32 %add44, ptr %.omp.iv33, align 4
  br label %omp.inner.for.cond36

omp.inner.for.end45:                              ; preds = %omp.inner.for.cond36
  br label %omp.loop.exit46

omp.loop.exit46:                                  ; preds = %omp.inner.for.end45
  call void @llvm.directive.region.exit(token %34) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %33) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %32) [ "DIR.OMP.END.TARGET"() ]

  %40 = load i32, ptr %n, align 4
  store i32 %40, ptr %.thread_limit.5, align 4
  %41 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 5),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %.thread_limit.5, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv48, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb49, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub50, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i54, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp47, i32 0, i32 1) ]

  %42 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv48, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb49, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub50, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i54, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp47, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb49, align 4
  store i32 1023, ptr %.omp.ub50, align 4
  %43 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv48, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb49, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub50, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i54, i32 0, i32 1) ]

  %44 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i54, i32 0, i32 1, i32 1) ]

  %45 = load i32, ptr %.omp.lb49, align 4
  store i32 %45, ptr %.omp.iv48, align 4
  br label %omp.inner.for.cond51

omp.inner.for.cond51:                             ; preds = %omp.inner.for.inc58, %omp.loop.exit46
  %46 = load i32, ptr %.omp.iv48, align 4
  %47 = load i32, ptr %.omp.ub50, align 4
  %cmp52 = icmp sle i32 %46, %47
  br i1 %cmp52, label %omp.inner.for.body53, label %omp.inner.for.end60

omp.inner.for.body53:                             ; preds = %omp.inner.for.cond51
  %48 = load i32, ptr %.omp.iv48, align 4
  %mul55 = mul nsw i32 %48, 1
  %add56 = add nsw i32 0, %mul55
  store i32 %add56, ptr %i54, align 4
  br label %omp.body.continue57

omp.body.continue57:                              ; preds = %omp.inner.for.body53
  br label %omp.inner.for.inc58

omp.inner.for.inc58:                              ; preds = %omp.body.continue57
  %49 = load i32, ptr %.omp.iv48, align 4
  %add59 = add nsw i32 %49, 1
  store i32 %add59, ptr %.omp.iv48, align 4
  br label %omp.inner.for.cond51

omp.inner.for.end60:                              ; preds = %omp.inner.for.cond51
  br label %omp.loop.exit61

omp.loop.exit61:                                  ; preds = %omp.inner.for.end60
  call void @llvm.directive.region.exit(token %44) [ "DIR.OMP.END.SIMD"() ]

  call void @llvm.directive.region.exit(token %43) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %42) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %41) [ "DIR.OMP.END.TARGET"() ]

  %50 = load i32, ptr %m, align 4
  store i32 %50, ptr %.thread_limit.6, align 4
  %51 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 6),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %.thread_limit.6, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %n, i32 0, i32 1) ]

  %52 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.THREAD_LIMIT:TYPED"(ptr %n, i32 0) ]

  call void @llvm.directive.region.exit(token %52) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %51) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0, !1, !2, !3, !4, !5, !6}

!0 = !{i32 0, i32 66313, i32 229198024, !"_Z3foo", i32 5, i32 0, i32 0, i32 0}
!1 = !{i32 0, i32 66313, i32 229198024, !"_Z3foo", i32 7, i32 0, i32 1, i32 0}
!2 = !{i32 0, i32 66313, i32 229198024, !"_Z3foo", i32 10, i32 0, i32 2, i32 0}
!3 = !{i32 0, i32 66313, i32 229198024, !"_Z3foo", i32 13, i32 0, i32 3, i32 0}
!4 = !{i32 0, i32 66313, i32 229198024, !"_Z3foo", i32 16, i32 0, i32 4, i32 0}
!5 = !{i32 0, i32 66313, i32 229198024, !"_Z3foo", i32 19, i32 0, i32 5, i32 0}
!6 = !{i32 0, i32 66313, i32 229198024, !"_Z3foo", i32 22, i32 0, i32 6, i32 0}
