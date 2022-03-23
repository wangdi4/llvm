; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

; The test checks that schedule/dist_schedule chunk value is passed
; by pointer to the outlined routines.

; Original code:
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

; CHECK: define internal void @_Z21parallel_for_schedulei.DIR.OMP.PARALLEL.LOOP.{{[0-9]+}}.{{.*}}({{[^,]+\*[^,]*}}, {{[^,]+\*[^,]*}}, {{[^,]+\*[^,]*}}, {{[^,]+\*[^,]*}}, {{[^,]+\*[^,]*}})
; FIXME: the third parameter must be pointer, when we fix handling
;        of dist_schedule's chunk size.
; CHECK: define internal void @_Z32distribute_parallel_for_schedulei.DIR.OMP.DISTRIBUTE.PARLOOP.{{[0-9]+}}.{{.*}}({{[^,]+\*[^,]*}}, {{[^,]+\*[^,]*}}, {{[^,]+[^,]*}}, {{[^,]+\*[^,]*}}, {{[^,]+\*[^,]*}}, {{[^,]+\*[^,]*}}, {{[^,]+\*[^,]*}})

; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @_Z21parallel_for_schedulei(i32 %bbb) #0 {
entry:
  %bbb.addr = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %bbb, i32* %bbb.addr, align 4, !tbaa !2
  %0 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = load i32, i32* %bbb.addr, align 4, !tbaa !2
  store i32 %1, i32* %.capture_expr., align 4, !tbaa !2
  %2 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %4 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #2
  store i32 99, i32* %.omp.ub, align 4, !tbaa !2
  %5 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !2
  %6 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !2
  %7 = load i32, i32* %.capture_expr., align 4, !tbaa !2
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 %7), "QUAL.OMP.SHARED"(i32* %.capture_expr.), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %9 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store i32 %9, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %10 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %11 = load i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp = icmp sle i32 %10, %11
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %12) #2
  %13 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %mul = mul nsw i32 %13, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %14 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %14) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %add1 = add nsw i32 %15, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %16 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %16) #2
  %17 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #2
  %18 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #2
  %19 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #2
  %20 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #2
  %21 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %21) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local void @_Z32distribute_parallel_for_schedulei(i32 %bbb) #0 {
entry:
  %bbb.addr = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 %bbb, i32* %bbb.addr, align 4, !tbaa !2
  %0 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = load i32, i32* %bbb.addr, align 4, !tbaa !2
  store i32 %1, i32* %.capture_expr., align 4, !tbaa !2
  %2 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %4 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #2
  store i32 99, i32* %.omp.ub, align 4, !tbaa !2
  %5 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !2
  %6 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !2
  %7 = load i32, i32* %bbb.addr, align 4, !tbaa !2
  %8 = load i32, i32* %.capture_expr., align 4, !tbaa !2
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.DIST_SCHEDULE.STATIC"(i32 %7), "QUAL.OMP.SCHEDULE.STATIC"(i32 %8), "QUAL.OMP.SHARED"(i32* %bbb.addr), "QUAL.OMP.SHARED"(i32* %.capture_expr.), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %10 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store i32 %10, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %11 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %12 = load i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp = icmp sle i32 %11, %12
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %13 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %13) #2
  %14 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %mul = mul nsw i32 %14, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %15 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %add1 = add nsw i32 %16, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  %17 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #2
  %18 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #2
  %19 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #2
  %20 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #2
  %21 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %21) #2
  %22 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #2
  ret void
}

attributes #0 = { nounwind uwtable "may-have-openmp-directive"="true" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
