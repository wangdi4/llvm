; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
;
; It tests whether compiler generates the codo to pass the loop description
; information into the runtime library.
;
; #define MAX 100
;
; int A[MAX][MAX], B[MAX][MAX], C[MAX][MAX];
;
; void __attribute__ ((noinline)) Compute()
; {
;   #pragma omp target map(to: A, B) map(tofrom: C)
;   {
;     #pragma omp parallel for collapse(2)
;     for (int i = 0; i < MAX; i++)
;       for (int j = 0; j < MAX; j++)
;         for (int k = 0; k < MAX; k++)
;           C[i][j] += A[i][k] * B[k][j];
;   }
; }

target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@A = common dso_local global [100 x [100 x i32]] zeroinitializer, align 16
@B = common dso_local global [100 x [100 x i32]] zeroinitializer, align 16
@C = common dso_local global [100 x [100 x i32]] zeroinitializer, align 16
@.str = private unnamed_addr constant [10 x i8] c"%d %d %d\0A\00", align 1
@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind uwtable
define dso_local void @Compute() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  %.omp.iv1 = alloca i32, align 4
  %.omp.lb3 = alloca i32, align 4
  %.omp.ub4 = alloca i32, align 4
  %.omp.stride5 = alloca i32, align 4
  %.omp.is_last6 = alloca i32, align 4
  %j = alloca i32, align 4
  %k = alloca i32, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.MAP.TO"([100 x [100 x i32]]* @A), "QUAL.OMP.MAP.TO"([100 x [100 x i32]]* @B), "QUAL.OMP.MAP.TOFROM"([100 x [100 x i32]]* @C), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.stride), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last), "QUAL.OMP.PRIVATE"(i32* %.omp.lb3), "QUAL.OMP.PRIVATE"(i32* %.omp.stride5), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last6), "QUAL.OMP.PRIVATE"(i32* %k) ], !omp_offload.entry !10
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #1
  %2 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #1
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %3 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #1
  store volatile i32 99, i32* %.omp.ub, align 4, !tbaa !2
  %4 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #1
  store i32 1, i32* %.omp.stride, align 4, !tbaa !2
  %5 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #1
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !2
  br label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.TARGET.1
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv, i32* %.omp.iv1), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub, i32* %.omp.ub4), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %.omp.lb3), "QUAL.OMP.PRIVATE"(i32* %.omp.stride5), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last6), "QUAL.OMP.PRIVATE"(i32* %k), "QUAL.OMP.SHARED"([100 x [100 x i32]]* @A), "QUAL.OMP.SHARED"([100 x [100 x i32]]* @B), "QUAL.OMP.SHARED"([100 x [100 x i32]]* @C) ]
  %7 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @A to i8*))
  %8 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @B to i8*))
  %9 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @C to i8*))
  %10 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store volatile i32 %10, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.loop.exit, %DIR.OMP.PARALLEL.LOOP.3
  %11 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %12 = load volatile i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp = icmp sle i32 %11, %12
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit30

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %13 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %13) #1
  %14 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  store i32 %14, i32* %i, align 4, !tbaa !2
  %15 = bitcast i32* %.omp.iv1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %15) #1
  %16 = bitcast i32* %.omp.lb3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %16) #1
  store i32 0, i32* %.omp.lb3, align 4, !tbaa !2
  %17 = bitcast i32* %.omp.ub4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %17) #1
  store volatile i32 99, i32* %.omp.ub4, align 4, !tbaa !2
  %18 = bitcast i32* %.omp.stride5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %18) #1
  store i32 1, i32* %.omp.stride5, align 4, !tbaa !2
  %19 = bitcast i32* %.omp.is_last6 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %19) #1
  store i32 0, i32* %.omp.is_last6, align 4, !tbaa !2
  br label %DIR.OMP.PARALLEL.LOOP.5

