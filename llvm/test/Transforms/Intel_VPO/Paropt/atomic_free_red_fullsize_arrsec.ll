; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-ctrl=3 -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-ctrl=3 -S %s | FileCheck %s

; Original code:
; #include <stdio.h>
; int main() {
;     constexpr int n = 5;
;     int a[n][n] = {0};
;     for (int i = 0; i < n; i++)
;         for (int j = 0; j < n; j++)
;             a[i][j]=i*10+j;
;     int res[n] = {0};
;     #pragma omp target map(to:a)
;     #pragma omp parallel for reduction(+:res)
;     for (int i = 0; i < n; i++)
;         for (int j = 0; j < n; j++)
;             res[i] += a[i][j];
;     for (int i = 0; i < n; i++)
;       printf("%d ", res[i]);
;     printf("\n");
;     return 0;
; }

; Check that the atomic-free reduction is applied even though
; no ARRSECT modifier provided for a fullsized array reditem

; CHECK-LABEL: red.update.done.to.tree:
; CHECK-COUNT-7: lshr
; CHECK: add
; CHECK-LABEL: atomic.free.red.local.update.update.latch:
; CHECK-NEXT: call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT: lshr

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%d \00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [2 x i8] c"\0A\00", align 1

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %n = alloca i32, align 4
  %a = alloca [5 x [5 x i32]], align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %res = alloca [5 x i32], align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i10 = alloca i32, align 4
  %j13 = alloca i32, align 4
  %i29 = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %n.ascast = addrspacecast ptr %n to ptr addrspace(4)
  %a.ascast = addrspacecast ptr %a to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %j.ascast = addrspacecast ptr %j to ptr addrspace(4)
  %res.ascast = addrspacecast ptr %res to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i10.ascast = addrspacecast ptr %i10 to ptr addrspace(4)
  %j13.ascast = addrspacecast ptr %j13 to ptr addrspace(4)
  %i29.ascast = addrspacecast ptr %i29 to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  store i32 5, ptr addrspace(4) %n.ascast, align 4
  call void @llvm.memset.p4.i64(ptr addrspace(4) align 4 %a.ascast, i8 0, i64 100, i1 false)
  store i32 0, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc6, %entry
  %0 = load i32, ptr addrspace(4) %i.ascast, align 4
  %cmp = icmp slt i32 %0, 5
  br i1 %cmp, label %for.body, label %for.end8

for.body:                                         ; preds = %for.cond
  store i32 0, ptr addrspace(4) %j.ascast, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %1 = load i32, ptr addrspace(4) %j.ascast, align 4
  %cmp2 = icmp slt i32 %1, 5
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %2 = load i32, ptr addrspace(4) %i.ascast, align 4
  %mul = mul nsw i32 %2, 10
  %3 = load i32, ptr addrspace(4) %j.ascast, align 4
  %add = add nsw i32 %mul, %3
  %4 = load i32, ptr addrspace(4) %i.ascast, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds [5 x [5 x i32]], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom
  %5 = load i32, ptr addrspace(4) %j.ascast, align 4
  %idxprom4 = sext i32 %5 to i64
  %arrayidx5 = getelementptr inbounds [5 x i32], ptr addrspace(4) %arrayidx, i64 0, i64 %idxprom4
  store i32 %add, ptr addrspace(4) %arrayidx5, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %6 = load i32, ptr addrspace(4) %j.ascast, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, ptr addrspace(4) %j.ascast, align 4
  br label %for.cond1, !llvm.loop !8

for.end:                                          ; preds = %for.cond1
  br label %for.inc6

for.inc6:                                         ; preds = %for.end
  %7 = load i32, ptr addrspace(4) %i.ascast, align 4
  %inc7 = add nsw i32 %7, 1
  store i32 %inc7, ptr addrspace(4) %i.ascast, align 4
  br label %for.cond, !llvm.loop !10

