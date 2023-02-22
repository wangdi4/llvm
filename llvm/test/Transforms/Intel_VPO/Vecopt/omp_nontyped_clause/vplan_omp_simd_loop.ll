; RUN: opt -opaque-pointers=0 %s -S -passes='function(vpo-cfg-restructuring),vpo-paropt,vplan-vec' 2>&1 | FileCheck %s
; Verify that VPlan can vertorize the OMP simd loop.
;
; CHECK: add nsw <4 x i32>

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local void @foo(i32* %arr, i32 %n) #0 {
entry:
  %arr.addr = alloca i32*, align 8
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32* %arr, i32** %arr.addr, align 8, !tbaa !2
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %sub2 = sub nsw i32 %n, 1
  %cmp = icmp slt i32 0, %n
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !6
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store i32 %sub2, i32* %.omp.ub, align 4, !tbaa !6
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %omp.precond.then
  br label %DIR.OMP.PARALLEL.LOOP.2

DIR.OMP.PARALLEL.LOOP.2:                          ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
  %4 = load i32, i32* %.omp.lb, align 4, !tbaa !6
  store volatile i32 %4, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.PARALLEL.LOOP.2
  %5 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !6
  %6 = load i32, i32* %.omp.ub, align 4, !tbaa !6
  %cmp4 = icmp sle i32 %5, %6
  br i1 %cmp4, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !6
  %8 = load i32*, i32** %arr.addr, align 8, !tbaa !2
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds i32, i32* %8, i64 %idxprom
  store i32 %7, i32* %arrayidx, align 4, !tbaa !6
  call void (...) @baz()
  %9 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !6
  %add6 = add nsw i32 %9, 1
  store volatile i32 %add6, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %omp.loop.exit
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.4, %entry
  %10 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %10) #2
  %11 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local void @baz(...) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 064530e58b0615f7d4d7407a0cbdc32feaf6d8c1) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 18fdc69bd9a4e2d3ea4049be99ce3b6bc82cc39d)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
