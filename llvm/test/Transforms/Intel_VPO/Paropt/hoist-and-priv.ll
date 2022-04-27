; RUN: opt -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s

; CHECK: define{{.*}}split
; CHECK:  %Array.i = alloca [2050 x i32], align 16
; CHECK: call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %i.priv, i32 1), "QUAL.OMP.PRIVATE"([2050 x i32]* %Array.i) ]
; CHECK-NOT: %Array.i = alloca [2050 x i32], align 16

; After inlining, the alloca for Array is placed inside the SIMD loop. It
; needs to be hoisted out of the loop, and marked private in the SIMD
; directive.

; void bar(int *);
; static int foo(int i) {
;   int Array[2050];
;   bar(Array);
;   return Array[i];
; }

; void test() {
;   int Out[1024];
;   #pragma omp parallel for simd
;   for (int i = 0; i < 1024; ++i) {
;     Out[i] = foo(i);
;   }
;   bar(Out);
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: uwtable
define dso_local void @_Z4testv() local_unnamed_addr  {
entry:
  %Out = alloca [1024 x i32], align 16
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = bitcast [1024 x i32]* %Out to i8*
  call void @llvm.lifetime.start.p0i8(i64 4096, i8* nonnull %0)
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  %2 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2)
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %3 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3)
  store volatile i32 1023, i32* %.omp.ub, align 4, !tbaa !2
  %i.addr = alloca i32*, align 8
  %i.addr5 = alloca i32*, align 8
  %.omp.lb.addr = alloca i32*, align 8
  %Out.addr = alloca [1024 x i32]*, align 8
  store i32* %i, i32** %i.addr5, align 8
  store i32* %.omp.lb, i32** %.omp.lb.addr, align 8
  store [1024 x i32]* %Out, [1024 x i32]** %Out.addr, align 8
  %end.dir.temp9 = alloca i1, align 1
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"([1024 x i32]* %Out), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.OPERAND.ADDR"(i32* %i, i32** %i.addr5), "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb, i32** %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"([1024 x i32]* %Out, [1024 x i32]** %Out.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp9) ]
  %temp.load10 = load volatile i1, i1* %end.dir.temp9, align 1
  br i1 %temp.load10, label %DIR.OMP.END.SIMD.5.split, label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %entry
  %i6 = load volatile i32*, i32** %i.addr5, align 8
  %.omp.lb7 = load volatile i32*, i32** %.omp.lb.addr, align 8
  %Out8 = load volatile [1024 x i32]*, [1024 x i32]** %Out.addr, align 8
  store i32* %i6, i32** %i.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %i6, i32 1), "QUAL.OMP.OPERAND.ADDR"(i32* %i6, i32** %i.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %omp.loop.exit.split, label %DIR.OMP.SIMD.4

DIR.OMP.SIMD.4:                                   ; preds = %DIR.OMP.PARALLEL.LOOP.3
  %i3 = load volatile i32*, i32** %i.addr, align 8
  %6 = load i32, i32* %.omp.lb7, align 4, !tbaa !2
  store volatile i32 %6, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.4
  %7 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %8 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp.not = icmp sgt i32 %7, %8
  br i1 %cmp.not, label %omp.loop.exit.split, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = bitcast i32* %i3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9)
  %10 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  store i32 %10, i32* %i3, align 4, !tbaa !2
  %Array.i = alloca [2050 x i32], align 16
  %11 = bitcast [2050 x i32]* %Array.i to i8*
  call void @llvm.lifetime.start.p0i8(i64 8200, i8* nonnull %11)
  %arraydecay.i = getelementptr inbounds [2050 x i32], [2050 x i32]* %Array.i, i64 0, i64 0
  call void @_Z3barPi(i32* nonnull %arraydecay.i)
  %idxprom.i = sext i32 %10 to i64
  %arrayidx.i = getelementptr inbounds [2050 x i32], [2050 x i32]* %Array.i, i64 0, i64 %idxprom.i, !intel-tbaa !6
  %12 = load i32, i32* %arrayidx.i, align 4, !tbaa !6
  call void @llvm.lifetime.end.p0i8(i64 8200, i8* nonnull %11)
  %13 = load i32, i32* %i3, align 4, !tbaa !2
  %idxprom = sext i32 %13 to i64
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* %Out8, i64 0, i64 %idxprom, !intel-tbaa !8
  store i32 %12, i32* %arrayidx, align 4, !tbaa !8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %9)
  %14 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %add1 = add nsw i32 %14, 1
  store volatile i32 %add1, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond, %DIR.OMP.PARALLEL.LOOP.3
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.5.split

DIR.OMP.END.SIMD.5.split:                         ; preds = %entry, %omp.loop.exit.split
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1)
  %arraydecay = getelementptr inbounds [1024 x i32], [1024 x i32]* %Out, i64 0, i64 0
  call void @_Z3barPi(i32* nonnull %arraydecay)
  call void @llvm.lifetime.end.p0i8(i64 4096, i8* nonnull %0)
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

declare dso_local void @_Z3barPi(i32*) local_unnamed_addr

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA2050_i", !3, i64 0}
!8 = !{!9, !3, i64 0}
!9 = !{!"array@_ZTSA1024_i", !3, i64 0}
