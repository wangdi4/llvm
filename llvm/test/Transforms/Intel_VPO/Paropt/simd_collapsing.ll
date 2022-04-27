; RUN: opt -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S %s | FileCheck %s

; Original code:
; void foo(int n) {
; #pragma omp simd collapse(2)
;   for (int i = 0; i < n; ++i)
;     for (int j = 0; j < n; ++j);
; }

; CHECK-NOT: QUAL.OMP.FIRSTPRIVATE
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"()
; CHECK-SAME: QUAL.OMP.COLLAPSE
; CHECK-SAME: QUAL.OMP.NORMALIZED.IV
; CHECK-SAME: QUAL.OMP.NORMALIZED.UB
; CHECK-SAME: QUAL.OMP.PRIVATE

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32 %n) #0 {
entry:
  %n.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.2 = alloca i32, align 4
  %.capture_expr.3 = alloca i64, align 8
  %.omp.uncollapsed.iv = alloca i64, align 8
  %.omp.uncollapsed.iv16 = alloca i64, align 8
  %.omp.uncollapsed.lb = alloca i64, align 8
  %.omp.uncollapsed.ub = alloca i64, align 8
  %.omp.uncollapsed.lb23 = alloca i64, align 8
  %.omp.uncollapsed.ub24 = alloca i64, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 %n, i32* %n.addr, align 4, !tbaa !2
  %0 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = load i32, i32* %n.addr, align 4, !tbaa !2
  store i32 %1, i32* %.capture_expr., align 4, !tbaa !2
  %2 = bitcast i32* %.capture_expr.2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = load i32, i32* %n.addr, align 4, !tbaa !2
  store i32 %3, i32* %.capture_expr.2, align 4, !tbaa !2
  %4 = bitcast i64* %.capture_expr.3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %4) #2
  %5 = load i32, i32* %.capture_expr., align 4, !tbaa !2
  %sub = sub nsw i32 %5, 0
  %sub4 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub4, 1
  %div = sdiv i32 %add, 1
  %conv = sext i32 %div to i64
  %6 = load i32, i32* %.capture_expr.2, align 4, !tbaa !2
  %sub5 = sub nsw i32 %6, 0
  %sub6 = sub nsw i32 %sub5, 1
  %add7 = add nsw i32 %sub6, 1
  %div8 = sdiv i32 %add7, 1
  %conv9 = sext i32 %div8 to i64
  %mul = mul nsw i64 %conv, %conv9
  %sub10 = sub nsw i64 %mul, 1
  store i64 %sub10, i64* %.capture_expr.3, align 8, !tbaa !6
  %7 = load i32, i32* %.capture_expr., align 4, !tbaa !2
  %cmp = icmp slt i32 0, %7
  br i1 %cmp, label %land.lhs.true, label %omp.precond.end

