; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-target-make-distribute-fp-wilocal=true -vpo-paropt-handle-firstprivate-on-teams=false -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=CHECK_PRIVATE
; RUN: opt -vpo-paropt-target-make-distribute-fp-wilocal=true -vpo-paropt-handle-firstprivate-on-teams=false -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=CHECK_PRIVATE

; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-target-make-distribute-fp-wilocal=false -vpo-paropt-handle-firstprivate-on-teams=false -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=LOCAL
; RUN: opt -vpo-paropt-target-make-distribute-fp-wilocal=false -vpo-paropt-handle-firstprivate-on-teams=false -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=LOCAL

; Test src:

; #include <stdio.h>
;
; void foo() {
;   int x = 0;
; #pragma omp target firstprivate(x)
; #pragma omp teams distribute firstprivate(x)
;   for (int i = 0; i < 10; i++)
;     printf("%p\n", &x);
; }

; Firstprivate handling on teams is disabled by a command-line flag.

; With the make-distribute-fp-wilocal flag true, the allocation of the private
; copy of "x" happens in the private address-space

; CHECK_PRIVATE: define{{.*}} spir_kernel void @__omp_offloading{{.*}}foo{{.*}}(i64 [[X_VAL_PARM:%x.ascast[^ ,]+]], {{.*}})
; CHECK_PRIVATE:   [[X_TGTFP:%x.ascast.fpriv.*]] = alloca i32, align 4
; CHECK_PRIVATE:   [[X_DSTFP:%x.ascast.fpriv.*]] = alloca i32, align 4
; CHECK_PRIVATE:   br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru{{.*}}
; CHECK_PRIVATE: master.thread.code:
; CHECK_PRIVATE:   [[X_VAL_PARM_CAST:%x.ascast.*]] = trunc i64 [[X_VAL_PARM]] to i32
; CHECK_PRIVATE:   store i32 [[X_VAL_PARM_CAST]], ptr [[X_TGTFP]], align 4
; CHECK_PRIVATE:   [[X_TGTFP_VAL:%.+]] = load i32, ptr [[X_TGTFP]], align 4
; CHECK_PRIVATE:   [[X_TGTFP_VAL_CAST:%.+]] = zext i32 [[X_TGTFP_VAL]] to i64
; CHECK_PRIVATE:   [[X_DSTFP_CAST:%.+]] = addrspacecast ptr [[X_DSTFP]] to ptr addrspace(4)
; CHECK_PRIVATE:   [[X_TGTFP_VAL_CAST1:%.+]] = trunc i64 [[X_TGTFP_VAL_CAST]] to i32
;
; CHECK_PRIVATE:   store i32 [[X_TGTFP_VAL_CAST1]], ptr [[X_DSTFP]], align 4
;
; CHECK_PRIVATE:   call {{.*}} @_Z18__spirv_ocl_printfPU3AS2cz({{.*}}, ptr addrspace(4) [[X_DSTFP_CAST]])

; With the make-distribute-fp-wilocal flag false, the allocation of the private
; copy of "x" happens at the module level in address space local (3).

; LOCAL:  [[X_DSTFP:@x.ascast.fpriv.__local]] = internal addrspace(3) global i32 0
; LOCAL:  define{{.*}} spir_kernel void @__omp_offloading{{.*}}foo{{.*}}(i64 [[X_VAL_PARM:%x.ascast[^ ,]+]], {{.*}})
; LOCAL:    [[X_TGTFP:%x.ascast.fpriv.*]] = alloca i32, align 4
; LOCAL:    br i1 %is.master.thread, label %master.thread.code, label %master.thread.fallthru{{.*}}
; LOCAL:  master.thread.code:
; LOCAL:    [[X_VAL_PARM_CAST:%x.ascast.*]] = trunc i64 [[X_VAL_PARM]] to i32
; LOCAL:    store i32 [[X_VAL_PARM_CAST]], ptr [[X_TGTFP]], align 4
; LOCAL:    [[X_TGTFP_VAL:%.+]] = load i32, ptr [[X_TGTFP]], align 4
; LOCAL:    [[X_TGTFP_VAL_CAST:%.+]] = zext i32 [[X_TGTFP_VAL]] to i64
; LOCAL:    [[X_TGTFP_VAL_CAST1:%.+]] = trunc i64 [[X_TGTFP_VAL_CAST]] to i32
;
; LOCAL:    store i32 [[X_TGTFP_VAL_CAST1]], ptr addrspace(3) [[X_DSTFP]], align 4
;
; LOCAL:    call {{.*}} @_Z18__spirv_ocl_printfPU3AS2cz({{.*}}, ptr addrspace(4) addrspacecast (ptr addrspace(3) [[X_DSTFP]] to ptr addrspace(4)))


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%p\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @foo() #0 {
entry:
  %x = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %i = alloca i32, align 4
  %x.ascast = addrspacecast ptr %x to ptr addrspace(4)
  %.omp.lb.ascast = addrspacecast ptr %.omp.lb to ptr addrspace(4)
  %.omp.ub.ascast = addrspacecast ptr %.omp.ub to ptr addrspace(4)
  %tmp.ascast = addrspacecast ptr %tmp to ptr addrspace(4)
  %.omp.iv.ascast = addrspacecast ptr %.omp.iv to ptr addrspace(4)
  %i.ascast = addrspacecast ptr %i to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %x.ascast, align 4
  store i32 0, ptr addrspace(4) %.omp.lb.ascast, align 4
  store i32 9, ptr addrspace(4) %.omp.ub.ascast, align 4

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %x.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %x.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %tmp.ascast, i32 0, i32 1) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %x.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) %.omp.iv.ascast, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:WILOCAL.TYPED"(ptr addrspace(4) %.omp.lb.ascast, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) %.omp.ub.ascast, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %i.ascast, i32 0, i32 1) ]

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
  %call = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), ptr addrspace(4) noundef %x.ascast) #3
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr addrspace(4) %.omp.iv.ascast, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, ptr addrspace(4) %.omp.iv.ascast, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent
declare spir_func i32 @printf(ptr addrspace(4) noundef, ...) #2

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { convergent nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 66313, i32 117992200, !"_Z3foo", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