DIR.OMP.PARALLEL.LOOP.5:                          ; preds = %omp.inner.for.body
  %20 = alloca i32
  %21 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @A to i8*))
  %22 = bitcast i8* %21 to [100 x [100 x i32]]*
  %23 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @B to i8*))
  %24 = bitcast i8* %23 to [100 x [100 x i32]]*
  %25 = call i8* @llvm.launder.invariant.group.p0i8(i8* bitcast ([100 x [100 x i32]]* @C to i8*))
  %26 = bitcast i8* %25 to [100 x [100 x i32]]*
  %27 = load i32, i32* %.omp.lb3, align 4, !tbaa !2
  store volatile i32 %27, i32* %.omp.iv1, align 4, !tbaa !2
  br label %omp.inner.for.cond7

omp.inner.for.cond7:                              ; preds = %for.cond.cleanup, %DIR.OMP.PARALLEL.LOOP.5
  %28 = load volatile i32, i32* %.omp.iv1, align 4, !tbaa !2
  %29 = load volatile i32, i32* %.omp.ub4, align 4, !tbaa !2
  %cmp8 = icmp sle i32 %28, %29
  br i1 %cmp8, label %omp.inner.for.body9, label %omp.loop.exit

omp.inner.for.body9:                              ; preds = %omp.inner.for.cond7
  %30 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %30) #1
  %31 = load volatile i32, i32* %.omp.iv1, align 4, !tbaa !2
  store i32 %31, i32* %j, align 4, !tbaa !2
  %32 = bitcast i32* %k to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %32) #1
  store i32 0, i32* %k, align 4, !tbaa !2
  br label %for.cond

for.cond:                                         ; preds = %for.body, %omp.inner.for.body9
  %33 = load i32, i32* %k, align 4, !tbaa !2
  %cmp12 = icmp slt i32 %33, 100
  br i1 %cmp12, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %32) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %30) #1
  %34 = load volatile i32, i32* %.omp.iv1, align 4, !tbaa !2
  %add25 = add nsw i32 %34, 1
  store volatile i32 %add25, i32* %.omp.iv1, align 4, !tbaa !2
  br label %omp.inner.for.cond7

for.body:                                         ; preds = %for.cond
  %35 = load i32, i32* %i, align 4, !tbaa !2
  %idxprom = sext i32 %35 to i64
  %arrayidx = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %22, i64 0, i64 %idxprom
  %idxprom13 = sext i32 %33 to i64
  %arrayidx14 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx, i64 0, i64 %idxprom13
  %36 = load i32, i32* %arrayidx14, align 4, !tbaa !6
  %arrayidx16 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %24, i64 0, i64 %idxprom13
  %37 = load i32, i32* %j, align 4, !tbaa !2
  %idxprom17 = sext i32 %37 to i64
  %arrayidx18 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx16, i64 0, i64 %idxprom17
  %38 = load i32, i32* %arrayidx18, align 4, !tbaa !6
  %mul19 = mul nsw i32 %36, %38
  %arrayidx21 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %26, i64 0, i64 %idxprom
  %arrayidx23 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx21, i64 0, i64 %idxprom17
  %39 = load i32, i32* %arrayidx23, align 4, !tbaa !6
  %add24 = add nsw i32 %39, %mul19
  store i32 %add24, i32* %arrayidx23, align 4, !tbaa !6
  %40 = load i32, i32* %k, align 4, !tbaa !2
  %inc = add nsw i32 %40, 1
  store i32 %inc, i32* %k, align 4, !tbaa !2
  br label %for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond7
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %16) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #1
  %41 = load volatile i32, i32* %.omp.iv, align 4, !tbaa !2
  %add28 = add nsw i32 %41, 1
  store volatile i32 %add28, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.loop.exit30:                                  ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %5) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %3) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %2) #1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %1) #1
  br label %DIR.OMP.END.TARGET.9