land.lhs.true:                                    ; preds = %entry
  %8 = load i32, i32* %.capture_expr.2, align 4, !tbaa !2
  %cmp13 = icmp slt i32 0, %8
  br i1 %cmp13, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %land.lhs.true
  %9 = bitcast i64* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %9) #2
  %10 = bitcast i64* %.omp.uncollapsed.iv16 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %10) #2
  %11 = bitcast i64* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %11) #2
  store i64 0, i64* %.omp.uncollapsed.lb, align 8, !tbaa !6
  %12 = bitcast i64* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %12) #2
  %13 = load i32, i32* %.capture_expr., align 4, !tbaa !2
  %sub17 = sub nsw i32 %13, 0
  %sub18 = sub nsw i32 %sub17, 1
  %add19 = add nsw i32 %sub18, 1
  %div20 = sdiv i32 %add19, 1
  %conv21 = sext i32 %div20 to i64
  %sub22 = sub nsw i64 %conv21, 1
  store i64 %sub22, i64* %.omp.uncollapsed.ub, align 8, !tbaa !6
  %14 = bitcast i64* %.omp.uncollapsed.lb23 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %14) #2
  store i64 0, i64* %.omp.uncollapsed.lb23, align 8, !tbaa !6
  %15 = bitcast i64* %.omp.uncollapsed.ub24 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %15) #2
  %16 = load i32, i32* %.capture_expr.2, align 4, !tbaa !2
  %sub25 = sub nsw i32 %16, 0
  %sub26 = sub nsw i32 %sub25, 1
  %add27 = add nsw i32 %sub26, 1
  %div28 = sdiv i32 %add27, 1
  %conv29 = sext i32 %div28 to i64
  %sub30 = sub nsw i64 %conv29, 1
  store i64 %sub30, i64* %.omp.uncollapsed.ub24, align 8, !tbaa !6
  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.uncollapsed.iv, i64* %.omp.uncollapsed.iv16), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.uncollapsed.ub, i64* %.omp.uncollapsed.ub24) ]
  %18 = load i64, i64* %.omp.uncollapsed.lb, align 8, !tbaa !6
  store i64 %18, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc44, %omp.precond.then
  %19 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
  %20 = load i64, i64* %.omp.uncollapsed.ub, align 8, !tbaa !6
  %cmp31 = icmp sle i64 %19, %20
  br i1 %cmp31, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end46

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %21 = load i64, i64* %.omp.uncollapsed.lb23, align 8, !tbaa !6
  store i64 %21, i64* %.omp.uncollapsed.iv16, align 8, !tbaa !6
  br label %omp.uncollapsed.loop.cond33

omp.uncollapsed.loop.cond33:                      ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %22 = load i64, i64* %.omp.uncollapsed.iv16, align 8, !tbaa !6
  %23 = load i64, i64* %.omp.uncollapsed.ub24, align 8, !tbaa !6
  %cmp34 = icmp sle i64 %22, %23
  br i1 %cmp34, label %omp.uncollapsed.loop.body36, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body36:                      ; preds = %omp.uncollapsed.loop.cond33
  %24 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %24) #2
  %25 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %25) #2
  %26 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
  %mul37 = mul nsw i64 %26, 1
  %add38 = add nsw i64 0, %mul37
  %conv39 = trunc i64 %add38 to i32
  store i32 %conv39, i32* %i, align 4, !tbaa !2
  %27 = load i64, i64* %.omp.uncollapsed.iv16, align 8, !tbaa !6
  %mul40 = mul nsw i64 %27, 1
  %add41 = add nsw i64 0, %mul40
  %conv42 = trunc i64 %add41 to i32
  store i32 %conv42, i32* %j, align 4, !tbaa !2
  %28 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %28) #2
  %29 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %29) #2
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.uncollapsed.loop.body36
  %30 = load i64, i64* %.omp.uncollapsed.iv16, align 8, !tbaa !6
  %add43 = add nsw i64 %30, 1
  store i64 %add43, i64* %.omp.uncollapsed.iv16, align 8, !tbaa !6
  br label %omp.uncollapsed.loop.cond33

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond33
  br label %omp.uncollapsed.loop.inc44

omp.uncollapsed.loop.inc44:                       ; preds = %omp.uncollapsed.loop.end
  %31 = load i64, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
  %add45 = add nsw i64 %31, 1
  store i64 %add45, i64* %.omp.uncollapsed.iv, align 8, !tbaa !6
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end46:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %17) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.uncollapsed.loop.end46, %land.lhs.true, %entry
  %32 = bitcast i64* %.omp.uncollapsed.ub24 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %32) #2
  %33 = bitcast i64* %.omp.uncollapsed.lb23 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %33) #2
  %34 = bitcast i64* %.omp.uncollapsed.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %34) #2
  %35 = bitcast i64* %.omp.uncollapsed.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %35) #2
  %36 = bitcast i64* %.omp.uncollapsed.iv16 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %36) #2
  %37 = bitcast i64* %.omp.uncollapsed.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %37) #2
  %38 = bitcast i64* %.capture_expr.3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %38) #2
  %39 = bitcast i32* %.capture_expr.2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %39) #2
  %40 = bitcast i32* %.capture_expr. to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %40) #2
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
!6 = !{!7, !7, i64 0}
!7 = !{!"long", !4, i64 0}
