; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-opt-scalar-fp=false -vpo-paropt-enable-outline-verification=true -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-opt-scalar-fp=false -vpo-paropt-enable-outline-verification=true -S %s | FileCheck %s

; Test src:
;
; void foo(int s, int e) {
; #pragma omp parallel for ordered(2)
;   for (int i = s; i < e; ++i)
;     for (int j = s; j < e; ++j)
;       ;
; }

; Check that the doacross loop nest's tripcounts are passed via pointers.
; 'opt' will just fail otherwise, since verification is enabled by the option.
; CHECK: define internal void @foo{{[0-9A-Za-z_.]*}}(ptr{{[^,]*}}, ptr{{[^,]*}}, ptr{{[^,]*}}, ptr{{[^,]*}}, ptr{{[^,]*}}, ptr{{[^,]*}}, ptr{{[^,]*}}, ptr{{[^,]*}}, ptr{{[^,]*}}, ptr{{[^,]*}}, ptr{{[^,]*}}, ptr{{[^,]*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32 noundef %s, i32 noundef %e) #0 {
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
  store i32 %s, ptr %s.addr, align 4, !tbaa !4
  store i32 %e, ptr %e.addr, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.0) #2
  %0 = load i32, ptr %s.addr, align 4, !tbaa !4
  store i32 %0, ptr %.capture_expr.0, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.1) #2
  %1 = load i32, ptr %e.addr, align 4, !tbaa !4
  store i32 %1, ptr %.capture_expr.1, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.2) #2
  %2 = load i32, ptr %s.addr, align 4, !tbaa !4
  store i32 %2, ptr %.capture_expr.2, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.3) #2
  %3 = load i32, ptr %e.addr, align 4, !tbaa !4
  store i32 %3, ptr %.capture_expr.3, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.4) #2
  %4 = load i32, ptr %.capture_expr.1, align 4, !tbaa !4
  %5 = load i32, ptr %.capture_expr.0, align 4, !tbaa !4
  %sub = sub i32 %4, %5
  %sub1 = sub i32 %sub, 1
  %add = add i32 %sub1, 1
  %div = udiv i32 %add, 1
  %sub2 = sub i32 %div, 1
  store i32 %sub2, ptr %.capture_expr.4, align 4, !tbaa !4
  %6 = load i32, ptr %.capture_expr.0, align 4, !tbaa !4
  %7 = load i32, ptr %.capture_expr.1, align 4, !tbaa !4
  %cmp = icmp slt i32 %6, %7
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub) #2
  %8 = load i32, ptr %.capture_expr.4, align 4, !tbaa !4
  store i32 %8, ptr %.omp.ub, align 4, !tbaa !4
  %9 = load i32, ptr %.capture_expr.1, align 4, !tbaa !4
  %10 = load i32, ptr %.capture_expr.0, align 4, !tbaa !4
  %sub3 = sub i32 %9, %10
  %sub4 = sub i32 %sub3, 1
  %add5 = add i32 %sub4, 1
  %div6 = udiv i32 %add5, 1
  %11 = load i32, ptr %.capture_expr.3, align 4, !tbaa !4
  %12 = load i32, ptr %.capture_expr.2, align 4, !tbaa !4
  %sub7 = sub i32 %11, %12
  %sub8 = sub i32 %sub7, 1
  %add9 = add i32 %sub8, 1
  %div10 = udiv i32 %add9, 1
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.ORDERED"(i32 2, i32 %div6, i32 %div10),
    "QUAL.OMP.SHARED:TYPED"(ptr %s.addr, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %e.addr, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.0, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.2, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %.capture_expr.3, i32 0, i32 1) ]
  %14 = load i32, ptr %.omp.lb, align 4, !tbaa !4
  store i32 %14, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %15 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %16 = load i32, ptr %.omp.ub, align 4, !tbaa !4
  %add11 = add i32 %16, 1
  %cmp12 = icmp ult i32 %15, %add11
  br i1 %cmp12, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %i) #2
  %17 = load i32, ptr %.capture_expr.0, align 4, !tbaa !4
  %18 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %mul = mul i32 %18, 1
  %add13 = add i32 %17, %mul
  store i32 %add13, ptr %i, align 4, !tbaa !4
  call void @llvm.lifetime.start.p0(i64 4, ptr %j) #2
  %19 = load i32, ptr %s.addr, align 4, !tbaa !4
  store i32 %19, ptr %j, align 4, !tbaa !4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.inner.for.body
  %20 = load i32, ptr %j, align 4, !tbaa !4
  %21 = load i32, ptr %e.addr, align 4, !tbaa !4
  %cmp14 = icmp slt i32 %20, %21
  br i1 %cmp14, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0(i64 4, ptr %j) #2
  br label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %22 = load i32, ptr %j, align 4, !tbaa !4
  %inc = add nsw i32 %22, 1
  store i32 %inc, ptr %j, align 4, !tbaa !4
  br label %for.cond, !llvm.loop !8

for.end:                                          ; preds = %for.cond.cleanup
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end
  call void @llvm.lifetime.end.p0(i64 4, ptr %i) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %23 = load i32, ptr %.omp.iv, align 4, !tbaa !4
  %add15 = add nuw i32 %23, 1
  store i32 %add15, ptr %.omp.iv, align 4, !tbaa !4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.4) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.3) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.2) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.1) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %.capture_expr.0) #2
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
