; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers -passes='cgscc(inline)' -inline-report=0xe807 -S < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -disable-output 2>&1 | FileCheck %s

; CHECK: INLINE: foo{{.*}}Callee has single callsite and local linkage
; CHECK-NOT: call{{.*}}@foo

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

define dso_local i32 @bar() local_unnamed_addr {
entry:
  %A = alloca [32 x i32], align 16
  call void @llvm.lifetime.start.p0(i64 128, ptr nonnull %A)
  call void @llvm.memset.p0.i64(ptr nonnull align 16 dereferenceable(128) %A, i8 0, i64 128, i1 false)
  %arraydecay = getelementptr inbounds [32 x i32], ptr %A, i64 0, i64 0
  call fastcc void @foo(ptr nonnull %arraydecay)
  %arrayidx = getelementptr inbounds [32 x i32], ptr %A, i64 0, i64 16
  %i1 = load i32, ptr %arrayidx, align 16
  call void @llvm.lifetime.end.p0(i64 128, ptr nonnull %A)
  ret i32 %i1
}

define internal fastcc void @foo(ptr %A) unnamed_addr #1 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %I = alloca i32, align 4
  %I.addr = alloca ptr, align 8
  br label %for.cond

for.cond:                                         ; preds = %omp.loop.exit.split, %entry
  %X.0 = phi i32 [ 0, %entry ], [ %inc, %omp.loop.exit.split ]
  %cmp = icmp ult i32 %X.0, 3
  br i1 %cmp, label %omp.precond.then, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

omp.precond.then:                                 ; preds = %for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.iv)
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %.omp.ub)
  store volatile i32 31, ptr %.omp.ub, align 4
  store ptr %I, ptr %I.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %i2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV"(ptr %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(ptr %.omp.ub), "QUAL.OMP.LINEAR:IV"(ptr %I, i32 1), "QUAL.OMP.OPERAND.ADDR"(ptr %I, ptr %I.addr), "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %omp.loop.exit.split, label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %omp.precond.then
  %I8 = load volatile ptr, ptr %I.addr, align 8
  store volatile i32 0, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.2
  %i3 = load volatile i32, ptr %.omp.iv, align 4
  %i4 = load volatile i32, ptr %.omp.ub, align 4
  %cmp4 = icmp ugt i32 %i3, %i4
  br i1 %cmp4, label %omp.loop.exit.split, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 4, ptr %I8)
  %i6 = load volatile i32, ptr %.omp.iv, align 4
  store i32 %i6, ptr %I8, align 4
  %idxprom = sext i32 %i6 to i64
  %ptridx = getelementptr inbounds i32, ptr %A, i64 %idxprom
  %i7 = load i32, ptr %ptridx, align 4
  %add6 = add nsw i32 %i7, %i6
  store i32 %add6, ptr %ptridx, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %I8)
  %i8 = load volatile i32, ptr %.omp.iv, align 4
  %add7 = add nuw i32 %i8, 1
  store volatile i32 %add7, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit.split:                              ; preds = %omp.inner.for.cond, %omp.precond.then
  call void @llvm.directive.region.exit(token %i2) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.ub)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.omp.iv)
  %inc = add nuw nsw i32 %X.0, 1
  br label %for.cond
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

attributes #0 = { nounwind }
attributes #1 = { "may-have-openmp-directive"="true" }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #3 = { argmemonly nofree nounwind willreturn writeonly }
; end INTEL_FEATURE_SW_ADVANCED
