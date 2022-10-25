; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-par-global=false -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-par-global=false -S %s | FileCheck %s

; This test checks that global update loop of atomic-free is formed correctly
; in precense of multiple items of different kind
;
; int main(void)
; {
;   int i;
;   int sum_arr0[1] = {0}, sum_arr1[1] = {1};
;   int sum0 = 0, sum1 = 0;
;
; #pragma omp target teams distribute parallel for reduction(+:sum_arr0[0], sum0, sum_arr1[0], sum1)
;   for (i=0; i<10; i++) {
;     sum_arr0[0]+=i;
;     sum0+=i;
;     sum_arr1[0]+=i;
;     sum1+=i;
;   }
;
;   return 0;
; }

; CHECK-LABEL: counter_check:
; CHECK-LABEL: atomic.free.red.global.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64 [ 0
; CHECK: %[[SCALAR_PHI1:[^,]+]] = phi i32
; CHECK: %[[SCALAR_PHI0:[^,]+]] = phi i32
; CHECK-LABEL: atomic.free.red.global.update.body:
; CHECK-LABEL: red.update.body:
; CHECK: load i32, i32 addrspace(1)*
; CHECK: load i32, i32 addrspace(1)*
; CHECK: store i32 %{{[0-9]+}}, i32 addrspace(1)*
; CHECK-LABEL: item.exit:
; CHECK: %[[SCALAR0:[^,]+]] = add i32 %[[SCALAR_PHI0]]
; CHECK-LABEL: red.update.body{{[0-9]+}}:
; CHECK: load i32, i32 addrspace(1)*
; CHECK: load i32, i32 addrspace(1)*
; CHECK: store i32 %{{[0-9]+}}, i32 addrspace(1)*
; CHECK-LABEL: item.exit{{[0-9]+}}:
; CHECK: %[[SCALAR1:[^,]+]] = add i32 %[[SCALAR_PHI1]]
; CHECK-LABEL: atomic.free.red.global.update.latch:
; CHECK-LABEL: atomic.free.red.global.update.store:
; CHECK-NEXT: store i32 %[[SCALAR_PHI0]], i32 addrspace(1)* 
; CHECK-NEXT: store i32 %[[SCALAR_PHI1]], i32 addrspace(1)* 


; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"


; Function Attrs: convergent noinline nounwind optnone
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum_arr0 = alloca [1 x i32], align 4
  %sum_arr1 = alloca [1 x i32], align 4
  %sum0 = alloca i32, align 4
  %sum1 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %sum_arr0.ascast = addrspacecast [1 x i32]* %sum_arr0 to [1 x i32] addrspace(4)*
  %sum_arr1.ascast = addrspacecast [1 x i32]* %sum_arr1 to [1 x i32] addrspace(4)*
  %sum0.ascast = addrspacecast i32* %sum0 to i32 addrspace(4)*
  %sum1.ascast = addrspacecast i32* %sum1 to i32 addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  %0 = bitcast [1 x i32] addrspace(4)* %sum_arr0.ascast to i8 addrspace(4)*
  call void @llvm.memset.p4i8.i64(i8 addrspace(4)* align 4 %0, i8 0, i64 4, i1 false)
  %1 = bitcast [1 x i32] addrspace(4)* %sum_arr1.ascast to i8 addrspace(4)*
  call void @llvm.memset.p4i8.i64(i8 addrspace(4)* align 4 %1, i8 0, i64 4, i1 false)
  store i32 0, i32 addrspace(4)* %sum0.ascast, align 4
  store i32 1, i32 addrspace(4)* %sum1.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 9, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [1 x i32], [1 x i32] addrspace(4)* %sum_arr0.ascast, i64 0, i64 0
  %arrayidx1 = getelementptr inbounds [1 x i32], [1 x i32] addrspace(4)* %sum_arr1.ascast, i64 0, i64 0
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([1 x i32] addrspace(4)* %sum_arr0.ascast, i32 addrspace(4)* %arrayidx, i64 4, i64 547, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %sum0.ascast, i32 addrspace(4)* %sum0.ascast, i64 4, i64 547, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"([1 x i32] addrspace(4)* %sum_arr1.ascast, i32 addrspace(4)* %arrayidx1, i64 4, i64 547, i8* null, i8* null), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* %sum1.ascast, i32 addrspace(4)* %sum1.ascast, i64 4, i64 547, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([1 x i32] addrspace(4)* %sum_arr0.ascast, i64 1, i64 0, i64 1, i64 1), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %sum0.ascast), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([1 x i32] addrspace(4)* %sum_arr1.ascast, i64 1, i64 0, i64 1, i64 1), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %sum1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([1 x i32] addrspace(4)* %sum_arr0.ascast, i64 1, i64 0, i64 1, i64 1), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %sum0.ascast), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([1 x i32] addrspace(4)* %sum_arr1.ascast, i64 1, i64 0, i64 1, i64 1), "QUAL.OMP.REDUCTION.ADD"(i32 addrspace(4)* %sum1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast) ]
  %5 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %5, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %7 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %6, %7
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  %9 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %arrayidx2 = getelementptr inbounds [1 x i32], [1 x i32] addrspace(4)* %sum_arr0.ascast, i64 0, i64 0
  %10 = load i32, i32 addrspace(4)* %arrayidx2, align 4
  %add3 = add nsw i32 %10, %9
  store i32 %add3, i32 addrspace(4)* %arrayidx2, align 4
  %11 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %12 = load i32, i32 addrspace(4)* %sum0.ascast, align 4
  %add4 = add nsw i32 %12, %11
  store i32 %add4, i32 addrspace(4)* %sum0.ascast, align 4
  %13 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %arrayidx5 = getelementptr inbounds [1 x i32], [1 x i32] addrspace(4)* %sum_arr1.ascast, i64 0, i64 0
  %14 = load i32, i32 addrspace(4)* %arrayidx5, align 4
  %add6 = add nsw i32 %14, %13
  store i32 %add6, i32 addrspace(4)* %arrayidx5, align 4
  %15 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %16 = load i32, i32 addrspace(4)* %sum1.ascast, align 4
  %add7 = add nsw i32 %16, %15
  store i32 %add7, i32 addrspace(4)* %sum1.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %17 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add8 = add nsw i32 %17, 1
  store i32 %add8, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p4i8.i64(i8 addrspace(4)* nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #1

attributes #0 = { convergent noinline nounwind "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66311, i32 42218157, !"_Z4main", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
