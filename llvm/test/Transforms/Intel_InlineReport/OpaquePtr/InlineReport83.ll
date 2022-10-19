; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -dtrans-inline-heuristics -intel-libirc-allowed -inline -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers < %s -dtrans-inline-heuristics -intel-libirc-allowed -passes='cgscc(inline)' -inline-report=0xe807 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-CL
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -dtrans-inline-heuristics -intel-libirc-allowed -inline -inline-report=0xe886 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -dtrans-inline-heuristics -intel-libirc-allowed -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-MD

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

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #1 {
entry:
  %a = alloca [1000 x i32], align 16
  call void @llvm.lifetime.start.p0(i64 4000, ptr nonnull %a) #0
  %arraydecay = getelementptr inbounds [1000 x i32], ptr %a, i64 0, i64 0
  %call = call i32 @foo(ptr nonnull %arraydecay, i32 1000)
  call void @llvm.lifetime.end.p0(i64 4000, ptr nonnull %a) #0
  ret i32 0
}

; Function Attrs: nounwind uwtable
define internal i32 @foo(ptr %a, i32 %N) local_unnamed_addr #2 {
DIR.OMP.PARALLEL.323:
  %a.addr = alloca ptr, align 8
  %N.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store ptr %a, ptr %a.addr, align 8
  store i32 %N, ptr %N.addr, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i) #0
  %i.addr = alloca ptr, align 8
  %.omp.lb.addr = alloca ptr, align 8
  %.capture_expr.1.addr = alloca ptr, align 8
  %.omp.iv.addr = alloca ptr, align 8
  %.omp.lb.addr11 = alloca ptr, align 8
  %.omp.ub.addr = alloca ptr, align 8
  %.capture_expr.0.addr = alloca ptr, align 8
  %tmp.addr = alloca ptr, align 8
  %a.addr.addr = alloca ptr, align 8
  %N.addr.addr = alloca ptr, align 8
  %i.addr17 = alloca ptr, align 8
  store ptr %.capture_expr.1, ptr %.capture_expr.1.addr, align 8
  store ptr %.omp.iv, ptr %.omp.iv.addr, align 8
  store ptr %.omp.lb, ptr %.omp.lb.addr11, align 8
  store ptr %.omp.ub, ptr %.omp.ub.addr, align 8
  store ptr %.capture_expr.0, ptr %.capture_expr.0.addr, align 8
  store ptr %tmp, ptr %tmp.addr, align 8
  store ptr %a.addr, ptr %a.addr.addr, align 8
  store ptr %N.addr, ptr %N.addr.addr, align 8
  store ptr %i, ptr %i.addr17, align 8
  %cmp.new = icmp sgt i32 %N, 50
  br i1 %cmp.new, label %if.then, label %if.end

if.then:                                          ; preds = %DIR.OMP.PARALLEL.323
  store i32 40, ptr %N.addr, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %DIR.OMP.PARALLEL.323
  %end.dir.temp20 = alloca i1, align 1
  %i2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(ptr %a.addr), "QUAL.OMP.SHARED"(ptr %N.addr), "QUAL.OMP.SHARED"(ptr %i), "QUAL.OMP.PRIVATE"(ptr %.capture_expr.1), "QUAL.OMP.PRIVATE"(ptr %.omp.iv), "QUAL.OMP.PRIVATE"(ptr %.omp.lb), "QUAL.OMP.PRIVATE"(ptr %.omp.ub), "QUAL.OMP.PRIVATE"(ptr %.capture_expr.0), "QUAL.OMP.PRIVATE"(ptr %tmp), "QUAL.OMP.OPERAND.ADDR"(ptr %.capture_expr.1, ptr %.capture_expr.1.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.iv, ptr %.omp.iv.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb, ptr %.omp.lb.addr11), "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.ub, ptr %.omp.ub.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %.capture_expr.0, ptr %.capture_expr.0.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %tmp, ptr %tmp.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %a.addr, ptr %a.addr.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %N.addr, ptr %N.addr.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %i, ptr %i.addr17), "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp20) ]
  %temp.load21 = load volatile i1, ptr %end.dir.temp20, align 1
  br i1 %temp.load21, label %DIR.OMP.END.PARALLEL.8, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %if.end
  %.capture_expr.19 = load volatile ptr, ptr %.capture_expr.1.addr, align 8
  %.omp.iv10 = load volatile ptr, ptr %.omp.iv.addr, align 8
  %.omp.lb12 = load volatile ptr, ptr %.omp.lb.addr11, align 8
  %.omp.ub13 = load volatile ptr, ptr %.omp.ub.addr, align 8
  %.capture_expr.014 = load volatile ptr, ptr %.capture_expr.0.addr, align 8
  %a.addr15 = load volatile ptr, ptr %a.addr.addr, align 8
  %N.addr16 = load volatile ptr, ptr %N.addr.addr, align 8
  %i18 = load volatile ptr, ptr %i.addr17, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.014) #0
  %i4 = load i32, ptr %N.addr16, align 4
  store i32 %i4, ptr %.capture_expr.014, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.capture_expr.19) #0
  %i6 = load i32, ptr %.capture_expr.014, align 4
  %sub1 = add nsw i32 %i6, -1
  store i32 %sub1, ptr %.capture_expr.19, align 4
  %i8 = load i32, ptr %.capture_expr.014, align 4
  %cmp = icmp sgt i32 %i8, 0
  br i1 %cmp, label %DIR.OMP.LOOP.524, label %omp.precond.end

