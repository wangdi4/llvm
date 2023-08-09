; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s --check-prefixes=HST,ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s --check-prefixes=HST,ALL
;
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S %s | FileCheck %s --check-prefixes=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s --check-prefixes=ALL
;
; This tests checks paropt lowering of 'omp target' construct with 'thread_limit' clause. IR was hand modified because front end does not yet support the THREAD_LIMIT clause.
;
; Test src:
;
; Check that paropt nullify the thread_limit clause value before generating the outlined function so that it doesn't become a live-in in the outlined function.

;void foo()
;{
;  #pragma omp target teams thread_limit(4)
;  { }
;  #pragma omp target teams loop thread_limit(4)
;  for(int i = 0; i < 1024;  i++)
;  { }
;  #pragma omp target teams distribute thread_limit(4)
;  for(int i = 0; i < 1024;  i++)
;  { }
;  #pragma omp target teams distribute simd thread_limit(4)
;  for(int i = 0; i < 1024;  i++)
;  { }
;  #pragma omp target teams distribute parallel for thread_limit(4)
;  for(int i = 0; i < 1024;  i++)
;  { }
;  #pragma omp target teams distribute parallel for simd thread_limit(4)
;  for(int i = 0; i < 1024;  i++)
;  { }
;  #pragma omp target thread_limit(4)
;  #pragma omp teams thread_limit(2)
;  { }
;}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

; HST: define dso_local void @foo{{.*}}
; HST: i32 @__tgt_target_teams(i64 {{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 0, ptr null, ptr null, ptr null, ptr null, i32 0, i32 4)
; HST: i32 @__tgt_target_teams(i64 {{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 0, ptr null, ptr null, ptr null, ptr null, i32 0, i32 4)
; HST: i32 @__tgt_target_teams(i64 {{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 0, ptr null, ptr null, ptr null, ptr null, i32 0, i32 4)
; HST: i32 @__tgt_target_teams(i64 {{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 0, ptr null, ptr null, ptr null, ptr null, i32 0, i32 4)
; HST: i32 @__tgt_target_teams(i64 {{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 0, ptr null, ptr null, ptr null, ptr null, i32 0, i32 4)
; HST: i32 @__tgt_target_teams(i64 {{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 0, ptr null, ptr null, ptr null, ptr null, i32 0, i32 4)
; HST: i32 @__tgt_target_teams(i64 {{.*}}, ptr @{{.*}}Z3foo{{.*}}, i32 0, ptr null, ptr null, ptr null, ptr null, i32 0, i32 4)

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
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %i9 = alloca i32, align 4
  %tmp17 = alloca i32, align 4
  %.omp.iv18 = alloca i32, align 4
  %.omp.lb19 = alloca i32, align 4
  %.omp.ub20 = alloca i32, align 4
  %i24 = alloca i32, align 4
  %tmp32 = alloca i32, align 4
  %.omp.iv33 = alloca i32, align 4
  %.omp.lb34 = alloca i32, align 4
  %.omp.ub35 = alloca i32, align 4
  %i39 = alloca i32, align 4
  %tmp47 = alloca i32, align 4
  %.omp.iv48 = alloca i32, align 4
  %.omp.lb49 = alloca i32, align 4
  %.omp.ub50 = alloca i32, align 4
  %i54 = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.THREAD_LIMIT"(i32 4) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.THREAD_LIMIT"(i32 4),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 1023, ptr %.omp.ub, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %5 = load i32, ptr %.omp.lb, align 4
  store i32 %5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, ptr %.omp.iv, align 4
  %7 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %9, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.GENERICLOOP"() ]

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2),
    "QUAL.OMP.THREAD_LIMIT"(i32 4),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i9, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1) ]

  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv3, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub5, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i9, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp2, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb4, align 4
  store i32 1023, ptr %.omp.ub5, align 4
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv3, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub5, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i9, i32 0, i32 1) ]

  %13 = load i32, ptr %.omp.lb4, align 4
  store i32 %13, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc13, %omp.loop.exit
  %14 = load i32, ptr %.omp.iv3, align 4
  %15 = load i32, ptr %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %14, %15
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end15

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %16 = load i32, ptr %.omp.iv3, align 4
  %mul10 = mul nsw i32 %16, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, ptr %i9, align 4
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %17 = load i32, ptr %.omp.iv3, align 4
  %add14 = add nsw i32 %17, 1
  store i32 %add14, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.DISTRIBUTE"() ]

  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TARGET"() ]

  %18 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3),
    "QUAL.OMP.THREAD_LIMIT"(i32 4),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv18, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb19, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub20, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i24, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp17, i32 0, i32 1) ]

  %19 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv18, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb19, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub20, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i24, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp17, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb19, align 4
  store i32 1023, ptr %.omp.ub20, align 4
  %20 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv18, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb19, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub20, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i24, i32 0, i32 1) ]

  %21 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i24, i32 0, i32 1, i32 1) ]

  %22 = load i32, ptr %.omp.lb19, align 4
  store i32 %22, ptr %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.cond21:                             ; preds = %omp.inner.for.inc28, %omp.loop.exit16
  %23 = load i32, ptr %.omp.iv18, align 4
  %24 = load i32, ptr %.omp.ub20, align 4
  %cmp22 = icmp sle i32 %23, %24
  br i1 %cmp22, label %omp.inner.for.body23, label %omp.inner.for.end30

