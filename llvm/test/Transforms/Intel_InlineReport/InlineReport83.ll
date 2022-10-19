; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -dtrans-inline-heuristics -intel-libirc-allowed -inline -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt < %s -dtrans-inline-heuristics -intel-libirc-allowed -passes='cgscc(inline)' -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -dtrans-inline-heuristics -intel-libirc-allowed -inline -inline-report=0xe886 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

target triple = "x86_64-unknown-linux-gnu"

; Check that foo is inlined when -dtrans-inline-heuristics -intel-libirc-allowed is set, despite
; the existence of dynamic alloca %end.dir.temp20

; CHECK-MD: COMPILE FUNC: main
; CHECK-MD: INLINE: foo{{.*}}Callee has single callsite and local linkage
; CHECK: define dso_local i32 @main()
; CHECK-NOT: call i32 @foo
; CHECK-NOT: define internal i32 @foo({{.*}})
; CHECK-CL: COMPILE FUNC: main
; CHECK-CL: INLINE: foo{{.*}}Callee has single callsite and local linkage

declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

declare token @llvm.directive.region.entry() #2

declare void @llvm.directive.region.exit(token) #2

declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  %a = alloca [1000 x i32], align 16
  %0 = bitcast [1000 x i32]* %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 4000, i8* nonnull %0) #2
  %arraydecay = getelementptr inbounds [1000 x i32], [1000 x i32]* %a, i64 0, i64 0
  %call = call i32 @foo(i32* nonnull %arraydecay, i32 1000)
  call void @llvm.lifetime.end.p0i8(i64 4000, i8* nonnull %0) #2
  ret i32 0
}

define internal i32 @foo(i32* %a, i32 %N) local_unnamed_addr #0 {
DIR.OMP.PARALLEL.323:
  %a.addr = alloca i32*, align 8
  %N.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32* %a, i32** %a.addr, align 8
  store i32 %N, i32* %N.addr, align 4
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #2
  %i.addr = alloca i32*, align 8
  %.omp.lb.addr = alloca i32*, align 8
  %.capture_expr.1.addr = alloca i32*, align 8
  %.omp.iv.addr = alloca i32*, align 8
  %.omp.lb.addr11 = alloca i32*, align 8
  %.omp.ub.addr = alloca i32*, align 8
  %.capture_expr.0.addr = alloca i32*, align 8
  %tmp.addr = alloca i32*, align 8
  %a.addr.addr = alloca i32**, align 8
  %N.addr.addr = alloca i32*, align 8
  %i.addr17 = alloca i32*, align 8
  store i32* %.capture_expr.1, i32** %.capture_expr.1.addr, align 8
  store i32* %.omp.iv, i32** %.omp.iv.addr, align 8
  store i32* %.omp.lb, i32** %.omp.lb.addr11, align 8
  store i32* %.omp.ub, i32** %.omp.ub.addr, align 8
  store i32* %.capture_expr.0, i32** %.capture_expr.0.addr, align 8
  store i32* %tmp, i32** %tmp.addr, align 8
  store i32** %a.addr, i32*** %a.addr.addr, align 8
  store i32* %N.addr, i32** %N.addr.addr, align 8
  store i32* %i, i32** %i.addr17, align 8
  %cmp.new = icmp sgt i32 %N, 50
  br i1 %cmp.new, label %if.then, label %if.end

if.then:
  store i32 40, i32* %N.addr, align 4
  br label %if.end

