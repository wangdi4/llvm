; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-device-memory-kind=1 -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -vpo-paropt-device-memory-kind=1 -S %s | FileCheck %s
;
; Test reduction local buffer uses global addrspace 
; #include <iostream>
; int main() {
;   double a[8];
;   #pragma omp target teams distribute parallel for reduction(+:a[:16]) 
;   for ( int i = 0; i < 1; ++i ) {
;
;   }
; }
;
; CHECK: %[[LOCAL_ID:[^,]+]] = call spir_func i64 @_Z12get_local_idj(i32 0)
; CHECK: %[[LOCAL_OFFSET:[^,]+]] = mul i64 %[[LOCAL_ID:[^,]+]], 16
; CHECK: %[[LOCAL_GEP:[^,]+]] = getelementptr [16384 x double], [16384 x double] addrspace(1)* @red_local_buf, i32 0, i64 %[[LOCAL_OFFSET:[^,]+]]
; CHECK: %[[DATA_GEP:[^,]+]] = getelementptr double, double addrspace(1)* %[[LOCAL_GEP:[^,]+]], i64 16
; CHECK: %red.update.isempty = icmp eq double addrspace(1)* %[[LOCAL_GEP:[^,]+]], %[[DATA_GEP:[^,]+]]
;
; ModuleID = 'J36687.cpp'
source_filename = "J36687.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline norecurse nounwind
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %a = alloca [8 x double], align 8
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %a.ascast = addrspacecast [8 x double]* %a to [8 x double] addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !8
  store i32 0, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !8
  %arrayidx = getelementptr inbounds [8 x double], [8 x double] addrspace(4)* %a.ascast, i64 0, i64 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"([8 x double] addrspace(4)* %a.ascast, double addrspace(4)* %arrayidx, i64 128, i64 547, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([8 x double] addrspace(4)* %a.ascast, i64 1, i64 0, i64 16, i64 1), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([8 x double] addrspace(4)* %a.ascast, i64 1, i64 0, i64 16, i64 1), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i.ascast) ]
  %3 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !8
  store i32 %3, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %4 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  %5 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !8
  %cmp = icmp sle i32 %4, %5
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %6 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  %mul = mul nsw i32 %6, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i.ascast, align 4, !tbaa !8
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  %add1 = add nsw i32 %7, 1
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !8
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %8 = load i32, i32 addrspace(4)* %retval.ascast, align 4
  ret i32 %8
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent mustprogress noinline norecurse nounwind "approx-func-fp-math"="true" "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}
!llvm.ident = !{!7}

!0 = !{i32 0, i32 2050, i32 49024361, !"_Z4main", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"clang version 14.0.0"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
