; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; The test checks that schedule/dist_schedule chunk value is passed
; by pointer to the outlined routines.

; Test src:
; void parallel_for_schedule(int bbb) {
; #pragma omp parallel for schedule(static, bbb)
;   for(int i = 0; i < 100; i++) {
;   }
; }
;
; void distribute_parallel_for_schedule(int bbb) {
; #pragma omp distribute parallel for dist_schedule(static, bbb) schedule(static, bbb)
;   for(int i = 0; i < 100; i++) {
;   }
; }

; Check that the chunk values in schedule/dist_schedule clauses are captured and
; passed into the kernel by address.

; CHECK: define internal void @_Z21parallel_for_schedulei.DIR.OMP.PARALLEL.LOOP.{{.*}}(ptr %tid, ptr %bid, ptr %schd.chunk.addr, ptr %.capture_expr.0, ptr %.omp.lb, ptr %.omp.ub)
; CHECK:  [[SCHD_CHUNK_VAL:%schd.chunk.*]] = load i32, ptr %schd.chunk.addr, align 4
; CHECK:  call void @__kmpc_for_static_init_4({{.*}}, i32 1, i32 [[SCHD_CHUNK_VAL]])

; CHECK: define internal void @_Z32distribute_parallel_for_schedulei.DIR.OMP.DISTRIBUTE.PARLOOP{{.*}}(ptr %tid, ptr %bid, ptr %schd.chunk.addr, ptr %dist.schd.chunk.addr, ptr %bbb.addr, ptr %.capture_expr.1, ptr %.omp.lb, ptr %.omp.ub)
; CHECK:  [[SCHD_CHUNK_VAL:%schd.chunk.*]] = load i32, ptr %schd.chunk.addr, align 4
; CHECK:  [[DIST_SCHD_CHUNK_VAL:%dist.schd.chunk.*]] = load i32, ptr %dist.schd.chunk.addr, align 4
; CHECK:  call void @__kmpc_team_static_init_4({{.*}}, i32 1, i32 [[DIST_SCHD_CHUNK_VAL]])
; CHECK:  call void @__kmpc_for_static_init_4({{.*}}, i32 1, i32 [[SCHD_CHUNK_VAL]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z21parallel_for_schedulei(i32 noundef %bbb) #0 {
entry:
  %bbb.addr = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %bbb, ptr %bbb.addr, align 4
  %i1 = load i32, ptr %bbb.addr, align 4
  store i32 %i1, ptr %.capture_expr.0, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %schd.chunk = load i32, ptr %.capture_expr.0, align 4
  %i3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 %schd.chunk),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.0, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %i4 = load i32, ptr %.omp.lb, align 4
  store i32 %i4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %i5 = load i32, ptr %.omp.iv, align 4
  %i6 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %i5, %i6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %i7 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %i7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %i8 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %i8, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %i3) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z32distribute_parallel_for_schedulei(i32 noundef %bbb) #0 {
entry:
  %bbb.addr = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %bbb, ptr %bbb.addr, align 4
  %i1 = load i32, ptr %bbb.addr, align 4
  store i32 %i1, ptr %.capture_expr.1, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %dist.schd.chunk = load i32, ptr %bbb.addr, align 4
  %schd.chunk = load i32, ptr %.capture_expr.1, align 4
  %i4 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.DIST_SCHEDULE.STATIC"(i32 %dist.schd.chunk),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 %schd.chunk),
    "QUAL.OMP.SHARED:TYPED"(ptr %bbb.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %i5 = load i32, ptr %.omp.lb, align 4
  store i32 %i5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %i6 = load i32, ptr %.omp.iv, align 4
  %i7 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %i6, %i7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %i8 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %i8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %i9 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %i9, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %i4) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  ret void
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
