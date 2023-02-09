; RUN: opt -enable-new-pm=0 -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -S %s | FileCheck %s

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

; CHECK: define internal void @_Z21parallel_for_schedulei.DIR.OMP.PARALLEL.LOOP.{{.*}}(i32* %tid, i32* %bid, i32* %schd.chunk.addr, i32* %.capture_expr., i32* %.omp.lb, i32* %.omp.ub)
; CHECK:  [[SCHD_CHUNK_VAL:%schd.chunk.*]] = load i32, i32* %schd.chunk.addr, align 4
; CHECK:  call void @__kmpc_for_static_init_4({{.*}}, i32 1, i32 [[SCHD_CHUNK_VAL]])

; CHECK: define internal void @_Z32distribute_parallel_for_schedulei.DIR.OMP.DISTRIBUTE.PARLOOP{{.*}}(i32* %tid, i32* %bid, i32* %schd.chunk.addr, i32* %dist.schd.chunk.addr, i32* %bbb.addr, i32* %.capture_expr., i32* %.omp.lb, i32* %.omp.ub)
; CHECK:  [[SCHD_CHUNK_VAL:%schd.chunk.*]] = load i32, i32* %schd.chunk.addr, align 4
; CHECK:  [[DIST_SCHD_CHUNK_VAL:%dist.schd.chunk.*]] = load i32, i32* %dist.schd.chunk.addr, align 4
; CHECK:  call void @__kmpc_team_static_init_4({{.*}}, i32 1, i32 [[DIST_SCHD_CHUNK_VAL]])
; CHECK:  call void @__kmpc_for_static_init_4({{.*}}, i32 1, i32 [[SCHD_CHUNK_VAL]])

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
  store i32 %bbb, i32* %bbb.addr, align 4, !tbaa !1
  %i1 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i1) #2
  %i2 = load i32, i32* %bbb.addr, align 4, !tbaa !1
  store i32 %i2, i32* %.capture_expr., align 4, !tbaa !1
  %i3 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i3) #2
  %i4 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i4) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !1
  %i5 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i5) #2
  store i32 99, i32* %.omp.ub, align 4, !tbaa !1
  %i6 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i6) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !1
  %i7 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i7) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !1
  %schd.chunk = load i32, i32* %.capture_expr., align 4, !tbaa !1
  %i9 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 %schd.chunk),
    "QUAL.OMP.SHARED"(i32* %.capture_expr.),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i) ]

  %i10 = load i32, i32* %.omp.lb, align 4, !tbaa !1
  store i32 %i10, i32* %.omp.iv, align 4, !tbaa !1
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %i11 = load i32, i32* %.omp.iv, align 4, !tbaa !1
  %i12 = load i32, i32* %.omp.ub, align 4, !tbaa !1
  %cmp = icmp sle i32 %i11, %i12
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %i13 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i13) #2
  %i14 = load i32, i32* %.omp.iv, align 4, !tbaa !1
  %mul = mul nsw i32 %i14, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !1
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %i15 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i15) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %i16 = load i32, i32* %.omp.iv, align 4, !tbaa !1
  %add1 = add nsw i32 %i16, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !1
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %i9) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  %i17 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i17) #2
  %i18 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i18) #2
  %i19 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i19) #2
  %i20 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i20) #2
  %i21 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i21) #2
  %i22 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i22) #2
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

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
  store i32 %bbb, i32* %bbb.addr, align 4, !tbaa !1
  %i1 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i1) #2
  %i2 = load i32, i32* %bbb.addr, align 4, !tbaa !1
  store i32 %i2, i32* %.capture_expr., align 4, !tbaa !1
  %i3 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i3) #2
  %i4 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i4) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !1
  %i5 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i5) #2
  store i32 99, i32* %.omp.ub, align 4, !tbaa !1
  %i6 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i6) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !1
  %i7 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i7) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !1
  %dist.schd.chunk = load i32, i32* %bbb.addr, align 4, !tbaa !1
  %schd.chunk = load i32, i32* %.capture_expr., align 4, !tbaa !1
  %i10 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.DIST_SCHEDULE.STATIC"(i32 %dist.schd.chunk),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 %schd.chunk),
    "QUAL.OMP.SHARED"(i32* %bbb.addr),
    "QUAL.OMP.SHARED"(i32* %.capture_expr.),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i) ]

  %i11 = load i32, i32* %.omp.lb, align 4, !tbaa !1
  store i32 %i11, i32* %.omp.iv, align 4, !tbaa !1
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %i12 = load i32, i32* %.omp.iv, align 4, !tbaa !1
  %i13 = load i32, i32* %.omp.ub, align 4, !tbaa !1
  %cmp = icmp sle i32 %i12, %i13
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %i14 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %i14) #2
  %i15 = load i32, i32* %.omp.iv, align 4, !tbaa !1
  %mul = mul nsw i32 %i15, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !1
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %i16 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i16) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %i17 = load i32, i32* %.omp.iv, align 4, !tbaa !1
  %add1 = add nsw i32 %i17, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !1
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %i10) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  %i18 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i18) #2
  %i19 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i19) #2
  %i20 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i20) #2
  %i21 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i21) #2
  %i22 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i22) #2
  %i23 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %i23) #2
  ret void
}

attributes #0 = { nounwind uwtable "may-have-openmp-directive"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C++ TBAA"}
