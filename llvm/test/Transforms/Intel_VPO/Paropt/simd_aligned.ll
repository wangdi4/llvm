; RUN: opt < %s -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S | FileCheck %s
; RUN: opt < %s -passes='function(loop(rotate),vpo-cfg-restructuring,vpo-paropt-prepare,simplify-cfg,loop(simplify-cfg),sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s

; Original code:
; void foo(int *A, unsigned N) {
; #pragma omp simd aligned(A:32)
;   for (int I = 0; I < N; ++I)
;     A[I] = I;
; }

; CHECK-LABEL: define {{[^@]+}}@foo
; CHECK-SAME:  (i32* [[PTR:%.+]], i32 %{{.+}})
; CHECK:       call void @llvm.assume(i1 true) [ "align"(i32* [[PTR]], i32 32) ]
; CHECK:         [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.ALIGNED"(i32* null, i32 32),
; CHECK-NEXT:    br label %[[LOOPBODY:[^,]+]]
; CHECK:       [[LOOPBODY]]:
; CHECK:         br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LOOPEXIT:[^,]+]]
; CHECK:       [[LOOPEXIT]]:
; CHECK:         call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* %A, i32 %N) #0 {
entry:
  %A.addr = alloca i32*, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32* %A, i32** %A.addr, align 8, !tbaa !2
  store i32 %N, i32* %N.addr, align 4, !tbaa !6
  %0 = load i32, i32* %N.addr, align 4, !tbaa !6
  store i32 %0, i32* %.capture_expr., align 4, !tbaa !6
  %1 = load i32, i32* %.capture_expr., align 4, !tbaa !6
  %sub = sub i32 %1, 0
  %div = udiv i32 %sub, 1
  %sub2 = sub i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4, !tbaa !6
  %2 = load i32, i32* %.capture_expr., align 4, !tbaa !6
  %cmp = icmp ult i32 0, %2
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %3 = load i32, i32* %.capture_expr.1, align 4, !tbaa !6
  store i32 %3, i32* %.omp.ub, align 4, !tbaa !6
  %4 = load i32*, i32** %A.addr, align 8, !tbaa !2
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.ALIGNED"(i32* %4, i32 32), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LINEAR:IV"(i32* %I, i32 1) ]
  store i32 0, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %6 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %7 = load i32, i32* %.omp.ub, align 4, !tbaa !6
  %cmp3 = icmp ule i32 %6, %7
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %mul = mul i32 %8, 1
  %add = add i32 0, %mul
  store i32 %add, i32* %I, align 4, !tbaa !6
  %9 = load i32, i32* %I, align 4, !tbaa !6
  %10 = load i32*, i32** %A.addr, align 8, !tbaa !2
  %11 = load i32, i32* %I, align 4, !tbaa !6
  %idxprom = sext i32 %11 to i64
  %ptridx = getelementptr inbounds i32, i32* %10, i64 %idxprom
  store i32 %9, i32* %ptridx, align 4, !tbaa !6
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %add4 = add nuw i32 %12, 1
  store i32 %add4, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