DIR.OMP.END.TARGET.9:                             ; preds = %omp.loop.exit30
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #3 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.cond.cleanup3, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc16, %for.cond.cleanup3 ]
  %cmp = icmp slt i32 %i.0, 100
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  call void @Compute()
  %0 = load i32, i32* getelementptr inbounds ([100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 20, i64 20), align 16, !tbaa !6
  %1 = load i32, i32* getelementptr inbounds ([100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 20, i64 20), align 16, !tbaa !6
  %2 = load i32, i32* getelementptr inbounds ([100 x [100 x i32]], [100 x [100 x i32]]* @C, i64 0, i64 20, i64 20), align 16, !tbaa !6
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i32 0, i32 0), i32 %0, i32 %1, i32 %2)
  ret i32 0

for.body:                                         ; preds = %for.cond
  br label %for.cond1

for.cond1:                                        ; preds = %for.body4, %for.body
  %j.0 = phi i32 [ 0, %for.body ], [ %inc, %for.body4 ]
  %cmp2 = icmp slt i32 %j.0, 100
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond1
  %inc16 = add nsw i32 %i.0, 1
  br label %for.cond

for.body4:                                        ; preds = %for.cond1
  %mul = mul nsw i32 %i.0, 100
  %mul5 = mul nsw i32 %j.0, 2
  %add = add nsw i32 %mul, %mul5
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @A, i64 0, i64 %idxprom
  %idxprom6 = sext i32 %j.0 to i64
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx, i64 0, i64 %idxprom6
  store i32 %add, i32* %arrayidx7, align 4, !tbaa !6
  %mul9 = mul nsw i32 %j.0, 3
  %add10 = add nsw i32 %mul, %mul9
  %arrayidx12 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @B, i64 0, i64 %idxprom
  %arrayidx14 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx12, i64 0, i64 %idxprom6
  store i32 %add10, i32* %arrayidx14, align 4, !tbaa !6
  %inc = add nsw i32 %j.0, 1
  br label %for.cond1
}

declare dso_local i32 @printf(i8*, ...) #4

; Function Attrs: inaccessiblememonly nounwind speculatable
declare i8* @llvm.launder.invariant.group.p0i8(i8*) #5

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { inaccessiblememonly nounwind speculatable }

!omp_offload.info = !{!9}
!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e6de10bf60ed5be7555542cd7b35318c8f7cb851) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm bc802712af75710cebe23cf4e9278d312d72f9db)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA100_A100_i", !8, i64 0}
!8 = !{!"array@_ZTSA100_i", !3, i64 0}
!9 = !{i32 0, i32 54, i32 -698850821, !"Compute", i32 50, i32 0}
!10 = distinct !{i32 0}

; CHECK: [[LEVEL:%[0-9]+]] = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64 }* %loop.parameter.rec, i32 0, i32 0
; CHECK: store i64 2, i64* [[LEVEL]]
; CHECK: [[LB1:%[0-9]+]] = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64 }* %loop.parameter.rec, i32 0, i32 1
; CHECK:  store i64 0, i64* [[LB1]]
; CHECK: [[UB1:%[0-9]+]] = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64 }* %loop.parameter.rec, i32 0, i32 2
; CHECK: store i64 99, i64* [[UB1]]
; CHECK: [[ST1:%[0-9]+]] = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64 }* %loop.parameter.rec, i32 0, i32 3
; CHECK: store i64 1, i64* [[ST1]]
; CHECK: [[LB2:%[0-9]+]] = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64 }* %loop.parameter.rec, i32 0, i32 4
; CHECK: store i64 0, i64* [[LB2]]
; CHECK: [[UB2:%[0-9]+]] = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64 }* %loop.parameter.rec, i32 0, i32 5
; CHECK: store i64 99, i64* [[UB2]]
; CHECK: [[ST2:%[0-9]+]] = getelementptr inbounds { i64, i64, i64, i64, i64, i64, i64 }, { i64, i64, i64, i64, i64, i64, i64 }* %loop.parameter.rec, i32 0, i32 6
; CHECK: store i64 1, i64* [[ST2]]
