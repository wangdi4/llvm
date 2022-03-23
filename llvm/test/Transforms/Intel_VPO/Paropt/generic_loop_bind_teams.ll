; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; This test checks that the "loop" construct is mapped to "distribute parallel for"
; after prepare pass, if binding rule is teams.

; int aaa[1000];
; void foo() {
;   for (int i=0; i<1000; ++i) {
; # pragma omp loop bind(teams)
;     for (int j=0; j<100; j++) {
;       aaa[i] += i + j;
;     }
;   }
; }



; ModuleID = 'generic_loop_bind_teams.c'
source_filename = "generic_loop_bind_teams.c"
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
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  store i32 0, i32* %i, align 4, !tbaa !2
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, i32* %i, align 4, !tbaa !2
  %cmp = icmp slt i32 %1, 1000
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %2 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #2
  br label %for.end

for.body:                                         ; preds = %for.cond
  %3 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #2
  %4 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %5 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #2
  store i32 99, i32* %.omp.ub, align 4, !tbaa !2

; Verify that DIR.OMP.GENERICLOOP is mapped to DIR.OMP.DISTRIBUTE.PARLOOP
; CHECK-NOT: call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), {{.*}}
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
; CHECK-NOT: "QUAL.OMP.BIND.TEAMS"
; CHECK-SAME:  "QUAL.OMP.FIRSTPRIVATE"({{.*}}), "QUAL.OMP.NORMALIZED.IV"({{.*}}), "QUAL.OMP.NORMALIZED.UB"({{.*}}), "QUAL.OMP.PRIVATE"({{.*}}), "QUAL.OMP.SHARED"({{.*}}), "QUAL.OMP.SHARED"({{.*}})

  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.BIND.TEAMS"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.SHARED"([1000 x i32]* @aaa) ]
  %7 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store i32 %7, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.body
  %8 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %9 = load i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp1 = icmp sle i32 %8, %9
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10) #2
  %11 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %mul = mul nsw i32 %11, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %j, align 4, !tbaa !2
  %12 = load i32, i32* %i, align 4, !tbaa !2
  %13 = load i32, i32* %j, align 4, !tbaa !2
  %add2 = add nsw i32 %12, %13
  %14 = load i32, i32* %i, align 4, !tbaa !2
  %idxprom = sext i32 %14 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], [1000 x i32]* @aaa, i64 0, i64 %idxprom, !intel-tbaa !6
  %15 = load i32, i32* %arrayidx, align 4, !tbaa !6
  %add3 = add nsw i32 %15, %add2
  store i32 %add3, i32* %arrayidx, align 4, !tbaa !6
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %16 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %16) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %17 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %add4 = add nsw i32 %17, 1
  store i32 %add4, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end

; CHECK-NOT: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.GENERICLOOP"() {{.*}}
; CHECK: call void @llvm.directive.region.exit(token %{{.*}}) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() {{.*}}

  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.GENERICLOOP"() ]
  %18 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #2
  %19 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #2
  %20 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #2
  br label %for.inc

for.inc:                                          ; preds = %omp.loop.exit
  %21 = load i32, i32* %i, align 4, !tbaa !2
  %inc = add nsw i32 %21, 1
  store i32 %inc, i32* %i, align 4, !tbaa !2
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA1000_i", !3, i64 0}