for.end8:                                         ; preds = %for.cond
  call void @llvm.memset.p4.i64(ptr addrspace(4) align 4 %res.ascast, i8 0, i64 20, i1 false)
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 4, ptr addrspace(4) %.omp.ub.ascast, align 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %res.ascast, ptr addrspace(4) %res.ascast, i64 20, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TO"(ptr addrspace(4) %a.ascast, ptr addrspace(4) %a.ascast, i64 100, i64 33, ptr null, ptr null), ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j13.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %array.begin = getelementptr inbounds [5 x i32], ptr addrspace(4) %res.ascast, i32 0, i32 0
  %array.begin28 = getelementptr inbounds [5 x [5 x i32]], ptr addrspace(4) %a.ascast, i32 0, i32 0, i32 0
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %res.ascast, i32 0, i64 5),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast, i32 0, i64 25),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i10.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %j13.ascast, i32 0, i32 1) ]

  %10 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %10, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.end8
  %11 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %12 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp9 = icmp sle i32 %11, %12
  br i1 %cmp9, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %13 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul11 = mul nsw i32 %13, 1
  %add12 = add nsw i32 0, %mul11
  store i32 %add12, ptr addrspace(4) %i10.ascast, align 4
  store i32 0, ptr addrspace(4) %j13.ascast, align 4
  br label %for.cond14

for.cond14:                                       ; preds = %for.inc24, %omp.inner.for.body
  %14 = load i32, ptr addrspace(4) %j13.ascast, align 4
  %cmp15 = icmp slt i32 %14, 5
  br i1 %cmp15, label %for.body16, label %for.end26

for.body16:                                       ; preds = %for.cond14
  %15 = load i32, ptr addrspace(4) %i10.ascast, align 4
  %idxprom17 = sext i32 %15 to i64
  %arrayidx18 = getelementptr inbounds [5 x [5 x i32]], ptr addrspace(4) %a.ascast, i64 0, i64 %idxprom17
  %16 = load i32, ptr addrspace(4) %j13.ascast, align 4
  %idxprom19 = sext i32 %16 to i64
  %arrayidx20 = getelementptr inbounds [5 x i32], ptr addrspace(4) %arrayidx18, i64 0, i64 %idxprom19
  %17 = load i32, ptr addrspace(4) %arrayidx20, align 4
  %18 = load i32, ptr addrspace(4) %i10.ascast, align 4
  %idxprom21 = sext i32 %18 to i64
  %arrayidx22 = getelementptr inbounds [5 x i32], ptr addrspace(4) %res.ascast, i64 0, i64 %idxprom21
  %19 = load i32, ptr addrspace(4) %arrayidx22, align 4
  %add23 = add nsw i32 %19, %17
  store i32 %add23, ptr addrspace(4) %arrayidx22, align 4
  br label %for.inc24

for.inc24:                                        ; preds = %for.body16
  %20 = load i32, ptr addrspace(4) %j13.ascast, align 4
  %inc25 = add nsw i32 %20, 1
  store i32 %inc25, ptr addrspace(4) %j13.ascast, align 4
  br label %for.cond14, !llvm.loop !11

for.end26:                                        ; preds = %for.cond14
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end26
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %21 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add27 = add nsw i32 %21, 1
  store i32 %add27, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET"() ]

  store i32 0, ptr addrspace(4) %i29.ascast, align 4
  br label %for.cond30

for.cond30:                                       ; preds = %for.inc35, %omp.loop.exit
  %22 = load i32, ptr addrspace(4) %i29.ascast, align 4
  %cmp31 = icmp slt i32 %22, 5
  br i1 %cmp31, label %for.body32, label %for.end37

for.body32:                                       ; preds = %for.cond30
  %23 = load i32, ptr addrspace(4) %i29.ascast, align 4
  %idxprom33 = sext i32 %23 to i64
  %arrayidx34 = getelementptr inbounds [5 x i32], ptr addrspace(4) %res.ascast, i64 0, i64 %idxprom33
  %24 = load i32, ptr addrspace(4) %arrayidx34, align 4
  %call = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), i32 noundef %24) #4
  br label %for.inc35

for.inc35:                                        ; preds = %for.body32
  %25 = load i32, ptr addrspace(4) %i29.ascast, align 4
  %inc36 = add nsw i32 %25, 1
  store i32 %inc36, ptr addrspace(4) %i29.ascast, align 4
  br label %for.cond30, !llvm.loop !12

for.end37:                                        ; preds = %for.cond30
  %call38 = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str.1 to ptr addrspace(4))) #4
  ret i32 0
}

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p4.i64(ptr addrspace(4) nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: convergent
declare spir_func i32 @printf(ptr addrspace(4) noundef, ...) #3

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nounwind willreturn writeonly }
attributes #2 = { nounwind }
attributes #3 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #4 = { convergent }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 66313, i32 186593496, !"_Z4main", i32 9, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"clang version 14.0.0"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
!11 = distinct !{!11, !9}
!12 = distinct !{!12, !9}