if.end:
  %end.dir.temp20 = alloca i1, align 1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32** %a.addr), "QUAL.OMP.SHARED"(i32* %N.addr), "QUAL.OMP.SHARED"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.1), "QUAL.OMP.PRIVATE"(i32* %.omp.iv), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %.capture_expr.0), "QUAL.OMP.PRIVATE"(i32* %tmp), "QUAL.OMP.OPERAND.ADDR"(i32* %.capture_expr.1, i32** %.capture_expr.1.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.iv, i32** %.omp.iv.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb, i32** %.omp.lb.addr11), "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.ub, i32** %.omp.ub.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %.capture_expr.0, i32** %.capture_expr.0.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %tmp, i32** %tmp.addr), "QUAL.OMP.OPERAND.ADDR"(i32** %a.addr, i32*** %a.addr.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %N.addr, i32** %N.addr.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %i, i32** %i.addr17), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp20) ]
  %temp.load21 = load volatile i1, i1* %end.dir.temp20, align 1
  br i1 %temp.load21, label %DIR.OMP.END.PARALLEL.8, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.323
  %.capture_expr.19 = load volatile i32*, i32** %.capture_expr.1.addr, align 8
  %.omp.iv10 = load volatile i32*, i32** %.omp.iv.addr, align 8
  %.omp.lb12 = load volatile i32*, i32** %.omp.lb.addr11, align 8
  %.omp.ub13 = load volatile i32*, i32** %.omp.ub.addr, align 8
  %.capture_expr.014 = load volatile i32*, i32** %.capture_expr.0.addr, align 8
  %a.addr15 = load volatile i32**, i32*** %a.addr.addr, align 8
  %N.addr16 = load volatile i32*, i32** %N.addr.addr, align 8
  %i18 = load volatile i32*, i32** %i.addr17, align 8
  %2 = bitcast i32* %.capture_expr.014 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = load i32, i32* %N.addr16, align 4
  store i32 %3, i32* %.capture_expr.014, align 4
  %4 = bitcast i32* %.capture_expr.19 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #2
  %5 = load i32, i32* %.capture_expr.014, align 4
  %sub1 = add nsw i32 %5, -1
  store i32 %sub1, i32* %.capture_expr.19, align 4
  %6 = load i32, i32* %.capture_expr.014, align 4
  %cmp = icmp sgt i32 %6, 0
  br i1 %cmp, label %DIR.OMP.LOOP.524, label %omp.precond.end

DIR.OMP.LOOP.524:                                 ; preds = %DIR.OMP.PARALLEL.3
  %7 = bitcast i32* %.omp.iv10 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #2
  %8 = bitcast i32* %.omp.lb12 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %8) #2
  store i32 0, i32* %.omp.lb12, align 4
  %9 = bitcast i32* %.omp.ub13 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #2
  %10 = load i32, i32* %.capture_expr.19, align 4
  store volatile i32 %10, i32* %.omp.ub13, align 4
  store i32* %i18, i32** %i.addr, align 8
  store i32* %.omp.lb12, i32** %.omp.lb.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.PRIVATE"(i32* %i18), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv10), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb12), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub13), "QUAL.OMP.OPERAND.ADDR"(i32* %i18, i32** %i.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb12, i32** %.omp.lb.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.LOOP.6, label %DIR.OMP.LOOP.5

DIR.OMP.LOOP.5:                                   ; preds = %DIR.OMP.LOOP.524
  %i7 = load volatile i32*, i32** %i.addr, align 8
  %.omp.lb8 = load volatile i32*, i32** %.omp.lb.addr, align 8
  %12 = load i32, i32* %.omp.lb8, align 4
  store volatile i32 %12, i32* %.omp.iv10, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.5
  %13 = load volatile i32, i32* %.omp.iv10, align 4
  %14 = load volatile i32, i32* %.omp.ub13, align 4
  %cmp3.not = icmp sgt i32 %13, %14
  br i1 %cmp3.not, label %DIR.OMP.END.LOOP.6, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %15 = load volatile i32, i32* %.omp.iv10, align 4
  store i32 %15, i32* %i7, align 4
  %16 = load i32*, i32** %a.addr15, align 8
  %idxprom = sext i32 %15 to i64
  %ptridx = getelementptr inbounds i32, i32* %16, i64 %idxprom
  store i32 15, i32* %ptridx, align 4
  %17 = load volatile i32, i32* %.omp.iv10, align 4
  %add5 = add nsw i32 %17, 1
  store volatile i32 %add5, i32* %.omp.iv10, align 4
  br label %omp.inner.for.cond

DIR.OMP.END.LOOP.6:                               ; preds = %DIR.OMP.LOOP.524, %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.LOOP.6, %DIR.OMP.PARALLEL.3
  %18 = bitcast i32* %.omp.ub13 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #2
  %19 = bitcast i32* %.omp.lb12 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #2
  %20 = bitcast i32* %.omp.iv10 to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %4) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #2
  br label %DIR.OMP.END.PARALLEL.8

DIR.OMP.END.PARALLEL.8:                           ; preds = %DIR.OMP.PARALLEL.323, %omp.precond.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #2
  ret i32 0
}

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
; end INTEL_FEATURE_SW_ADVANCED
