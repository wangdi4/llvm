; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -vpo-paropt-enable-outline-verification=true -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -vpo-paropt-enable-outline-verification=true -S %s | FileCheck %s

; Original code:
; void foo(int s, int e) {
; #pragma omp parallel for ordered(2)
;   for (int i = s; i < e; ++i)
;     for (int j = s; j < e; ++j);
; }

; Check that the doacross loop nest's tripcounts are passed via pointers.
; 'opt' will just fail otherwise, since verification is enabled by the option.
; CHECK: define internal void @foo{{[0-9A-Za-z_.]*}}(i32*{{[^,]*}}, i32*{{[^,]*}}, i32*{{[^,]*}}, i32*{{[^,]*}}, i32*{{[^,]*}}, i32*{{[^,]*}}, i32*{{[^,]*}}, i32*{{[^,]*}}, i32*{{[^,]*}}, i32*{{[^,]*}}, i32*{{[^,]*}}, i32*{{[^,]*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32 %s, i32 %e) #0 {
entry:
  %s.addr = alloca i32, align 4
  %e.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i32, align 4
  %.capture_expr.4 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 %s, i32* %s.addr, align 4, !tbaa !2
  store i32 %e, i32* %e.addr, align 4, !tbaa !2
  %0 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = load i32, i32* %s.addr, align 4, !tbaa !2
  store i32 %1, i32* %.capture_expr.0, align 4, !tbaa !2
  %2 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = load i32, i32* %e.addr, align 4, !tbaa !2
  store i32 %3, i32* %.capture_expr.1, align 4, !tbaa !2
  %4 = bitcast i32* %.capture_expr.2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #2
  %5 = load i32, i32* %s.addr, align 4, !tbaa !2
  store i32 %5, i32* %.capture_expr.2, align 4, !tbaa !2
  %6 = bitcast i32* %.capture_expr.3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #2
  %7 = load i32, i32* %e.addr, align 4, !tbaa !2
  store i32 %7, i32* %.capture_expr.3, align 4, !tbaa !2
  %8 = bitcast i32* %.capture_expr.4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %8) #2
  %9 = load i32, i32* %.capture_expr.1, align 4, !tbaa !2
  %10 = load i32, i32* %.capture_expr.0, align 4, !tbaa !2
  %sub = sub i32 %9, %10
  %sub1 = sub i32 %sub, 1
  %add = add i32 %sub1, 1
  %div = udiv i32 %add, 1
  %sub2 = sub i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.4, align 4, !tbaa !2
  %11 = load i32, i32* %.capture_expr.0, align 4, !tbaa !2
  %12 = load i32, i32* %.capture_expr.1, align 4, !tbaa !2
  %cmp = icmp slt i32 %11, %12
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %13 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %13) #2
  %14 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %14) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %15 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %15) #2
  %16 = load i32, i32* %.capture_expr.4, align 4, !tbaa !2
  store i32 %16, i32* %.omp.ub, align 4, !tbaa !2
  %17 = load i32, i32* %.capture_expr.1, align 4, !tbaa !2
  %18 = load i32, i32* %.capture_expr.0, align 4, !tbaa !2
  %sub3 = sub i32 %17, %18
  %sub4 = sub i32 %sub3, 1
  %add5 = add i32 %sub4, 1
  %div6 = udiv i32 %add5, 1
  %19 = load i32, i32* %.capture_expr.3, align 4, !tbaa !2
  %20 = load i32, i32* %.capture_expr.2, align 4, !tbaa !2
  %sub7 = sub i32 %19, %20
  %sub8 = sub i32 %sub7, 1
  %add9 = add i32 %sub8, 1
  %div10 = udiv i32 %add9, 1
  %21 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.ORDERED"(i32 2, i32 %div6, i32 %div10), "QUAL.OMP.SHARED"(i32* %s.addr), "QUAL.OMP.SHARED"(i32* %e.addr), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(i32* %.capture_expr.0), "QUAL.OMP.SHARED"(i32* %.capture_expr.1), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"(i32* %.capture_expr.2), "QUAL.OMP.SHARED"(i32* %.capture_expr.3) ]
  %22 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store i32 %22, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %23 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %24 = load i32, i32* %.omp.ub, align 4, !tbaa !2
  %add11 = add i32 %24, 1
  %cmp12 = icmp ult i32 %23, %add11
  br i1 %cmp12, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %25 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %25) #2
  %26 = load i32, i32* %.capture_expr.0, align 4, !tbaa !2
  %27 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %mul = mul i32 %27, 1
  %add13 = add i32 %26, %mul
  store i32 %add13, i32* %i, align 4, !tbaa !2
  %28 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %28) #2
  %29 = load i32, i32* %s.addr, align 4, !tbaa !2
  store i32 %29, i32* %j, align 4, !tbaa !2
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.inner.for.body
  %30 = load i32, i32* %j, align 4, !tbaa !2
  %31 = load i32, i32* %e.addr, align 4, !tbaa !2
  %cmp14 = icmp slt i32 %30, %31
  br i1 %cmp14, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  %32 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %32) #2
  br label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %33 = load i32, i32* %j, align 4, !tbaa !2
  %inc = add nsw i32 %33, 1
  store i32 %inc, i32* %j, align 4, !tbaa !2
  br label %for.cond

for.end:                                          ; preds = %for.cond.cleanup
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end
  %34 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %34) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %35 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %add15 = add nuw i32 %35, 1
  store i32 %add15, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %21) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %36 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %36) #2
  %37 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %37) #2
  %38 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %38) #2
  %39 = bitcast i32* %.capture_expr.4 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %39) #2
  %40 = bitcast i32* %.capture_expr.3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %40) #2
  %41 = bitcast i32* %.capture_expr.2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %41) #2
  %42 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %42) #2
  %43 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %43) #2
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

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
