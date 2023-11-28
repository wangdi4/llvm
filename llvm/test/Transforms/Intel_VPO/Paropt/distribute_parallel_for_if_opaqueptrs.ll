; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -S <%s | FileCheck %s
; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt" -S <%s | FileCheck %s

; // C++ source
; #include <stdio.h>
; #include <omp.h>
; int cond;
; void bar(int);
;
; void foo1() {
;    #pragma omp teams
;    #pragma omp distribute parallel for if(cond)
;    for (int i = 0; i < 10; i++) {
;      bar(i);
;      // printf("team %d   thread %d   i = %d\n", omp_get_team_num(), omp_get_thread_num(), i);
;    }
; }
;
; void foo2_num_threads() {
;    #pragma omp teams
;    #pragma omp distribute parallel for if(cond) num_threads(4)
;    for (int i = 0; i < 10; i++) {
;      bar(i);
;      // printf("team %d   thread %d   i = %d\n", omp_get_team_num(), omp_get_thread_num(), i);
;    }
; }
;
; For DISTRIBUTE PARALLEL FOR IF(cond) we do not optimize the IF(false) path
; by calling the outlined function directly.
; CHECK-NOT: call void @__kmpc_serialized_parallel
;
; Instead, we set the number of threads to 1 in the IF(false) path by calling
; __kmpc_push_num_threads(LOC, TID, 1).
; If there is also a num_threads(N) clause then it is set in the IF(true) path.
;
; 1. foo1(): case without NUM_THREADS clause
;
; CHECK-LABEL: define {{.*}} @_Z4foo1v.DIR.OMP.TEAMS
; CHECK:         br i1 %tobool, label %if.then, label %if.else
;
; CHECK-LABEL: if.then:
; CHECK-NEXT:    br label %if.end
;
; CHECK-LABEL: if.else:
; CHECK-NEXT:    call void @__kmpc_push_num_threads(ptr {{.*}}, i32 %my.tid, i32 1)
; CHECK-NEXT:    br label %if.end
;
; CHECK-LABEL: if.end:
; CHECK-NEXT:    call void {{.*}} @__kmpc_fork_call
;
;
; 2. foo2_num_threads(): case with NUM_THREADS(4) clause
;
; CHECK-LABEL: define {{.*}} @_Z16foo2_num_threadsv.DIR.OMP.TEAMS
; CHECK:         br i1 %tobool, label %if.then, label %if.else
;
; CHECK-LABEL: if.then:
; CHECK-NEXT:    call void @__kmpc_push_num_threads(ptr {{.*}}, i32 %my.tid, i32 4)
; CHECK-NEXT:    br label %if.end
;
; CHECK-LABEL: if.else:
; CHECK-NEXT:    call void @__kmpc_push_num_threads(ptr {{.*}}, i32 %my.tid, i32 1)
; CHECK-NEXT:    br label %if.end
;
; CHECK-LABEL: if.end:
; CHECK-NEXT:    call void {{.*}} @__kmpc_fork_call

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@cond = dso_local global i32 0, align 4

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z4foo1v() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @cond, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %1 = load i32, ptr @cond, align 4
  %tobool = icmp ne i32 %1, 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.IF"(i1 %tobool),
    "QUAL.OMP.SHARED:TYPED"(ptr @cond, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %3 = load i32, ptr %.omp.lb, align 4
  store i32 %3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %7 = load i32, ptr %i, align 4
  call void @_Z3bari(i32 noundef %7)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

declare dso_local void @_Z3bari(i32 noundef)

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z16foo2_num_threadsv() {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @cond, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.iv, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %.omp.ub, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %tmp, i32 0, i32 1) ]

  store i32 0, ptr %.omp.lb, align 4
  store i32 9, ptr %.omp.ub, align 4
  %1 = load i32, ptr @cond, align 4
  %tobool = icmp ne i32 %1, 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.IF"(i1 %tobool),
    "QUAL.OMP.NUM_THREADS"(i32 4),
    "QUAL.OMP.SHARED:TYPED"(ptr @cond, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %3 = load i32, ptr %.omp.lb, align 4
  store i32 %3, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, ptr %.omp.iv, align 4
  %5 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %7 = load i32, ptr %i, align 4
  call void @_Z3bari(i32 noundef %7)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TEAMS"() ]

  ret void
}

