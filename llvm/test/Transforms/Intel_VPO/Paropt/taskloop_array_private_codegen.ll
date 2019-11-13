; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s

; This file tests the OpenMP code generation for the private array clause in the taskloop directive.
; void foo() {
; #pragma omp taskloop
; for (int i = 0; i < 10; ++i);
; }
;
target triple = "x86_64-unknown-linux-gnu"

@_Z5sivar = internal global [10 x i32] zeroinitializer, align 16
@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local i32 @_Z3foov() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %i = alloca i32, align 4
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %1) #2
  store i64 0, i64* %.omp.lb, align 8, !tbaa !2
  %2 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %2) #2
  store volatile i64 9, i64* %.omp.ub, align 8, !tbaa !2
  br label %DIR.OMP.TASKLOOP.1

DIR.OMP.TASKLOOP.1:                               ; preds = %entry
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(), "QUAL.OMP.PRIVATE"([10 x i32]* @_Z5sivar), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %4 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([10 x i32]* @_Z5sivar to i8*))
  %5 = bitcast i8* %4 to [10 x i32]*
  %6 = load i64, i64* %.omp.lb, align 8, !tbaa !2
  %conv = trunc i64 %6 to i32
  store volatile i32 %conv, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.TASKLOOP.1
  %7 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !6
  %conv1 = sext i32 %7 to i64
  %8 = load volatile i64, i64* %.omp.ub, align 8, !tbaa !2
  %cmp = icmp ule i64 %conv1, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #2
  %10 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !6
  store i32 %10, i32* %i, align 4, !tbaa !6
  %mul2 = mul nsw i32 2, %10
  %idxprom = sext i32 %10 to i64
  %arrayidx = getelementptr inbounds [10 x i32], [10 x i32]* %5, i64 0, i64 %idxprom
  store i32 %mul2, i32* %arrayidx, align 4, !tbaa !8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %9) #2
  %11 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !6
  %add3 = add nsw i32 %11, 1
  store volatile i32 %add3, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASKLOOP"() ]
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %2) #2
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %1) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #2
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

; Function Attrs: inaccessiblememonly nounwind speculatable
declare i8* @llvm.launder.invariant.group.p0i8(i8*) #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { inaccessiblememonly nounwind speculatable }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!9, !7, i64 0}
!9 = !{!"array@_ZTSA10_i", !7, i64 0}

; Check that private thunk has space for firstprivate %omp.lb,
; private @_Z5sivar and private %i.
; CHECK: %__struct.kmp_privates.t = type { i64, [10 x i32], i32 }

