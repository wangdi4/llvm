; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; This test checks that the "loop" construct is mapped to "distribute parallel for"
; after prepare pass.

; int aaa[1000];
; void foo() {
; #pragma omp target teams
;   for (int i=0; i<1000; ++i) {
; # pragma omp loop
;     for (int j=0; j<100; j++) {
;       aaa[i] += i + j;
;     }
;   }
; }



; ModuleID = 'generic_loop_in_teams.c'
source_filename = "generic_loop_in_teams.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@aaa = common dso_local global [1000 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %j = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([1000 x i32]* @aaa, [1000 x i32]* @aaa, i64 4000, i64 547), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"([1000 x i32]* @aaa), "QUAL.OMP.PRIVATE"(i32* %tmp) ]
  %2 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #1
  store i32 0, i32* %i, align 4, !tbaa !3
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %3 = load i32, i32* %i, align 4, !tbaa !3
  %cmp = icmp slt i32 %3, 1000
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %4 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #1
  br label %for.end

for.body:                                         ; preds = %for.cond
  %5 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #1
  %6 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #1
  store i32 0, i32* %.omp.lb, align 4, !tbaa !3
  %7 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #1
  store i32 99, i32* %.omp.ub, align 4, !tbaa !3

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.DISTRIBUTE.PARLOOP
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.FIRSTPRIVATE"({{.*}}), "QUAL.OMP.NORMALIZED.IV"({{.*}}), "QUAL.OMP.NORMALIZED.UB"({{.*}}), "QUAL.OMP.PRIVATE"({{.*}}), "QUAL.OMP.SHARED"({{.*}}), "QUAL.OMP.SHARED"({{.*}})

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.SHARED"([1000 x i32]* @aaa) ]
  %9 = load i32, i32* %.omp.lb, align 4, !tbaa !3
  store i32 %9, i32* %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.body
  %10 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %11 = load i32, i32* %.omp.ub, align 4, !tbaa !3
  %cmp1 = icmp sle i32 %10, %11
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %12) #1
  %13 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %mul = mul nsw i32 %13, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %j, align 4, !tbaa !3
  %14 = load i32, i32* %i, align 4, !tbaa !3
  %15 = load i32, i32* %j, align 4, !tbaa !3
  %add2 = add nsw i32 %14, %15
  %16 = load i32, i32* %i, align 4, !tbaa !3
  %idxprom = sext i32 %16 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @aaa, i64 0, i64 %idxprom, !intel-tbaa !7
  %17 = load i32, i32* %arrayidx, align 4, !tbaa !7
  %add3 = add nsw i32 %17, %add2
  store i32 %add3, i32* %arrayidx, align 4, !tbaa !7
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %18 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #1
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %19 = load i32, i32* %.omp.iv, align 4, !tbaa !3
  %add4 = add nsw i32 %19, 1
  store i32 %add4, i32* %.omp.iv, align 4, !tbaa !3
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

; CHECK-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() {{.*}}

  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.GENERICLOOP"() ]
  %20 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #1
  %21 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %21) #1
  %22 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #1
  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit
  %23 = load i32, i32* %i, align 4, !tbaa !3
  %inc = add nsw i32 %23, 1
  store i32 %inc, i32* %i, align 4, !tbaa !3
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind willreturn }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2052, i32 41162786, !"foo", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA1000_i", !4, i64 0}
