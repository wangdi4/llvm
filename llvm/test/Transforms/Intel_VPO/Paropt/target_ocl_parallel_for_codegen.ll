; RUN: opt < %s -switch-to-offload=true -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s

; The test case checks whether the omp parlalel loop is partitioned as expected
; on top of OCL.
;
; #define MAX 10000
;
; int A[MAX], B[MAX], C[MAX];
;
; void __attribute__ ((noinline)) Compute(int n)
; {
;      #pragma omp target map(tofrom: A, B, C) map(to: n)
;      {
;            int i;
;            #pragma omp parallel for
;            for (i = 1; i < n; i++) {
;               A[i] = B[i] + C[i];
;            }
;       }
; }

target triple = "spir64"
target device_triples = "spir64"

@A = external dso_local global [100 x [100 x i32]], align 4
@B = external dso_local global [100 x [100 x i32]], align 4
@C = external dso_local global [100 x [100 x i32]], align 4
@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local spir_func void @Compute() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.MAP.TO"([100 x [100 x i32]]* @A), "QUAL.OMP.MAP.TO"([100 x [100 x i32]]* @B), "QUAL.OMP.MAP.TOFROM"([100 x [100 x i32]]* @C), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.stride), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  store i32 0, i32* %.omp.lb, align 4
  store volatile i32 99, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
  br label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.TARGET.1
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.SHARED"([100 x [100 x i32]]* @A), "QUAL.OMP.SHARED"([100 x [100 x i32]]* @B), "QUAL.OMP.SHARED"([100 x [100 x i32]]* @C) ]
  %2 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @A to i8*))
  %3 = bitcast i8* %2 to [100 x [100 x i32]]*
  %4 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @B to i8*))
  %5 = bitcast i8* %4 to [100 x [100 x i32]]*
  %6 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @C to i8*))
  %7 = bitcast i8* %6 to [100 x [100 x i32]]*
  %8 = load i32, i32* %.omp.lb, align 4
  store volatile i32 %8, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.PARALLEL.LOOP.3
  %9 = load volatile i32, i32* %.omp.iv, align 4
  %10 = load volatile i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %9, %10
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = load volatile i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %11, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  store i32 0, i32* %j, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc17, %omp.inner.for.body
  %12 = load i32, i32* %j, align 4
  %cmp1 = icmp slt i32 %12, 100
  br i1 %cmp1, label %for.body, label %omp.inner.for.inc

for.body:                                         ; preds = %for.cond
  store i32 0, i32* %k, align 4
  br label %for.cond2

for.cond2:                                        ; preds = %for.body4, %for.body
  %13 = load i32, i32* %k, align 4
  %cmp3 = icmp slt i32 %13, 100
  br i1 %cmp3, label %for.body4, label %for.inc17

for.body4:                                        ; preds = %for.cond2
  %14 = load i32, i32* %i, align 4
  %idxprom = sext i32 %14 to i64
  %arrayidx = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %3, i64 0, i64 %idxprom
  %15 = load i32, i32* %k, align 4
  %idxprom5 = sext i32 %15 to i64
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx, i64 0, i64 %idxprom5
  %16 = load i32, i32* %arrayidx6, align 4
  %17 = load i32, i32* %k, align 4
  %idxprom7 = sext i32 %17 to i64
  %arrayidx8 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %5, i64 0, i64 %idxprom7
  %18 = load i32, i32* %j, align 4
  %idxprom9 = sext i32 %18 to i64
  %arrayidx10 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx8, i64 0, i64 %idxprom9
  %19 = load i32, i32* %arrayidx10, align 4
  %mul11 = mul nsw i32 %16, %19
  %20 = load i32, i32* %i, align 4
  %idxprom12 = sext i32 %20 to i64
  %arrayidx13 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %7, i64 0, i64 %idxprom12
  %21 = load i32, i32* %j, align 4
  %idxprom14 = sext i32 %21 to i64
  %arrayidx15 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx13, i64 0, i64 %idxprom14
  %22 = load i32, i32* %arrayidx15, align 4
  %add16 = add nsw i32 %22, %mul11
  store i32 %add16, i32* %arrayidx15, align 4
  %23 = load i32, i32* %k, align 4
  %inc = add nsw i32 %23, 1
  store i32 %inc, i32* %k, align 4
  br label %for.cond2

for.inc17:                                        ; preds = %for.cond2
  %24 = load i32, i32* %j, align 4
  %inc18 = add nsw i32 %24, 1
  store i32 %inc18, i32* %j, align 4
  br label %for.cond

omp.inner.for.inc:                                ; preds = %for.cond
  %25 = load volatile i32, i32* %.omp.iv, align 4
  %add20 = add nsw i32 %25, 1
  store volatile i32 %add20, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %DIR.OMP.END.PARALLEL.LOOP.5

DIR.OMP.END.PARALLEL.LOOP.5:                      ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: inaccessiblememonly nounwind speculatable
declare i8* @llvm.launder.invariant.group.p0i8(i8*) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { inaccessiblememonly nounwind speculatable }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 58, i32 -1939105753, !"Compute", i32 11, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 8.0.0"}
!4 = distinct !{i32 0}

; CHECK: %{{.*}} = call i64 @_Z14get_local_sizej(i32 0)
; CHECK: %{{.*}} = call i64 @_Z12get_local_idj(i32 0)
