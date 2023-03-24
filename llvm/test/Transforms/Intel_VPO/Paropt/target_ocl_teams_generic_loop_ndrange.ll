; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-paropt-prepare)' -S <%s | FileCheck %s

; Check that VPO Paropt Prepare pass does not remove
; the "QUAL.OMP.OFFLOAD.NDRANGE" qualifier from the TARGET construct or
; the "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE" qualifier from the GENERICLOOP construct.
;
; // C++ test
; void foo(float *A) {
;   #pragma omp target teams loop is_device_ptr(A)
;     for (auto j = 0; j < 100; ++j)
;       A[j] = 1.23;
; }

source_filename = "lit.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@"@tid.addr" = external global i32

; Function Attrs: convergent mustprogress noinline nounwind optnone
define protected spir_func void @_Z3fooPf(float addrspace(4)* noundef %A) #0 {
entry:
  %A.addr = alloca float addrspace(4)*, align 8
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %j = alloca i32, align 4
  %A.addr.ascast = addrspacecast float addrspace(4)** %A.addr to float addrspace(4)* addrspace(4)*
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %j.ascast = addrspacecast i32* %j to i32 addrspace(4)*
  store float addrspace(4)* %A, float addrspace(4)* addrspace(4)* %A.addr.ascast, align 8
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 99, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %0 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %A.addr.ascast, align 8
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  br label %DIR.OMP.TARGET.12

DIR.OMP.TARGET.12:                                ; preds = %DIR.OMP.TARGET.2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(float addrspace(4)* addrspace(4)* %A.addr.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast), "QUAL.OMP.OFFLOAD.NDRANGE"(i32 addrspace(4)* %.omp.ub.ascast, i32 0) ]

; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), {{.*}}, "QUAL.OMP.OFFLOAD.NDRANGE"({{.*}})

  br label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %DIR.OMP.TARGET.12
  br label %DIR.OMP.TEAMS.4

DIR.OMP.TEAMS.4:                                  ; preds = %DIR.OMP.TARGET.3
  br label %DIR.OMP.TEAMS.2

DIR.OMP.TEAMS.2:                                  ; preds = %DIR.OMP.TEAMS.4
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.SHARED"(float addrspace(4)* addrspace(4)* %A.addr.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  br label %DIR.OMP.TEAMS.5

DIR.OMP.TEAMS.5:                                  ; preds = %DIR.OMP.TEAMS.2
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.SHARED"(float addrspace(4)* addrspace(4)* %A.addr.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %j.ascast), "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"(i1 false) ]

; Note: Prepare pass changes the GENERICLOOP to a DISTRIBUTE
; CHECK: @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(), {{.*}}, "QUAL.OMP.OFFLOAD.KNOWN.NDRANGE"({{.*}})


  br label %DIR.OMP.GENERICLOOP.6

DIR.OMP.GENERICLOOP.6:                            ; preds = %DIR.OMP.TEAMS.5
  %4 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4
  store i32 %4, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %DIR.OMP.GENERICLOOP.6
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %6 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %j.ascast, align 4
  %8 = load float addrspace(4)*, float addrspace(4)* addrspace(4)* %A.addr.ascast, align 8
  %9 = load i32, i32 addrspace(4)* %j.ascast, align 4
  %idxprom = sext i32 %9 to i64
  %arrayidx = getelementptr inbounds float, float addrspace(4)* %8, i64 %idxprom
  store float 0x3FF3AE1480000000, float addrspace(4)* %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %10, 1
  store i32 %add1, i32 addrspace(4)* %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.GENERICLOOP"() ]
  br label %DIR.OMP.END.GENERICLOOP.7

DIR.OMP.END.GENERICLOOP.7:                        ; preds = %omp.loop.exit
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TEAMS"() ]
  br label %DIR.OMP.END.TEAMS.8

DIR.OMP.END.TEAMS.8:                              ; preds = %DIR.OMP.END.GENERICLOOP.7
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  br label %DIR.OMP.END.TARGET.9

DIR.OMP.END.TARGET.9:                             ; preds = %DIR.OMP.END.TEAMS.8
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent mustprogress noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66, i32 -684787708, !"_Z3fooPf", i32 2, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
