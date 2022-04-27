; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Verify that all parameters to the outlined function are pointers.
; CHECK: define {{.*}} @_Z3fooPfx.DIR.OMP.PARALLEL.LOOP.{{[0-9]+}}.{{.*}}({{[^,]+}}*{{[^,]*}}, {{[^,]+}}*{{[^,]*}}, {{[^,]+}}*{{[^,]*}}, {{[^,]+}}*{{[^,]*}}, {{[^,]+}}*{{[^,]*}}, {{[^,]+}}*{{[^,]*}})

; Original code:
; void foo(float *x, long long int n)
; {
; #pragma omp parallel for
;   for (long long int i = 0; i < n; ++i)
;     x[i] = (float)(int)&n;
; }

; The test verifies that the promotion of the normalized upper bound
; does not produce new live-in values, which are not referenced
; in the region's clauses.  If it does, this would look like
; a non-pointer value being passed to the outlined function.

; ModuleID = 'ompfor.cpp'
source_filename = "ompfor.cpp"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @_Z3fooPfx(float* %x, i64 %n) #0 {
entry:
  %x.addr = alloca float*, align 4
  %n.addr = alloca i64, align 8
  %.omp.iv = alloca i64, align 8
  %tmp = alloca i64, align 4
  %.capture_expr. = alloca i64, align 8
  %.capture_expr.1 = alloca i64, align 8
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %.omp.stride = alloca i64, align 8
  %.omp.is_last = alloca i32, align 4
  %i = alloca i64, align 8
  store float* %x, float** %x.addr, align 4, !tbaa !3
  store i64 %n, i64* %n.addr, align 8, !tbaa !7
  %0 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #2
  %1 = bitcast i64* %.capture_expr. to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %1) #2
  %2 = load i64, i64* %n.addr, align 8, !tbaa !7
  store i64 %2, i64* %.capture_expr., align 8, !tbaa !7
  %3 = bitcast i64* %.capture_expr.1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %3) #2
  %4 = load i64, i64* %.capture_expr., align 8, !tbaa !7
  %sub = sub nsw i64 %4, -1
  %sub2 = sub nsw i64 %sub, 1
  %add = add nsw i64 %sub2, 1
  %div = sdiv i64 %add, 1
  %sub3 = sub nsw i64 %div, 1
  store i64 %sub3, i64* %.capture_expr.1, align 8, !tbaa !7
  %5 = load i64, i64* %.capture_expr., align 8, !tbaa !7
  %cmp = icmp slt i64 -1, %5
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %6 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %6) #2
  store i64 0, i64* %.omp.lb, align 8, !tbaa !7
  %7 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %7) #2
  %8 = load i64, i64* %.capture_expr.1, align 8, !tbaa !7
  store i64 %8, i64* %.omp.ub, align 8, !tbaa !7
  %9 = bitcast i64* %.omp.stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %9) #2
  store i64 1, i64* %.omp.stride, align 8, !tbaa !7
  %10 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %10) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !9
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i64* %i), "QUAL.OMP.SHARED"(i64* %n.addr), "QUAL.OMP.SHARED"(float** %x.addr) ]
  %12 = load i64, i64* %.omp.lb, align 8, !tbaa !7
  store i64 %12, i64* %.omp.iv, align 8, !tbaa !7
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %13 = load i64, i64* %.omp.iv, align 8, !tbaa !7
  %14 = load i64, i64* %.omp.ub, align 8, !tbaa !7
  %cmp4 = icmp sle i64 %13, %14
  br i1 %cmp4, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %15 = bitcast i64* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %15) #2
  %16 = load i64, i64* %.omp.iv, align 8, !tbaa !7
  %mul = mul nsw i64 %16, 1
  %add5 = add nsw i64 -1, %mul
  store i64 %add5, i64* %i, align 8, !tbaa !7
  %17 = ptrtoint i64* %n.addr to i32
  %conv = sitofp i32 %17 to float
  %18 = load float*, float** %x.addr, align 4, !tbaa !3
  %19 = load i64, i64* %i, align 8, !tbaa !7
  %idxprom = trunc i64 %19 to i32
  %arrayidx = getelementptr inbounds float, float* %18, i32 %idxprom
  store float %conv, float* %arrayidx, align 4, !tbaa !11
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %20 = bitcast i64* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %20) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %21 = load i64, i64* %.omp.iv, align 8, !tbaa !7
  %add6 = add nsw i64 %21, 1
  store i64 %add6, i64* %.omp.iv, align 8, !tbaa !7
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %22 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #2
  %23 = bitcast i64* %.omp.stride to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %23) #2
  %24 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %24) #2
  %25 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %25) #2
  %26 = bitcast i64* %.capture_expr.1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %26) #2
  %27 = bitcast i64* %.capture_expr. to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %27) #2
  %28 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %28) #2
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

attributes #0 = { nounwind uwtable }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"NumRegisterParameters", i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!3 = !{!4, !4, i64 0}
!4 = !{!"pointer@_ZTSPf", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"long long", !5, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !5, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"float", !5, i64 0}