omp.inner.for.body23:                             ; preds = %omp.inner.for.cond21
  %25 = load i32, ptr %.omp.iv18, align 4
  %mul25 = mul nsw i32 %25, 1
  %add26 = add nsw i32 0, %mul25
  store i32 %add26, ptr %i24, align 4
  br label %omp.body.continue27

omp.body.continue27:                              ; preds = %omp.inner.for.body23
  br label %omp.inner.for.inc28

omp.inner.for.inc28:                              ; preds = %omp.body.continue27
  %26 = load i32, ptr %.omp.iv18, align 4
  %add29 = add nsw i32 %26, 1
  store i32 %add29, ptr %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.end30:                              ; preds = %omp.inner.for.cond21
  br label %omp.loop.exit31

omp.loop.exit31:                                  ; preds = %omp.inner.for.end30
  call void @llvm.directive.region.exit(token %21) [ "DIR.OMP.END.SIMD"() ]

  call void @llvm.directive.region.exit(token %20) [ "DIR.OMP.END.DISTRIBUTE"() ]

  call void @llvm.directive.region.exit(token %19) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %18) [ "DIR.OMP.END.TARGET"() ]

  %27 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 4),
    "QUAL.OMP.THREAD_LIMIT"(i32 4),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv33, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb34, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub35, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i39, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp32, i32 0, i32 1) ]

  %28 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv33, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb34, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub35, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i39, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp32, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb34, align 4
  store i32 1023, ptr %.omp.ub35, align 4
  %29 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv33, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb34, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub35, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i39, i32 0, i32 1) ]

  %30 = load i32, ptr %.omp.lb34, align 4
  store i32 %30, ptr %.omp.iv33, align 4
  br label %omp.inner.for.cond36

omp.inner.for.cond36:                             ; preds = %omp.inner.for.inc43, %omp.loop.exit31
  %31 = load i32, ptr %.omp.iv33, align 4
  %32 = load i32, ptr %.omp.ub35, align 4
  %cmp37 = icmp sle i32 %31, %32
  br i1 %cmp37, label %omp.inner.for.body38, label %omp.inner.for.end45

omp.inner.for.body38:                             ; preds = %omp.inner.for.cond36
  %33 = load i32, ptr %.omp.iv33, align 4
  %mul40 = mul nsw i32 %33, 1
  %add41 = add nsw i32 0, %mul40
  store i32 %add41, ptr %i39, align 4
  br label %omp.body.continue42

omp.body.continue42:                              ; preds = %omp.inner.for.body38
  br label %omp.inner.for.inc43

