; RUN: opt < %s -switch-to-offload=true -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
;
; The compiler checks the loop is only partitioned by teams.
;#define MAX 100
;
;int A[MAX][MAX], B[MAX][MAX], C[MAX][MAX];
;
;void __attribute__ ((noinline)) Compute()
;{
;  #pragma omp target map(to: A, B) map(tofrom: C)
;  {
;    #pragma omp teams num_teams(2)
;    #pragma omp distribute
;    for (int i = 0; i < MAX; i++)
;    for (int j = 0; j < MAX; j++)
;    for (int k = 0; k < MAX; k++)
;         C[i][j] += A[i][k] * B[k][j];
;  }
;}

target triple = "spir64"
target device_triples = "spir64"

@A = external dso_local global [100 x [100 x i32]], align 4
@B = external dso_local global [100 x [100 x i32]], align 4
@C = external dso_local global [100 x [100 x i32]], align 4
@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind uwtable
define dso_local spir_func void @_Z7Computev() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TO"([100 x [100 x i32]]* @A), "QUAL.OMP.MAP.TO"([100 x [100 x i32]]* @B), "QUAL.OMP.MAP.TOFROM"([100 x [100 x i32]]* @C), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.stride), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %k) ]
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.NUM_TEAMS"(i32 2), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.stride), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.SHARED"([100 x [100 x i32]]* @A), "QUAL.OMP.SHARED"([100 x [100 x i32]]* @B), "QUAL.OMP.SHARED"([100 x [100 x i32]]* @C) ]
  %2 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @A to i8*))
  %3 = bitcast i8* %2 to [100 x [100 x i32]]*
  %4 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @B to i8*))
  %5 = bitcast i8* %4 to [100 x [100 x i32]]*
  %6 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @C to i8*))
  %7 = bitcast i8* %6 to [100 x [100 x i32]]*
  %8 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %8) #1
  %9 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #1
  store i32 0, i32* %.omp.lb, align 4, !tbaa !4
  %10 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10) #1
  store volatile i32 99, i32* %.omp.ub, align 4, !tbaa !4
  %11 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %11) #1
  store i32 1, i32* %.omp.stride, align 4, !tbaa !4
  %12 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %12) #1
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !4
  br label %DIR.OMP.DISTRIBUTE.4

DIR.OMP.DISTRIBUTE.4:                             ; preds = %DIR.OMP.TARGET.2
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %k) ]
  %14 = load i32, i32* %.omp.lb, align 4, !tbaa !4
  store volatile i32 %14, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %for.cond.cleanup, %DIR.OMP.DISTRIBUTE.4
  %15 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !4
  %16 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !4
  %cmp = icmp sle i32 %15, %16
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %17 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %17) #1
  %18 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !4
  store i32 %18, i32* %i, align 4, !tbaa !4
  %19 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %19) #1
  store i32 0, i32* %j, align 4, !tbaa !4
  br label %for.cond

for.cond:                                         ; preds = %for.cond.cleanup4, %omp.inner.for.body
  %20 = load i32, i32* %j, align 4, !tbaa !4
  %cmp1 = icmp slt i32 %20, 100
  br i1 %cmp1, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #1
  %21 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !4
  %add21 = add nsw i32 %21, 1
  store volatile i32 %add21, i32* %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

for.body:                                         ; preds = %for.cond
  %22 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %22) #1
  store i32 0, i32* %k, align 4, !tbaa !4
  br label %for.cond2

for.cond2:                                        ; preds = %for.body5, %for.body
  %23 = load i32, i32* %k, align 4, !tbaa !4
  %cmp3 = icmp slt i32 %23, 100
  br i1 %cmp3, label %for.body5, label %for.cond.cleanup4

for.cond.cleanup4:                                ; preds = %for.cond2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #1
  %24 = load i32, i32* %j, align 4, !tbaa !4
  %inc19 = add nsw i32 %24, 1
  store i32 %inc19, i32* %j, align 4, !tbaa !4
  br label %for.cond

for.body5:                                        ; preds = %for.cond2
  %25 = load i32, i32* %i, align 4, !tbaa !4
  %idxprom = sext i32 %25 to i64
  %arrayidx = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %3, i64 0, i64 %idxprom, !intel-tbaa !8
  %idxprom6 = sext i32 %23 to i64
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx, i64 0, i64 %idxprom6, !intel-tbaa !11
  %26 = load i32, i32* %arrayidx7, align 4, !tbaa !12
  %arrayidx9 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %5, i64 0, i64 %idxprom6, !intel-tbaa !8
  %27 = load i32, i32* %j, align 4, !tbaa !4
  %idxprom10 = sext i32 %27 to i64
  %arrayidx11 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx9, i64 0, i64 %idxprom10, !intel-tbaa !11
  %28 = load i32, i32* %arrayidx11, align 4, !tbaa !12
  %mul12 = mul nsw i32 %26, %28
  %arrayidx14 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %7, i64 0, i64 %idxprom, !intel-tbaa !8
  %arrayidx16 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx14, i64 0, i64 %idxprom10, !intel-tbaa !11
  %29 = load i32, i32* %arrayidx16, align 4, !tbaa !12
  %add17 = add nsw i32 %29, %mul12
  store i32 %add17, i32* %arrayidx16, align 4, !tbaa !12
  %30 = load i32, i32* %k, align 4, !tbaa !4
  %inc = add nsw i32 %30, 1
  store i32 %inc, i32* %k, align 4, !tbaa !4
  br label %for.cond2

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %9) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %8) #1
  br label %DIR.OMP.END.TEAMS.7

DIR.OMP.END.TEAMS.7:                              ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TEAMS.8

DIR.OMP.END.TEAMS.8:                              ; preds = %DIR.OMP.END.TEAMS.7
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

; Function Attrs: inaccessiblememonly nounwind speculatable
declare i8* @llvm.launder.invariant.group.p0i8(i8*) #3

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind }
attributes #3 = { inaccessiblememonly nounwind speculatable }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 58, i32 -1933259435, !"_Z7Computev", i32 11, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 8.0.0"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!9, !10, i64 0}
!9 = !{!"array@_ZTSA100_A100_i", !10, i64 0}
!10 = !{!"array@_ZTSA100_i", !5, i64 0}
!11 = !{!10, !5, i64 0}
!12 = !{!9, !5, i64 0}

; CHECK: %{{.*}} = call i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %{{.*}} = call i64 @_Z12get_group_idj(i32 0)
; CHECK-NOT: %{{.*}} = call i64 @_Z14get_local_sizej(i32 0)
; CHECK-NOT: %{{.*}} = call i64 @_Z12get_local_idj(i32 0)
