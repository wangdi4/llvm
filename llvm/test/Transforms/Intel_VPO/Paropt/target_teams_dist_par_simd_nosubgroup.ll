; RUN: opt -switch-to-offload -vpo-paropt-enable-device-simd-codegen -vpo-paropt-emit-spirv-builtins -vpo-paropt-gpu-execution-scheme=0 -enable-device-simd -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -vpo-paropt-enable-device-simd-codegen -vpo-paropt-emit-spirv-builtins -vpo-paropt-gpu-execution-scheme=0 -enable-device-simd -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; #include <stdio.h>
; #include <math.h>
; #include <stdlib.h>
;
; void test_target__parallel__for() {
;   const int N0 = 262144;
;   double    expected_value = N0;
;   double    counter_N0 = 0;
;
; #pragma omp target teams distribute parallel for simd reduction(+: counter_N0)
;   for (int i0 = 0 ; i0 < N0 ; i0++ ) {
;     counter_N0 = counter_N0 +  1. ;
;   }
;
;  if( fabs(counter_N0 - N0) > 0.00001 ) {
;    printf( "error!\n" );
;    exit(1);
;  }
; }

; CHECK-NOT: @_Z22get_sub_group_local_idv
; CHECK-NOT: @_Z20sub_group_reduce_addd

; ModuleID = 'omp-par-simd-nosubgrp.cpp'
source_filename = "omp-par-simd-nosubgrp.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [8 x i8] c"error!\0A\00", align 1

; Function Attrs: convergent noinline nounwind mustprogress
define hidden spir_func void @_Z26test_target__parallel__forv() #0 {
entry:
  %N0 = alloca i32, align 4
  %N0.ascast = addrspacecast i32* %N0 to i32 addrspace(4)*
  %expected_value = alloca double, align 8
  %expected_value.ascast = addrspacecast double* %expected_value to double addrspace(4)*
  %counter_N0 = alloca double, align 8
  %counter_N0.ascast = addrspacecast double* %counter_N0 to double addrspace(4)*
  %.omp.lb = alloca i32, align 4
  %.omp.lb.ascast = addrspacecast i32* %.omp.lb to i32 addrspace(4)*
  %.omp.ub = alloca i32, align 4
  %.omp.ub.ascast = addrspacecast i32* %.omp.ub to i32 addrspace(4)*
  %tmp = alloca i32, align 4
  %tmp.ascast = addrspacecast i32* %tmp to i32 addrspace(4)*
  %.omp.iv = alloca i32, align 4
  %.omp.iv.ascast = addrspacecast i32* %.omp.iv to i32 addrspace(4)*
  %i0 = alloca i32, align 4
  %i0.ascast = addrspacecast i32* %i0 to i32 addrspace(4)*
  store i32 262144, i32 addrspace(4)* %N0.ascast, align 4, !tbaa !7
  store double 2.621440e+05, double addrspace(4)* %expected_value.ascast, align 8, !tbaa !11
  store double 0.000000e+00, double addrspace(4)* %counter_N0.ascast, align 8, !tbaa !11
  store i32 0, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !7
  store i32 262143, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !7
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(double addrspace(4)* %counter_N0.ascast, double addrspace(4)* %counter_N0.ascast, i64 8, i64 547, i8* null, i8* null), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i0.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TEAMS"(), "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %counter_N0.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.SHARED"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i0.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %tmp.ascast) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %counter_N0.ascast), "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* %.omp.iv.ascast), "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %.omp.lb.ascast), "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* %.omp.ub.ascast), "QUAL.OMP.PRIVATE"(i32 addrspace(4)* %i0.ascast) ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD"(double addrspace(4)* %counter_N0.ascast), "QUAL.OMP.LINEAR:IV"(i32 addrspace(4)* %i0.ascast, i32 1) ]
  %4 = load i32, i32 addrspace(4)* %.omp.lb.ascast, align 4, !tbaa !7
  store i32 %4, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !7
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !7
  %6 = load i32, i32 addrspace(4)* %.omp.ub.ascast, align 4, !tbaa !7
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !7
  %mul = mul nsw i32 %7, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32 addrspace(4)* %i0.ascast, align 4, !tbaa !7
  %8 = load double, double addrspace(4)* %counter_N0.ascast, align 8, !tbaa !11
  %add1 = fadd fast double %8, 1.000000e+00
  store double %add1, double addrspace(4)* %counter_N0.ascast, align 8, !tbaa !11
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !7
  %add2 = add nsw i32 %9, 1
  store i32 %add2, i32 addrspace(4)* %.omp.iv.ascast, align 4, !tbaa !7
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.SIMD"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TEAMS"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %10 = load double, double addrspace(4)* %counter_N0.ascast, align 8, !tbaa !11
  %sub = fsub fast double %10, 2.621440e+05
  %11 = call fast double @llvm.fabs.f64(double %sub)
  %cmp3 = fcmp fast ogt double %11, 1.000000e-05
  br i1 %cmp3, label %if.then, label %if.end

if.then:                                          ; preds = %omp.loop.exit
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([8 x i8], [8 x i8] addrspace(4)* addrspacecast ([8 x i8] addrspace(1)* @.str to [8 x i8] addrspace(4)*), i64 0, i64 0)) #5
  call spir_func void @exit(i32 1) #6
  unreachable

if.end:                                           ; preds = %omp.loop.exit
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.fabs.f64(double) #2

; Function Attrs: convergent
declare spir_func i32 @printf(i8 addrspace(4)*, ...) #3

; Function Attrs: convergent noreturn nounwind
declare spir_func void @exit(i32) #4

attributes #0 = { convergent noinline nounwind mustprogress "contains-openmp-target"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { convergent "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #4 = { convergent noreturn nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #5 = { convergent }
attributes #6 = { convergent noreturn nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!5}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!6}

!0 = !{i32 0, i32 64770, i32 20191465, !"_Z26test_target__parallel__forv", i32 11, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{}
!5 = !{!"cl_doubles"}
!6 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!7 = !{!8, !8, i64 0}
!8 = !{!"double", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C++ TBAA"}
!11 = !{!12, !12, i64 0}
!12 = !{!"int", !9, i64 0}
