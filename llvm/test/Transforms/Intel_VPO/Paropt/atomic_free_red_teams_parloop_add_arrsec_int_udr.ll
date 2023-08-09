; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-ctrl=3 -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-red-use-fp-team-counter=false  -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-ctrl=3 -vpo-paropt-atomic-free-red-local-buf-size=0 -vpo-paropt-atomic-free-red-use-fp-team-counter=false  -S %s | FileCheck %s

; Test src:
;
; int main(void)
; {
;   int i;
;   int sum[2] = {0, 0};
;
; #pragma omp declare reduction(myadd : int : omp_out += omp_in) initializer (omp_priv = 0)
; #pragma omp target teams distribute parallel for reduction(myadd:sum[1])
;   for (i=0; i<10; i++) {
;     sum[1]+=i;
;   }
;
;   return 0;
; }

; CHECK: define weak dso_local spir_kernel void @__omp_offloading{{.*}}main{{.*}}(ptr addrspace(1) noalias %[[RESULT_PTR:sum.*]], ptr addrspace(1) %[[RED_GLOBAL_BUF:red_buf.*]], ptr addrspace(1) %[[TEAMS_COUNTER_PTR:teams_counter.*]], i64 %.omp.lb

; CHECK: %[[LOCAL_BUF_BASE:team.buf.baseptr[^,]*]] = getelementptr inbounds [1 x i32], ptr addrspace(1) %[[RED_GLOBAL_BUF]]
; CHECK: %[[MINUS_OFFSET:[^,]+]] = getelementptr i32, ptr addrspace(1) %[[LOCAL_BUF_BASE]], i64 -[[CONST_OFFSET:[^,]+]]

; CHECK-LABEL: atomic.free.red.local.update.update.header:
; CHECK: %[[IDX_PHI:[^,]+]] = phi i64
; CHECK-LABEL: atomic.free.red.local.update.update.idcheck:
; CHECK: %[[DST_PTR_BASE:[^,]+]] = getelementptr i32, ptr addrspace(1) %[[MINUS_OFFSET]], i64 [[CONST_OFFSET]]
; CHECK-LABEL: atomic.free.red.local.update.update.body:
; CHECK: %[[DST_PTR:red.cpy.dest.ptr[^,]+]] = phi ptr addrspace(1) [ %[[DST_PTR_BASE]]
; CHECK: %[[SRC_PTR:red.cpy.src.ptr[^,]+]] = phi ptr
; CHECK: %[[DST_PTR_ASCAST:[^,]+]] = addrspacecast ptr addrspace(1) %[[DST_PTR]] to ptr addrspace(4)
; CHECK: %[[SRC_PTR_ASCAST:[^,]+]] = addrspacecast ptr %[[SRC_PTR]] to ptr addrspace(4)
; CHECK: call spir_func void @.omp_combiner.(ptr addrspace(4) %[[DST_PTR_ASCAST]], ptr addrspace(4) %[[SRC_PTR_ASCAST]])
; CHECK: br i1 %{{[0-9a-z.]+}}, label %atomic.free.red.local.update.update.latch, label %atomic.free.red.local.update.update.body
; CHECK-LABEL: atomic.free.red.local.update.update.latch:
; CHECK: call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK: add
; CHECK: br label %atomic.free.red.local.update.update.header

; CHECK-LABEL: counter_check:
; CHECK-LABEL: atomic.free.red.global.update.header:
; CHECK: %[[IDX_PHI_GLOBAL:[^,]+]] = phi i64
; CHECK: %[[NUM_TEAMS_0:.*]] = call spir_func i64 @_Z14get_num_groupsj(i32 0)
; CHECK: %[[GLOBAL_UPDATE_DONE:.*]] = icmp uge i64 %{{.*}}, %[[NUM_TEAMS_0]]
; CHECK: %[[GLOBAL_BUF_BASE:[^,]+]] = getelementptr [1 x i32], ptr addrspace(1) %[[RED_GLOBAL_BUF]], i64 %[[IDX_PHI_GLOBAL]]
; CHECK: br i1 %[[GLOBAL_UPDATE_DONE]], label %counter.reset, label %atomic.free.red.global.update.body
; CHECK-LABEL: counter.reset:
; CHECK: store i32 0, ptr addrspace(1) %teams_counter, align 4
; CHECK: br label %red.update.done
; CHECK-LABEL: atomic.free.red.global.update.body:
; CHECK: %[[RESULT_PTR_OFFSET:[^,]+]] = getelementptr i32, ptr addrspace(1) %[[RESULT_PTR]], i64 [[CONST_OFFSET]]
; CHECK-LABEL: red.update.body:
; CHECK: %[[DST_PTR_GLOBAL:red.cpy.dest.ptr[^,]+]] = phi ptr addrspace(1) [ %[[RESULT_PTR_OFFSET]]
; CHECK: %[[SRC_PTR_GLOBAL:red.cpy.src.ptr[^,]+]] = phi ptr addrspace(1) [ %[[GLOBAL_BUF_BASE]]
; CHECK: %[[DST_PTR_GLOBAL_ASCAST:[^,]+]] = addrspacecast ptr addrspace(1) %[[DST_PTR_GLOBAL]] to ptr addrspace(4)
; CHECK: %[[SRC_PTR_GLOBAL_ASCAST:[^,]+]] = addrspacecast ptr addrspace(1) %[[SRC_PTR_GLOBAL]] to ptr addrspace(4)
; CHECK: call spir_func void @.omp_combiner.(ptr addrspace(4) %[[DST_PTR_GLOBAL_ASCAST]], ptr addrspace(4) %[[SRC_PTR_GLOBAL_ASCAST]]), !paropt_guarded_by_thread_check
; CHECK: br i1 %red.cpy.done{{.*}}, label %item.exit, label %red.update.body
; CHECK-NOT: store

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca [2 x i32], align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  %sum.ascast = addrspacecast ptr %sum to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  call void @llvm.memset.p4.i64(ptr addrspace(4) align 4 %sum.ascast, i8 0, i64 8, i1 false)
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 9, ptr addrspace(4) %.omp.ub.ascast, align 4
  %arrayidx = getelementptr inbounds [2 x i32], ptr addrspace(4) %sum.ascast, i64 0, i64 1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sum.ascast, ptr addrspace(4) %arrayidx, i64 4, i64 547, ptr null, ptr null), ; MAP type: 547 = 0x223 = IMPLICIT (0x200) | TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.REDUCTION.UDR:ARRSECT.TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i64 1, i64 1, ptr null, ptr null, ptr @.omp_combiner., ptr @.omp_initializer.),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(),
    "QUAL.OMP.REDUCTION.UDR:ARRSECT.TYPED"(ptr addrspace(4) %sum.ascast, i32 0, i64 1, i64 1, ptr null, ptr null, ptr @.omp_combiner., ptr @.omp_initializer.),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0) ]

  %3 = load i32, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 %3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %5 = load i32, ptr addrspace(4) %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr addrspace(4) %i.ascast, align 4
  %7 = load i32, ptr addrspace(4) %i.ascast, align 4
  %arrayidx1 = getelementptr inbounds [2 x i32], ptr addrspace(4) %sum.ascast, i64 0, i64 1
  %8 = load i32, ptr addrspace(4) %arrayidx1, align 4
  %add2 = add nsw i32 %8, %7
  store i32 %add2, ptr addrspace(4) %arrayidx1, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add3 = add nsw i32 %9, 1
  store i32 %add3, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p4.i64(ptr addrspace(4) nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: convergent noinline nounwind
define internal void @.omp_combiner.(ptr addrspace(4) noalias noundef %0, ptr addrspace(4) noalias noundef %1) #3 {
entry:
  %.addr = alloca ptr addrspace(4), align 8
  %.addr1 = alloca ptr addrspace(4), align 8
  %.addr.ascast = addrspacecast ptr %.addr to ptr addrspace(4)
  %.addr1.ascast = addrspacecast ptr %.addr1 to ptr addrspace(4)
  store ptr addrspace(4) %0, ptr addrspace(4) %.addr.ascast, align 8
  store ptr addrspace(4) %1, ptr addrspace(4) %.addr1.ascast, align 8
  %2 = load ptr addrspace(4), ptr addrspace(4) %.addr1.ascast, align 8
  %3 = load ptr addrspace(4), ptr addrspace(4) %.addr.ascast, align 8
  %4 = load i32, ptr addrspace(4) %2, align 4
  %5 = load i32, ptr addrspace(4) %3, align 4
  %add = add nsw i32 %5, %4
  store i32 %add, ptr addrspace(4) %3, align 4
  ret void
}

; Function Attrs: convergent noinline nounwind
define internal void @.omp_initializer.(ptr addrspace(4) noalias noundef %0, ptr addrspace(4) noalias noundef %1) #3 {
entry:
  %.addr = alloca ptr addrspace(4), align 8
  %.addr1 = alloca ptr addrspace(4), align 8
  %.addr.ascast = addrspacecast ptr %.addr to ptr addrspace(4)
  %.addr1.ascast = addrspacecast ptr %.addr1 to ptr addrspace(4)
  store ptr addrspace(4) %0, ptr addrspace(4) %.addr.ascast, align 8
  store ptr addrspace(4) %1, ptr addrspace(4) %.addr1.ascast, align 8
  %2 = load ptr addrspace(4), ptr addrspace(4) %.addr1.ascast, align 8
  %3 = load ptr addrspace(4), ptr addrspace(4) %.addr.ascast, align 8
  store i32 0, ptr addrspace(4) %3, align 4
  ret void
}

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #2 = { nounwind }
attributes #3 = { convergent noinline nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 2050, i32 60967242, !"_Z4main", i32 7, i32 0, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 8, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}

