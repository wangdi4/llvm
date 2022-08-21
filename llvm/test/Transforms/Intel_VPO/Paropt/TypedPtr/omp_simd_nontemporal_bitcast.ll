; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
; void foo(float *A, float *B, char *C, char *D, int N) {
; #pragma omp simd nontemporal(A, B, C, D)
;   for (int I = 0; I < N; ++I) {
;     *((int*) &B[I]) = *((int *) &A[I]);
;     ((int *) C)[I] = ((int *) D)[I];
;   }
; }
;
; Check that we can parse the NONTEMPORAL clause and translate it into
; !nontemporal metadata even in presence of bitcasts.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(float* %A, float* %B, i8* %C, i8* %D, i32 %N) local_unnamed_addr #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  %cmp = icmp sgt i32 %N, 0
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %sub1 = add nsw i32 %N, -1
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  %1 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  store volatile i32 %sub1, i32* %.omp.ub, align 4, !tbaa !2
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %omp.precond.then
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NONTEMPORAL"(float* %A, float* %B, i8* %C, i8* %D), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LINEAR:IV"(i32* %I, i32 1), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  br label %DIR.OMP.SIMD.221

DIR.OMP.SIMD.221:                                 ; preds = %DIR.OMP.SIMD.1
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %omp.loop.exit.split, label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.221
  store volatile i32 0, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %3 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %4 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp3 = icmp sgt i32 %3, %4
  br i1 %cmp3, label %omp.loop.exit.split, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = bitcast i32* %I to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #2
  %6 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  store i32 %6, i32* %I, align 4, !tbaa !2
  %idxprom = sext i32 %6 to i64
  %ptridx = getelementptr inbounds float, float* %A, i64 %idxprom
  %7 = bitcast float* %ptridx to i32*
  ; CHECK: [[LD1:%.*]] = load i32, i32* %{{.*}}, align 4, !tbaa !2, !nontemporal
  %8 = load i32, i32* %7, align 4, !tbaa !2
  %ptridx6 = getelementptr inbounds float, float* %B, i64 %idxprom
  %9 = bitcast float* %ptridx6 to i32*
  ; CHECK: store i32 [[LD1]], i32* %{{.*}}, align 4, !tbaa !2, !nontemporal
  store i32 %8, i32* %9, align 4, !tbaa !2
  %10 = bitcast i8* %D to i32*
  %11 = load i32, i32* %I, align 4, !tbaa !2
  %idxprom7 = sext i32 %11 to i64
  %ptridx8 = getelementptr inbounds i32, i32* %10, i64 %idxprom7
  ; CHECK: [[LD2:%.*]] = load i32, i32* %ptridx8, align 4, !tbaa !2, !nontemporal
  %12 = load i32, i32* %ptridx8, align 4, !tbaa !2
  %13 = bitcast i8* %C to i32*
  %ptridx10 = getelementptr inbounds i32, i32* %13, i64 %idxprom7
  ; CHECK: store i32 [[LD2]], i32* %ptridx10, align 4, !tbaa !2, !nontemporal
  store i32 %12, i32* %ptridx10, align 4, !tbaa !2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %5) #2
  %14 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %add11 = add nsw i32 %14, 1
  store volatile i32 %add11, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond, %DIR.OMP.SIMD.221
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  %15 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %15) #2
  %16 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %16) #2
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

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
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
