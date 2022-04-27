; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; #if 0
; #include <stdio.h>
; #include <stdlib.h>
; #endif
;
; int main() {
;   int a[100] = {0};
; #pragma omp target parallel for map(from: a)
;   for (int i = 0; i < 100; ++i) {
;     int lp;
; #pragma omp parallel for collapse(2) lastprivate(lp)
;     for (int j = 0; j < 100; ++j)
;       for (int k = 0; k < 100; ++k)
;         lp = j + k;
;
;     a[i] = lp;
;   }
; #if 0
;   for (int i = 0; i < 100; ++i)
;     if (a[i] != 198) {
;       printf("a[%d] == %d != 198\n", i, a[i]);
;       exit(1);
;     }
; #endif
;   return 0;
; }

; Check that the inner parallel-for is not partitioned across
; WGs/WIs:
; CHECK: call{{.*}}get_global_id
; CHECK-NOT: get_local_id
; CHECK-NOT: get_local_size

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone
define hidden i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %a = alloca [100 x i32], align 4
  %a.ascast = addrspacecast [100 x i32]* %a to [100 x i32] addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %lp = alloca i32, align 4
  %lp.ascast = addrspacecast i32* %lp to i32 addrspace(4)*
  %tmp1 = alloca i32, align 4
  %tmp1.ascast = addrspacecast i32* %tmp1 to i32 addrspace(4)*
  %tmp2 = alloca i32, align 4
  %tmp2.ascast = addrspacecast i32* %tmp2 to i32 addrspace(4)*
  %.omp.uncollapsed.iv = alloca i32, align 4
  %.omp.uncollapsed.iv.ascast = addrspacecast i32* %.omp.uncollapsed.iv to i32 addrspace(4)*
  %.omp.uncollapsed.iv3 = alloca i32, align 4
  %.omp.uncollapsed.iv3.ascast = addrspacecast i32* %.omp.uncollapsed.iv3 to i32 addrspace(4)*
  %.omp.uncollapsed.lb = alloca i32, align 4
  %.omp.uncollapsed.lb.ascast = addrspacecast i32* %.omp.uncollapsed.lb to i32 addrspace(4)*
  %.omp.uncollapsed.ub = alloca i32, align 4
  %.omp.uncollapsed.ub.ascast = addrspacecast i32* %.omp.uncollapsed.ub to i32 addrspace(4)*
  %.omp.uncollapsed.lb4 = alloca i32, align 4
  %.omp.uncollapsed.lb4.ascast = addrspacecast i32* %.omp.uncollapsed.lb4 to i32 addrspace(4)*
  %.omp.uncollapsed.ub5 = alloca i32, align 4
  %.omp.uncollapsed.ub5.ascast = addrspacecast i32* %.omp.uncollapsed.ub5 to i32 addrspace(4)*
  %j = alloca i32, align 4
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  %k = alloca i32, align 4
  %k.ascast = addrspacecast i32* %k to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  %0 = bitcast [100 x i32] addrspace(4)* %a.ascast to i8 addrspace(4)*
  call void @llvm.memset.p4i8.i64(i8 addrspace(4)* align 4 %0, i8 0, i64 400, i1 false)
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.FROM"([100 x i32] addrspace(4)* %a.ascast, [100 x i32] addrspace(4)* %a.ascast, i64 400, i64 34), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %lp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp2.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"([100 x i32] addrspace(4)* %a.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %lp.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.iv3.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp1.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp2.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.uncollapsed.lb4.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.uncollapsed.ub5.ascast, align 4
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.COLLAPSE"(i32 2), "QUAL.OMP.LASTPRIVATE"(i32 addrspace(4)* %lp.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, i32 addrspace(4)* %.omp.uncollapsed.iv3.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, i32 addrspace(4)* %.omp.uncollapsed.ub5.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.uncollapsed.lb4.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %k.ascast) ]
  %8 = load i32, i32 addrspace(4)* %.omp.uncollapsed.lb.ascast, align 4
  store i32 %8, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.cond:                        ; preds = %omp.uncollapsed.loop.inc16, %omp.inner.for.body
  %9 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %10 = load i32, i32 addrspace(4)* %.omp.uncollapsed.ub.ascast, align 4
  %cmp6 = icmp sle i32 %9, %10
  br i1 %cmp6, label %omp.uncollapsed.loop.body, label %omp.uncollapsed.loop.end18

omp.uncollapsed.loop.body:                        ; preds = %omp.uncollapsed.loop.cond
  %11 = load i32, i32 addrspace(4)* %.omp.uncollapsed.lb4.ascast, align 4
  store i32 %11, i32 addrspace(4)* %.omp.uncollapsed.iv3.ascast, align 4
  br label %omp.uncollapsed.loop.cond7

omp.uncollapsed.loop.cond7:                       ; preds = %omp.uncollapsed.loop.inc, %omp.uncollapsed.loop.body
  %12 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv3.ascast, align 4
  %13 = load i32, i32 addrspace(4)* %.omp.uncollapsed.ub5.ascast, align 4
  %cmp8 = icmp sle i32 %12, %13
  br i1 %cmp8, label %omp.uncollapsed.loop.body9, label %omp.uncollapsed.loop.end

omp.uncollapsed.loop.body9:                       ; preds = %omp.uncollapsed.loop.cond7
  %14 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %mul10 = mul nsw i32 %14, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, i32 addrspace(4)* %j.ascast, align 4
  %15 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv3.ascast, align 4
  %mul12 = mul nsw i32 %15, 1
  %add13 = add nsw i32 0, %mul12
  store i32 %add13, i32 addrspace(4)* %k.ascast, align 4
  %16 = load i32, i32 addrspace(4)* %j.ascast, align 4
  %17 = load i32, i32 addrspace(4)* %k.ascast, align 4
  %add14 = add nsw i32 %16, %17
  store i32 %add14, i32 addrspace(4)* %lp.ascast, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.uncollapsed.loop.body9
  br label %omp.uncollapsed.loop.inc

omp.uncollapsed.loop.inc:                         ; preds = %omp.body.continue
  %18 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv3.ascast, align 4
  %add15 = add nsw i32 %18, 1
  store i32 %add15, i32 addrspace(4)* %.omp.uncollapsed.iv3.ascast, align 4
  br label %omp.uncollapsed.loop.cond7

omp.uncollapsed.loop.end:                         ; preds = %omp.uncollapsed.loop.cond7
  br label %omp.uncollapsed.loop.inc16

omp.uncollapsed.loop.inc16:                       ; preds = %omp.uncollapsed.loop.end
  %19 = load i32, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  %add17 = add nsw i32 %19, 1
  store i32 %add17, i32 addrspace(4)* %.omp.uncollapsed.iv.ascast, align 4
  br label %omp.uncollapsed.loop.cond

omp.uncollapsed.loop.end18:                       ; preds = %omp.uncollapsed.loop.cond
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %20 = load i32, i32 addrspace(4)* %lp.ascast, align 4
  %21 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %21 to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32] addrspace(4)* %a.ascast, i64 0, i64 %idxprom
  store i32 %20, i32 addrspace(4)* %arrayidx, align 4
  br label %omp.body.continue19

omp.body.continue19:                              ; preds = %omp.uncollapsed.loop.end18
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue19
  %22 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add20 = add nsw i32 %22, 1
  store i32 %add20, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.memset.p4i8.i64(i8 addrspace(4)* nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { noinline nounwind optnone "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn writeonly }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 0, i32 2054, i32 1856647, !"_Z4main", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{}
!4 = !{!"clang version 10.0.0"}
