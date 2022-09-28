; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-atomic-free-reduction-ctrl=3 -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-atomic-free-reduction-ctrl=3 -S %s | FileCheck %s

; Original code:
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


; Check that the atomic-free reduction is applied even though
; no ARRSECT modifier provided for a fullsized array reditem

; CHECK-LABEL: red.update.done.to.tree:
; CHECK-COUNT-7: lshr
; CHECK: add
; CHECK-LABEL: atomic.free.red.local.update.update.latch:
; CHECK-NEXT: call spir_func void @_Z22__spirv_ControlBarrieriii
; CHECK-NEXT: lshr

; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%d \00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [2 x i8] c"\0A\00", align 1

; Function Attrs: convergent mustprogress noinline norecurse nounwind
define hidden i32 @main() #0 {
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
  %i28 = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %n.ascast = addrspacecast i32* %n to i32 addrspace(4)*
  %a.ascast = addrspacecast [5 x [5 x i32]]* %a to [5 x [5 x i32]] addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  %res.ascast = addrspacecast [5 x i32]* %res to [5 x i32] addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i10.ascast = addrspacecast i32* %i10 to i32 addrspace(4)*
  %j13.ascast = addrspacecast i32* %j13 to i32 addrspace(4)*
  %i28.ascast = addrspacecast i32* %i28 to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  store i32 5, i32 addrspace(4)* %n.ascast, align 4
  %0 = bitcast [5 x [5 x i32]] addrspace(4)* %a.ascast to i8 addrspace(4)*
  call void @llvm.memset.p4i8.i64(i8 addrspace(4)* align 4 %0, i8 0, i64 100, i1 false)
  store i32 0, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc6, %entry
  %1 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %cmp = icmp slt i32 %1, 5
  br i1 %cmp, label %for.body, label %for.end8

for.body:                                         ; preds = %for.cond
  store i32 0, i32 addrspace(4)* %j.ascast, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %2 = load i32, i32 addrspace(4)* %j.ascast, align 4
  %cmp2 = icmp slt i32 %2, 5
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %3 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %mul = mul nsw i32 %3, 10
  %4 = load i32, i32 addrspace(4)* %j.ascast, align 4
  %add = add nsw i32 %mul, %4
  %5 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %idxprom = sext i32 %5 to i64
  %arrayidx = getelementptr inbounds [5 x [5 x i32]], [5 x [5 x i32]] addrspace(4)* %a.ascast, i64 0, i64 %idxprom
  %6 = load i32, i32 addrspace(4)* %j.ascast, align 4
  %idxprom4 = sext i32 %6 to i64
  %arrayidx5 = getelementptr inbounds [5 x i32], [5 x i32] addrspace(4)* %arrayidx, i64 0, i64 %idxprom4
  store i32 %add, i32 addrspace(4)* %arrayidx5, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %7 = load i32, i32 addrspace(4)* %j.ascast, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, i32 addrspace(4)* %j.ascast, align 4
  br label %for.cond1

for.end:                                          ; preds = %for.cond1
  br label %for.inc6

for.inc6:                                         ; preds = %for.end
  %8 = load i32, i32 addrspace(4)* %i.ascast, align 4
  %inc7 = add nsw i32 %8, 1
  store i32 %inc7, i32 addrspace(4)* %i.ascast, align 4
  br label %for.cond

