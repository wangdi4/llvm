; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-ctrl=3 -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-ctrl=3 -S %s | FileCheck %s

; Original code:
;
;void test4(int n, double *a) {
;  double sum[3] = {12.0, 12.0, 12.0};
;  for (int k = 0; k < 3; ++k) {
;#pragma omp target map(to: a[0:n]) map(tofrom: sum[k:1])
;#pragma omp teams distribute parallel for reduction(+: sum[k:1])
;    for (int i = 0; i < n; ++i) {
;      sum[k] += a[i];
;    }
;  }
;}

; The test IR was hand-modified to use a constant section size, and a
; simpler IR for section offset for reduction. CFE currently generates
; IR instructions to compute them.

; Check that the atomic-free reduction is not applied, because
; Paropt cannot properly hoist the array section's offset/size computation
; from the target region currently. The test used to trigger llvm_unreachable.

; CHECK-NOT: red_buf
; CHECK-NOT: teams_counter
; CHECK: declare spir_func void @__kmpc_critical

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@__const.test4.sum = private unnamed_addr addrspace(1) constant [3 x double] [double 1.200000e+01, double 1.200000e+01, double 1.200000e+01], align 8

; Function Attrs: convergent noinline nounwind optnone
define hidden spir_func void @test4(i32 %n, ptr addrspace(4) %a) #0 {
entry:
  %n.addr = alloca i32, align 4
  %a.addr = alloca ptr addrspace(4), align 8
  %sum = alloca [3 x double], align 8
  %k = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %.capture_expr.1 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %a.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %n.addr.ascast = addrspacecast ptr %n.addr to ptr addrspace(4)
  %a.addr.ascast = addrspacecast ptr %a.addr to ptr addrspace(4)
  %sum.ascast = addrspacecast ptr %sum to ptr addrspace(4)
  %k.ascast = addrspacecast ptr %k to ptr addrspace(4)
  %.capture_expr.0.ascast = addrspacecast ptr %.capture_expr.0 to ptr addrspace(4)
  %.capture_expr.1.ascast = addrspacecast ptr %.capture_expr.1 to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %a.map.ptr.tmp.ascast = addrspacecast ptr %a.map.ptr.tmp to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store i32 %n, ptr addrspace(4) %n.addr.ascast, align 4
  store ptr addrspace(4) %a, ptr addrspace(4) %a.addr.ascast, align 8
  %0 = bitcast ptr addrspace(4) %sum.ascast to ptr addrspace(4)
  call void @llvm.memcpy.p4.p1.i64(ptr addrspace(4) align 8 %0, ptr addrspace(1) align 8 @__const.test4.sum, i64 24, i1 false)
  store i32 0, ptr addrspace(4) %k.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, ptr addrspace(4) %k.ascast, align 4
  %cmp = icmp slt i32 %1, 3
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %2 = load i32, ptr addrspace(4) %n.addr.ascast, align 4
  store i32 %2, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %3 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %sub = sub nsw i32 %3, 0
  %sub1 = sub nsw i32 %sub, 1
  %add = add nsw i32 %sub1, 1
  %div = sdiv i32 %add, 1
  %sub2 = sub nsw i32 %div, 1
  store i32 %sub2, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  %4 = load i32, ptr addrspace(4) %.capture_expr.1.ascast, align 4
  store i32 %4, ptr addrspace(4) %.omp.ub.ascast, align 4
  %5 = load ptr addrspace(4), ptr addrspace(4) %a.addr.ascast, align 8
  %6 = load i32, ptr addrspace(4) %k.ascast, align 4
  %7 = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds [3 x double], ptr addrspace(4) %sum.ascast, i64 0, i64 %7
  %8 = load ptr addrspace(4), ptr addrspace(4) %a.addr.ascast, align 8
  %9 = load ptr addrspace(4), ptr addrspace(4) %a.addr.ascast, align 8
  %arrayidx3 = getelementptr inbounds double, ptr addrspace(4) %9, i64 0
  %10 = load i32, ptr addrspace(4) %n.addr.ascast, align 4
  %conv = sext i32 %10 to i64
  %11 = mul nuw i64 %conv, 8
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum.ascast, ptr addrspace(4) %arrayidx, i64 8, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %8, ptr addrspace(4) %arrayidx3, i64 %11, i64 33, ptr null, ptr null), ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  store ptr addrspace(4) %8, ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8
  %13 = load i32, ptr addrspace(4) %k.ascast, align 4
  %conv4 = sext i32 %13 to i64
  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr addrspace(4) %sum.ascast, double 0.000000e+00, i64 1, i64 %conv4),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.capture_expr.0.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %15 = load i32, ptr addrspace(4) %.capture_expr.0.ascast, align 4
  %cmp5 = icmp slt i32 0, %15
  br i1 %cmp5, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %for.body
  %16 = load i32, ptr addrspace(4) %k.ascast, align 4
  %conv7 = sext i32 %16 to i64
  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr addrspace(4) %sum.ascast, double 0.000000e+00, i64 1, i64 %conv7),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %k.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]

  %18 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %18, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %19 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %20 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp8 = icmp sle i32 %19, %20
  br i1 %cmp8, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %21 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %21, 1
  %add10 = add nsw i32 0, %mul
  store i32 %add10, ptr addrspace(4) %i.ascast, align 4
  %22 = load ptr addrspace(4), ptr addrspace(4) %a.map.ptr.tmp.ascast, align 8
  %23 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %23 to i64
  %arrayidx11 = getelementptr inbounds double, ptr addrspace(4) %22, i64 %idxprom
  %24 = load double, ptr addrspace(4) %arrayidx11, align 8
  %25 = load i32, ptr addrspace(4) %k.ascast, align 4
  %idxprom12 = sext i32 %25 to i64
  %arrayidx13 = getelementptr inbounds [3 x double], ptr addrspace(4) %sum.ascast, i64 0, i64 %idxprom12
  %26 = load double, ptr addrspace(4) %arrayidx13, align 8
  %add14 = fadd fast double %26, %24
  store double %add14, ptr addrspace(4) %arrayidx13, align 8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %27 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add15 = add nsw i32 %27, 1
  store i32 %add15, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %17) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %for.body
  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.TARGET"() ]

  br label %for.inc

for.inc:                                          ; preds = %omp.precond.end
  %28 = load i32, ptr addrspace(4) %k.ascast, align 4
  %inc = add nsw i32 %28, 1
  store i32 %inc, ptr addrspace(4) %k.ascast, align 4
  br label %for.cond, !llvm.loop !9

for.end:                                          ; preds = %for.cond
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p4.p1.i64(ptr addrspace(4) noalias nocapture writeonly, ptr addrspace(1) noalias nocapture readonly, i64, i1 immarg) #2

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { argmemonly nocallback nofree nounwind willreturn }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!7}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!8}

!0 = !{i32 0, i32 2053, i32 46675704, !"_Z5test4", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"cl_doubles"}
!8 = !{!"clang version 13.0.0"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