omp.inner.for.inc43:                              ; preds = %omp.body.continue42
  %34 = load i32, ptr %.omp.iv33, align 4
  %add44 = add nsw i32 %34, 1
  store i32 %add44, ptr %.omp.iv33, align 4
  br label %omp.inner.for.cond36

omp.inner.for.end45:                              ; preds = %omp.inner.for.cond36
  br label %omp.loop.exit46

omp.loop.exit46:                                  ; preds = %omp.inner.for.end45
  call void @llvm.directive.region.exit(token %29) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %28) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %27) [ "DIR.OMP.END.TARGET"() ]

  %35 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 5),
    "QUAL.OMP.THREAD_LIMIT"(i32 4),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv48, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb49, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub50, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i54, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp47, i32 0, i32 1) ]

  %36 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv48, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb49, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub50, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i54, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp47, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb49, align 4
  store i32 1023, ptr %.omp.ub50, align 4
  %37 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv48, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb49, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub50, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i54, i32 0, i32 1) ]

  %38 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
    "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i54, i32 0, i32 1, i32 1) ]

  %39 = load i32, ptr %.omp.lb49, align 4
  store i32 %39, ptr %.omp.iv48, align 4
  br label %omp.inner.for.cond51

omp.inner.for.cond51:                             ; preds = %omp.inner.for.inc58, %omp.loop.exit46
  %40 = load i32, ptr %.omp.iv48, align 4
  %41 = load i32, ptr %.omp.ub50, align 4
  %cmp52 = icmp sle i32 %40, %41
  br i1 %cmp52, label %omp.inner.for.body53, label %omp.inner.for.end60

omp.inner.for.body53:                             ; preds = %omp.inner.for.cond51
  %42 = load i32, ptr %.omp.iv48, align 4
  %mul55 = mul nsw i32 %42, 1
  %add56 = add nsw i32 0, %mul55
  store i32 %add56, ptr %i54, align 4
  br label %omp.body.continue57

omp.body.continue57:                              ; preds = %omp.inner.for.body53
  br label %omp.inner.for.inc58

omp.inner.for.inc58:                              ; preds = %omp.body.continue57
  %43 = load i32, ptr %.omp.iv48, align 4
  %add59 = add nsw i32 %43, 1
  store i32 %add59, ptr %.omp.iv48, align 4
  br label %omp.inner.for.cond51

omp.inner.for.end60:                              ; preds = %omp.inner.for.cond51
  br label %omp.loop.exit61

omp.loop.exit61:                                  ; preds = %omp.inner.for.end60
  call void @llvm.directive.region.exit(token %38) [ "DIR.OMP.END.SIMD"() ]

  call void @llvm.directive.region.exit(token %37) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %36) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %35) [ "DIR.OMP.END.TARGET"() ]

  %44 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 6),
    "QUAL.OMP.THREAD_LIMIT"(i32 4) ]

  %45 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.THREAD_LIMIT"(i32 2) ]

  call void @llvm.directive.region.exit(token %45) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %44) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0, !1, !2, !3, !4, !5, !6}

!0 = !{i32 0, i32 66313, i32 229198019, !"_Z3foo", i32 4, i32 0, i32 0, i32 0}
!1 = !{i32 0, i32 66313, i32 229198019, !"_Z3foo", i32 6, i32 0, i32 1, i32 0}
!2 = !{i32 0, i32 66313, i32 229198019, !"_Z3foo", i32 9, i32 0, i32 2, i32 0}
!3 = !{i32 0, i32 66313, i32 229198019, !"_Z3foo", i32 12, i32 0, i32 3, i32 0}
!4 = !{i32 0, i32 66313, i32 229198019, !"_Z3foo", i32 15, i32 0, i32 4, i32 0}
!5 = !{i32 0, i32 66313, i32 229198019, !"_Z3foo", i32 18, i32 0, i32 5, i32 0}
!6 = !{i32 0, i32 66313, i32 229198019, !"_Z3foo", i32 21, i32 0, i32 6, i32 0}
