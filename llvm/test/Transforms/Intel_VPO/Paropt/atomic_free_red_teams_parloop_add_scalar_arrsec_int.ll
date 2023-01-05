; RUN: opt -enable-new-pm=0 -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-par-global=false -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-par-global=false -S %s | FileCheck %s

; This test checks that global update loop of atomic-free is formed correctly
; in precense of multiple items of different kind

; Test src:
;
; int main(void)
; {
;   int i;
;   int sum_arr0[2] = {0, 0}, sum_arr1[1] = {1, 1};
;   int sum0 = 0, sum1 = 0;
;
; #pragma omp target teams distribute parallel for reduction(+:sum_arr0[1], sum0, sum_arr1[1], sum1)
;   for (i=0; i<10; i++) {
;     sum_arr0[1]+=i;
;     sum0+=i;
;     sum_arr1[1]+=i;
;     sum1+=i;
;   }
;
;   return 0;
; }

; The test IR was hand-modified to use a constant section offset.
; CFE currently generates IR instructions to compute it.

; CHECK-LABEL: counter_check:
; CHECK-LABEL: atomic.free.red.global.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64 [ 0
; CHECK: %[[SCALAR_PHI1:[^,]+]] = phi i32
; CHECK: %[[SCALAR_PHI0:[^,]+]] = phi i32
; CHECK-LABEL: atomic.free.red.global.update.body:
; CHECK-LABEL: red.update.body:
; CHECK: load i32, ptr addrspace(1)
; CHECK: load i32, ptr addrspace(1)
; CHECK: store i32 %{{[0-9]+}}, ptr addrspace(1)
; CHECK-LABEL: item.exit:
; CHECK: %[[SCALAR0:[^,]+]] = add i32 %[[SCALAR_PHI0]]
; CHECK-LABEL: red.update.body{{[0-9]+}}:
; CHECK: load i32, ptr addrspace(1)
; CHECK: load i32, ptr addrspace(1)
; CHECK: store i32 %{{[0-9]+}}, ptr addrspace(1)
; CHECK-LABEL: item.exit{{[0-9]+}}:
; CHECK: %[[SCALAR1:[^,]+]] = add i32 %[[SCALAR_PHI1]]
; CHECK-LABEL: atomic.free.red.global.update.latch:
; CHECK-LABEL: atomic.free.red.global.update.store:
; CHECK-NEXT: store i32 %[[SCALAR_PHI0]], ptr addrspace(1)
; CHECK-NEXT: store i32 %[[SCALAR_PHI1]], ptr addrspace(1)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@__const.main.sum_arr1 = private unnamed_addr addrspace(1) constant [2 x i32] [i32 1, i32 1], align 4

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum_arr0 = alloca [2 x i32], align 4
  %sum_arr1 = alloca [2 x i32], align 4
  %sum0 = alloca i32, align 4
  %sum1 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %sum_arr0.ascast = addrspacecast ptr %sum_arr0 to ptr addrspace(4)
  %sum_arr1.ascast = addrspacecast ptr %sum_arr1 to ptr addrspace(4)
  %sum0.ascast = addrspacecast ptr %sum0 to ptr addrspace(4)
  %sum1.ascast = addrspacecast ptr %sum1 to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  call void @llvm.memset.p4.i64(ptr addrspace(4) align 4 %sum_arr0.ascast, i8 0, i64 8, i1 false)
  call void @llvm.memcpy.p4.p1.i64(ptr addrspace(4) align 4 %sum_arr1.ascast, ptr addrspace(1) align 4 @__const.main.sum_arr1, i64 8, i1 false)
  store i32 0, ptr addrspace(4) %sum0.ascast, align 4
  store i32 0, ptr addrspace(4) %sum1.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 9, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [1 x i32], ptr addrspace(4) %sum_arr0.ascast, i64 0, i64 1
  %arrayidx1 = getelementptr inbounds [1 x i32], ptr addrspace(4) %sum_arr1.ascast, i64 0, i64 1

  %i3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum_arr0.ascast, ptr addrspace(4) %arrayidx, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum0.ascast, ptr addrspace(4) %sum0.ascast, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum_arr1.ascast, ptr addrspace(4) %arrayidx1, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum1.ascast, ptr addrspace(4) %sum1.ascast, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %i4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr addrspace(4) %sum_arr0.ascast, i32 0, i64 1, i64 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum0.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr addrspace(4) %sum_arr1.ascast, i32 0, i64 1, i64 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %i5 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr addrspace(4) %sum_arr0.ascast, i32 0, i64 1, i64 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum0.ascast, i32 0, i32 1),
    "QUAL.OMP.REDUCTION.ADD:ARRSECT.TYPED"(ptr addrspace(4) %sum_arr1.ascast, i32 0, i64 1, i64 1),
    "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr addrspace(4) %sum1.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0) ]

  %i6 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %i6, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %i7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %i8 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %i7, %i8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %i9 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %i9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %i10 = load i32, ptr addrspace(4) %i.ascast, align 4
  %arrayidx2 = getelementptr inbounds [2 x i32], ptr addrspace(4) %sum_arr0.ascast, i64 0, i64 1
  %i11 = load i32, ptr addrspace(4) %arrayidx2, align 4
  %add3 = add nsw i32 %i11, %i10
  store i32 %add3, ptr addrspace(4) %arrayidx2, align 4
  %i12 = load i32, ptr addrspace(4) %i.ascast, align 4
  %i13 = load i32, ptr addrspace(4) %sum0.ascast, align 4
  %add4 = add nsw i32 %i13, %i12
  store i32 %add4, ptr addrspace(4) %sum0.ascast, align 4
  %i14 = load i32, ptr addrspace(4) %i.ascast, align 4
  %arrayidx5 = getelementptr inbounds [2 x i32], ptr addrspace(4) %sum_arr1.ascast, i64 0, i64 1
  %i15 = load i32, ptr addrspace(4) %arrayidx5, align 4
  %add6 = add nsw i32 %i15, %i14
  store i32 %add6, ptr addrspace(4) %arrayidx5, align 4
  %i16 = load i32, ptr addrspace(4) %i.ascast, align 4
  %i17 = load i32, ptr addrspace(4) %sum1.ascast, align 4
  %add7 = add nsw i32 %i17, %i16
  store i32 %add7, ptr addrspace(4) %sum1.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %i18 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add8 = add nsw i32 %i18, 1
  store i32 %add8, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %i5) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %i4) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %i3) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: argmemonly nocallback nofree nounwind willreturn writeonly
declare void @llvm.memset.p4.i64(ptr addrspace(4) nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: argmemonly nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p4.p1.i64(ptr addrspace(4) noalias nocapture writeonly, ptr addrspace(1) noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nocallback nofree nounwind willreturn writeonly }
attributes #2 = { argmemonly nocallback nofree nounwind willreturn }
attributes #3 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66313, i32 186593301, !"_Z4main", i32 7, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
