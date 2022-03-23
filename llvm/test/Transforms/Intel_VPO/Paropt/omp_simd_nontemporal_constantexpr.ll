; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; float Ary[1024];
; int Buf[1024];
; int *Ptr = &Buf[128];
;
; void foo() {
; #pragma omp simd nontemporal(Ary, Buf)
;   for (int I = 0; I < 512; ++I)
;     (&Ary[256])[I] = ((float *) Buf)[I] + 1.0;
; }
;
; Check that we can parse the NONTEMPORAL clause and translate it into
; !nontemporal metadata even in presence of constant expressions (e.g.
; constant bitcast or getelementptr).
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@Ary = dso_local global [1024 x float] zeroinitializer, align 16
@Buf = dso_local global [1024 x i32] zeroinitializer, align 16
@Ptr = dso_local local_unnamed_addr global i32* getelementptr inbounds ([1024 x i32], [1024 x i32]* @Buf, i64 0, i64 128), align 8

; Function Attrs: nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  %1 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #2
  store volatile i32 511, i32* %.omp.ub, align 4
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NONTEMPORAL"([1024 x float]* @Ary, [1024 x i32]* @Buf), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LINEAR:IV"(i32* %I, i32 1), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  br label %DIR.OMP.SIMD.29

DIR.OMP.SIMD.29:                                  ; preds = %DIR.OMP.SIMD.1
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %omp.loop.exit.split, label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.29
  store volatile i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %3 = load volatile i32, i32* %.omp.iv, align 4
  %4 = load volatile i32, i32* %.omp.ub, align 4
  %cmp = icmp sgt i32 %3, %4
  br i1 %cmp, label %omp.loop.exit.split, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = bitcast i32* %I to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #2
  %6 = load volatile i32, i32* %.omp.iv, align 4
  store i32 %6, i32* %I, align 4
  %idxprom = sext i32 %6 to i64
  %ptridx8 = getelementptr inbounds [1024 x float], [1024 x float]* bitcast ([1024 x i32]* @Buf to [1024 x float]*), i64 0, i64 %idxprom
  ; CHECK: load float, float* %ptridx8, align 4, !nontemporal
  %7 = load float, float* %ptridx8, align 4
  %conv2 = fadd fast float %7, 1.000000e+00
  %ptridx4 = getelementptr inbounds float, float* getelementptr inbounds ([1024 x float], [1024 x float]* @Ary, i64 0, i64 256), i64 %idxprom
  ; CHECK: store float %conv2, float* %ptridx4, align 4, !nontemporal
  store float %conv2, float* %ptridx4, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %5) #2
  %8 = load volatile i32, i32* %.omp.iv, align 4
  %add5 = add nsw i32 %8, 1
  store volatile i32 %add5, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond, %DIR.OMP.SIMD.29
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
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

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
