; RUN: opt -mattr=avx512f -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefixes=CHECK
; RUN: opt -mattr=avx512f -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefixes=CHECK

; Original code:
; void foo(int *A, int *B, unsigned N) {
; #pragma omp simd aligned(A:32, B)
;   for (int I = 0; I < N; ++I) {
;     A[I] = I;
;     B[I] = I;
;   }
; }

; CHECK-LABEL: define {{[^@]+}}@foo
; CHECK-SAME:    (i32* [[PTR_A:%.+]], i32* [[PTR_B:%.+]], i32 %{{.+}})
;
; CHECK:         call void @llvm.assume(i1 true) [ "align"(i32* [[PTR_A]], i64 32) ]
; CHECK:         call void @llvm.assume(i1 true) [ "align"(i32* [[PTR_B]], i64 64) ]
;
; CHECK:         [[TOK:%.+]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.ALIGNED"(i32* null, i32 32),
; CHECK-NEXT:    br label %[[LOOPBODY:[^,]+]]
; CHECK:       [[LOOPBODY]]:
; CHECK:         br i1 %{{[^,]+}}, label %[[LOOPBODY]], label %[[LOOPEXIT:[^,]+]]
; CHECK:       [[LOOPEXIT]]:
; CHECK:         call void @llvm.directive.region.exit(token [[TOK]]) [ "DIR.OMP.END.SIMD"() ]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* %A, i32* %B, i32 %N) #0 {
entry:
  %A.addr = alloca i32*, align 8
  %B.addr = alloca i32*, align 8
  %N.addr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  store i32* %A, i32** %A.addr, align 8, !tbaa !2
  store i32* %B, i32** %B.addr, align 8, !tbaa !2
  store i32 %N, i32* %N.addr, align 4, !tbaa !6
  %0 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = load i32, i32* %N.addr, align 4, !tbaa !6
  store i32 %1, i32* %.capture_expr.0, align 4, !tbaa !6
  %2 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = load i32, i32* %.capture_expr.0, align 4, !tbaa !6
  %sub = sub i32 %3, 0
  %sub1 = sub i32 %sub, 1
  %add = add i32 %sub1, 1
  %div = udiv i32 %add, 1
  %sub2 = sub i32 %div, 1
  store i32 %sub2, i32* %.capture_expr.1, align 4, !tbaa !6
  %4 = load i32, i32* %.capture_expr.0, align 4, !tbaa !6
  %cmp = icmp ult i32 0, %4
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %5 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #2
  %6 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #2
  %7 = load i32, i32* %.capture_expr.1, align 4, !tbaa !6
  store i32 %7, i32* %.omp.ub, align 4, !tbaa !6
  %8 = load i32*, i32** %A.addr, align 8, !tbaa !2
  %9 = load i32*, i32** %B.addr, align 8, !tbaa !2
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.precond.then
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.ALIGNED"(i32* %8, i32 32), "QUAL.OMP.ALIGNED"(i32* %9, i32 0), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LINEAR:IV"(i32* %I, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  store i32 0, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.SIMD.2
  %11 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %12 = load i32, i32* %.omp.ub, align 4, !tbaa !6
  %cmp3 = icmp ule i32 %11, %12
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %13 = bitcast i32* %I to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %13) #2
  %14 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %mul = mul i32 %14, 1
  %add4 = add i32 0, %mul
  store i32 %add4, i32* %I, align 4, !tbaa !6
  %15 = load i32, i32* %I, align 4, !tbaa !6
  %16 = load i32*, i32** %A.addr, align 8, !tbaa !2
  %17 = load i32, i32* %I, align 4, !tbaa !6
  %idxprom = sext i32 %17 to i64
  %ptridx = getelementptr inbounds i32, i32* %16, i64 %idxprom
  store i32 %15, i32* %ptridx, align 4, !tbaa !6
  %18 = load i32, i32* %I, align 4, !tbaa !6
  %19 = load i32*, i32** %B.addr, align 8, !tbaa !2
  %20 = load i32, i32* %I, align 4, !tbaa !6
  %idxprom5 = sext i32 %20 to i64
  %ptridx6 = getelementptr inbounds i32, i32* %19, i64 %idxprom5
  store i32 %18, i32* %ptridx6, align 4, !tbaa !6
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %21 = bitcast i32* %I to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %21) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %22 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %add7 = add nuw i32 %22, 1
  store i32 %add7, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %23 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %23) #2
  %24 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %24) #2
  %25 = bitcast i32* %.capture_expr.1 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %25) #2
  %26 = bitcast i32* %.capture_expr.0 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %26) #2
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
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
