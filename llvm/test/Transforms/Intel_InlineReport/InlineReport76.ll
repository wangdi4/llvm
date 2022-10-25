; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -passes='cgscc(inline)' -inline-report=0xe807 -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s

; CHECK: INLINE: foo{{.*}}Callee has single callsite and local linkage
; CHECK-NOT: call{{.*}}@foo

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

define dso_local i32 @bar() local_unnamed_addr {
entry:
  %A = alloca [32 x i32], align 16
  %0 = bitcast [32 x i32]* %A to i8*
  call void @llvm.lifetime.start.p0i8(i64 128, i8* nonnull %0)
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 dereferenceable(128) %0, i8 0, i64 128, i1 false)
  %arraydecay = getelementptr inbounds [32 x i32], [32 x i32]* %A, i64 0, i64 0
  call fastcc void @foo(i32* nonnull %arraydecay)
  %arrayidx = getelementptr inbounds [32 x i32], [32 x i32]* %A, i64 0, i64 16
  %1 = load i32, i32* %arrayidx, align 16
  call void @llvm.lifetime.end.p0i8(i64 128, i8* nonnull %0)
  ret i32 %1
}

define internal fastcc void @foo(i32* %A) unnamed_addr #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  %I.addr = alloca i32*, align 8
  br label %for.cond

for.cond:                                         ; preds = %omp.loop.exit.split, %entry
  %X.0 = phi i32 [ 0, %entry ], [ %inc, %omp.loop.exit.split ]
  %cmp = icmp ult i32 %X.0, 3
  br i1 %cmp, label %omp.precond.then, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

omp.precond.then:                                 ; preds = %for.cond
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0)
  %1 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  store volatile i32 31, i32* %.omp.ub, align 4
  store i32* %I, i32** %I.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.LINEAR:IV"(i32* %I, i32 1), "QUAL.OMP.OPERAND.ADDR"(i32* %I, i32** %I.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %omp.loop.exit.split, label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %omp.precond.then
  %I8 = load volatile i32*, i32** %I.addr, align 8
  store volatile i32 0, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %3 = load volatile i32, i32* %.omp.iv, align 4
  %4 = load volatile i32, i32* %.omp.ub, align 4
  %cmp4 = icmp ugt i32 %3, %4
  br i1 %cmp4, label %omp.loop.exit.split, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = bitcast i32* %I8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5)
  %6 = load volatile i32, i32* %.omp.iv, align 4
  store i32 %6, i32* %I8, align 4
  %idxprom = sext i32 %6 to i64
  %ptridx = getelementptr inbounds i32, i32* %A, i64 %idxprom
  %7 = load i32, i32* %ptridx, align 4
  %add6 = add nsw i32 %7, %6
  store i32 %add6, i32* %ptridx, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %5)
  %8 = load volatile i32, i32* %.omp.iv, align 4
  %add7 = add nuw i32 %8, 1
  store volatile i32 %add7, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond, %omp.precond.then
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  %9 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %9)
  %10 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %10)
  %inc = add nuw nsw i32 %X.0, 1
  br label %for.cond
}

attributes #0 = { "may-have-openmp-directive"="true" }

; end INTEL_FEATURE_SW_ADVANCED