DIR.OMP.LOOP.524:                                 ; preds = %DIR.OMP.PARALLEL.3
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.iv10) #0
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.lb12) #0
  store i32 0, ptr %.omp.lb12, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %.omp.ub13) #0
  %i12 = load i32, ptr %.capture_expr.19, align 4
  store volatile i32 %i12, ptr %.omp.ub13, align 4
  store ptr %i18, ptr %i.addr, align 8
  store ptr %.omp.lb12, ptr %.omp.lb.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %i13 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.PRIVATE"(ptr %i18), "QUAL.OMP.NORMALIZED.IV"(ptr %.omp.iv10), "QUAL.OMP.FIRSTPRIVATE"(ptr %.omp.lb12), "QUAL.OMP.NORMALIZED.UB"(ptr %.omp.ub13), "QUAL.OMP.OPERAND.ADDR"(ptr %i18, ptr %i.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb12, ptr %.omp.lb.addr), "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.LOOP.6, label %DIR.OMP.LOOP.5

DIR.OMP.LOOP.5:                                   ; preds = %DIR.OMP.LOOP.524
  %i7 = load volatile ptr, ptr %i.addr, align 8
  %.omp.lb8 = load volatile ptr, ptr %.omp.lb.addr, align 8
  %i14 = load i32, ptr %.omp.lb8, align 4
  store volatile i32 %i14, ptr %.omp.iv10, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.LOOP.5
  %i15 = load volatile i32, ptr %.omp.iv10, align 4
  %i16 = load volatile i32, ptr %.omp.ub13, align 4
  %cmp3.not = icmp sgt i32 %i15, %i16
  br i1 %cmp3.not, label %DIR.OMP.END.LOOP.6, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %i17 = load volatile i32, ptr %.omp.iv10, align 4
  store i32 %i17, ptr %i7, align 4
  %i19 = load ptr, ptr %a.addr15, align 8
  %idxprom = sext i32 %i17 to i64
  %ptridx = getelementptr inbounds i32, ptr %i19, i64 %idxprom
  store i32 15, ptr %ptridx, align 4
  %i20 = load volatile i32, ptr %.omp.iv10, align 4
  %add5 = add nsw i32 %i20, 1
  store volatile i32 %add5, ptr %.omp.iv10, align 4
  br label %omp.inner.for.cond

DIR.OMP.END.LOOP.6:                               ; preds = %omp.inner.for.cond, %DIR.OMP.LOOP.524
  call void @llvm.directive.region.exit(token %i13) [ "DIR.OMP.END.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.LOOP.6, %DIR.OMP.PARALLEL.3
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.ub13) #0
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.lb12) #0
  call void @llvm.lifetime.end.p0(i64 4, ptr %.omp.iv10) #0
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.capture_expr.19) #0
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %.capture_expr.014) #0
  br label %DIR.OMP.END.PARALLEL.8

DIR.OMP.END.PARALLEL.8:                           ; preds = %omp.precond.end, %if.end
  call void @llvm.directive.region.exit(token %i2) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i) #0
  ret i32 0
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

attributes #0 = { nounwind }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { argmemonly nocallback nofree nosync nounwind willreturn }
; end INTEL_FEATURE_SW_ADVANCED