for.end8:                                         ; preds = %for.cond
  %9 = bitcast [5 x i32] addrspace(4)* %res.ascast to i8 addrspace(4)*
  call void @llvm.memset.p4i8.i64(i8 addrspace(4)* align 4 %9, i8 0, i64 20, i1 false)
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 4, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([5 x i32] addrspace(4)* %res.ascast, [5 x i32] addrspace(4)* %res.ascast, i64 20, i64 547, i8* null, i8* null), "QUAL.OMP.MAP.TO"([5 x [5 x i32]] addrspace(4)* %a.ascast, [5 x [5 x i32]] addrspace(4)* %a.ascast, i64 100, i64 33, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i10.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j13.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"([5 x i32] addrspace(4)* %res.ascast), "QUAL.OMP.SHARED"([5 x [5 x i32]] addrspace(4)* %a.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i10.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j13.ascast) ]
  %12 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %12, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.end8
  %13 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %14 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp9 = icmp sle i32 %13, %14
  br i1 %cmp9, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %15 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul11 = mul nsw i32 %15, 1
  %add12 = add nsw i32 0, %mul11
  store i32 %add12, i32 addrspace(4)* %i10.ascast, align 4
  store i32 0, i32 addrspace(4)* %j13.ascast, align 4
  br label %for.cond14

for.cond14:                                       ; preds = %for.inc24, %omp.inner.for.body
  %16 = load i32, i32 addrspace(4)* %j13.ascast, align 4
  %cmp15 = icmp slt i32 %16, 5
  br i1 %cmp15, label %for.body16, label %for.end26

for.body16:                                       ; preds = %for.cond14
  %17 = load i32, i32 addrspace(4)* %i10.ascast, align 4
  %idxprom17 = sext i32 %17 to i64
  %arrayidx18 = getelementptr inbounds [5 x [5 x i32]], [5 x [5 x i32]] addrspace(4)* %a.ascast, i64 0, i64 %idxprom17
  %18 = load i32, i32 addrspace(4)* %j13.ascast, align 4
  %idxprom19 = sext i32 %18 to i64
  %arrayidx20 = getelementptr inbounds [5 x i32], [5 x i32] addrspace(4)* %arrayidx18, i64 0, i64 %idxprom19
  %19 = load i32, i32 addrspace(4)* %arrayidx20, align 4
  %20 = load i32, i32 addrspace(4)* %i10.ascast, align 4
  %idxprom21 = sext i32 %20 to i64
  %arrayidx22 = getelementptr inbounds [5 x i32], [5 x i32] addrspace(4)* %res.ascast, i64 0, i64 %idxprom21
  %21 = load i32, i32 addrspace(4)* %arrayidx22, align 4
  %add23 = add nsw i32 %21, %19
  store i32 %add23, i32 addrspace(4)* %arrayidx22, align 4
  br label %for.inc24

for.inc24:                                        ; preds = %for.body16
  %22 = load i32, i32 addrspace(4)* %j13.ascast, align 4
  %inc25 = add nsw i32 %22, 1
  store i32 %inc25, i32 addrspace(4)* %j13.ascast, align 4
  br label %for.cond14

for.end26:                                        ; preds = %for.cond14
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end26
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %23 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add27 = add nsw i32 %23, 1
  store i32 %add27, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TARGET"() ]
  store i32 0, i32 addrspace(4)* %i28.ascast, align 4
  br label %for.cond29

for.cond29:                                       ; preds = %for.inc34, %omp.loop.exit
  %24 = load i32, i32 addrspace(4)* %i28.ascast, align 4
  %cmp30 = icmp slt i32 %24, 5
  br i1 %cmp30, label %for.body31, label %for.end36

for.body31:                                       ; preds = %for.cond29
  %25 = load i32, i32 addrspace(4)* %i28.ascast, align 4
  %idxprom32 = sext i32 %25 to i64
  %arrayidx33 = getelementptr inbounds [5 x i32], [5 x i32] addrspace(4)* %res.ascast, i64 0, i64 %idxprom32
  %26 = load i32, i32 addrspace(4)* %arrayidx33, align 4
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %26) #4
  br label %for.inc34

for.inc34:                                        ; preds = %for.body31
  %27 = load i32, i32 addrspace(4)* %i28.ascast, align 4
  %inc35 = add nsw i32 %27, 1
  store i32 %inc35, i32 addrspace(4)* %i28.ascast, align 4
  br label %for.cond29

for.end36:                                        ; preds = %for.cond29
  %call37 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([2 x i8], [2 x i8] addrspace(4)* addrspacecast ([2 x i8] addrspace(1)* @.str.1 to [2 x i8] addrspace(4)*), i64 0, i64 0)) #4
  ret i32 0
}

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p4i8.i64(i8 addrspace(4)* nocapture writeonly, i8, i64, i1 immarg) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: convergent
declare spir_func i32 @printf(i8 addrspace(4)*, ...) #3

attributes #0 = { convergent mustprogress noinline norecurse nounwind "approx-func-fp-math"="true" "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nounwind willreturn writeonly }
attributes #2 = { nounwind }
attributes #3 = { convergent "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #4 = { convergent }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!6}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66311, i32 42220824, !"_Z4main", i32 11, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
