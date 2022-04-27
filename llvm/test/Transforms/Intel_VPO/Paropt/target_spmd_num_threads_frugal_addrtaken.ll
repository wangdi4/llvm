; REQUIRES: asserts

; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-simulate-get-num-threads-frugally=true -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefixes=DEFAULT,ALL
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-simulate-get-num-threads-frugally=true -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefixes=DEFAULT,ALL

; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-simulate-get-num-threads-frugally=false -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefixes=NOFRUGAL,ALL
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-simulate-get-num-threads-frugally=false -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefixes=NOFRUGAL,ALL

; Test src:

; #include <omp.h>
; #include <stdio.h>
;
; #pragma omp declare target
; int val = 0;
;
; int f1() { return 111; }
; int f2() { return omp_get_num_threads(); }
; long f3() { printf("%p\n", f2); return (long) f2; }
; #pragma omp end declare target
;
; int main() {
; #pragma omp target
;   val = 222;
;
; #pragma omp target
;   printf("%d\n", val);  // 222
;
; #pragma omp target
;   printf("%d\n", f1()); // 111
;
; #pragma omp target
;   printf("%d\n", f2()); // 1
;
; #pragma omp target
;   printf("%ld\n", f3());
;
;   return 0;
; }

; Check that we skip kmpc_begin/end_spmd_target call generation for the first
; two regions. Since f2, which calls omp_get_num_threads, is address-taken, we
; assume that any other regions that call any function to be able to
; potentially call omp_get_num_threads (even f1, which doesn't call or
; reference f2/f3/omp_get_num_threads). The analysis may be expanded in
; the future if deemed beneficial.

; DEFAULT:  collectOmpNumThreadsCallerInfo: @f2 may call omp_get_num_threads.
; DEFAULT:  collectOmpNumThreadsCallerInfo: Function @f2 is address-taken by: 'i64 ptrtoint (i32 ()* @f2 to i64)'.

; DEFAULT:  mayCallOmpGetNumThreads: Didn't find any potential caller of omp_get_num_threads in the region.
; DEFAULT:  mayCallOmpGetNumThreads: Region #1 (target) may call omp_get_num_threads: No.
; NOFRUGAL: mayCallOmpGetNumThreads: Region #1 (target) may call omp_get_num_threads: Yes.

; DEFAULT:  mayCallOmpGetNumThreads: Ignoring the call to library function 'printf' from the region.
; DEFAULT:  mayCallOmpGetNumThreads: Didn't find any potential caller of omp_get_num_threads in the region.
; DEFAULT:  mayCallOmpGetNumThreads: Region #2 (target) may call omp_get_num_threads: No.
; NOFRUGAL: mayCallOmpGetNumThreads: Region #2 (target) may call omp_get_num_threads: Yes.

; DEFAULT:  mayCallOmpGetNumThreads: The region calls @f1.
; DEFAULT:  mayCallOmpGetNumThreads: omp_get_num_threads or one of its callers is address-taken. @f1 might be calling it.
; ALL:      mayCallOmpGetNumThreads: Region #3 (target) may call omp_get_num_threads: Yes.

; DEFAULT:  mayCallOmpGetNumThreads: The region calls @f2.
; DEFAULT:  mayCallOmpGetNumThreads: omp_get_num_threads or one of its callers is address-taken. @f2 might be calling it.
; ALL:      mayCallOmpGetNumThreads: Region #4 (target) may call omp_get_num_threads: Yes.

; DEFAULT:  mayCallOmpGetNumThreads: The region calls @f3.
; DEFAULT:  mayCallOmpGetNumThreads: omp_get_num_threads or one of its callers is address-taken. @f3 might be calling it.
; ALL:      mayCallOmpGetNumThreads: Region #5 (target) may call omp_get_num_threads: Yes.

; ALL: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; DEFAULT-NOT: call spir_func void @__kmpc_{{.*}}_spmd_target
; NOFRUGAL:    call spir_func void @__kmpc_begin_spmd_target()
; NOFRUGAL:    call spir_func void @__kmpc_end_spmd_target()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]

; ALL: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; DEFAULT-NOT: call spir_func void @__kmpc_{{.*}}_spmd_target
; NOFRUGAL:    call spir_func void @__kmpc_begin_spmd_target()
; NOFRUGAL:    call spir_func void @__kmpc_end_spmd_target()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]

; ALL:         call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; ALL:         call spir_func void @__kmpc_begin_spmd_target()
; ALL:         call spir_func void @__kmpc_end_spmd_target()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]

; ALL:         call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; ALL:         call spir_func void @__kmpc_begin_spmd_target()
; ALL:         call spir_func void @__kmpc_end_spmd_target()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]

; ALL:         call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; ALL:         call spir_func void @__kmpc_begin_spmd_target()
; ALL:         call spir_func void @__kmpc_end_spmd_target()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@val = hidden target_declare addrspace(1) global i32 0, align 4
@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%p\0A\00", align 1
@.str.1 = private unnamed_addr addrspace(1) constant [4 x i8] c"%d\0A\00", align 1
@.str.2 = private unnamed_addr addrspace(1) constant [5 x i8] c"%ld\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone
define hidden i32 @main() #3 {
entry:
  %retval = alloca i32, align 4
  %val.casted = alloca i64, align 8
  %val.casted1 = alloca i64, align 8
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %val.casted.ascast = addrspacecast i64* %val.casted to i64 addrspace(4)*
  %val.casted1.ascast = addrspacecast i64* %val.casted1 to i64 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  %0 = load i32, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @val to i32 addrspace(4)*), align 4
  %conv = bitcast i64 addrspace(4)* %val.casted.ascast to i32 addrspace(4)*
  store i32 %0, i32 addrspace(4)* %conv, align 4
  %1 = load i64, i64 addrspace(4)* %val.casted.ascast, align 8

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @val to i32 addrspace(4)*), i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @val to i32 addrspace(4)*), i64 4, i64 547, i8* null, i8* null) ]
  store i32 222, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @val to i32 addrspace(4)*), align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  %3 = load i32, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @val to i32 addrspace(4)*), align 4
  %conv2 = bitcast i64 addrspace(4)* %val.casted1.ascast to i32 addrspace(4)*
  store i32 %3, i32 addrspace(4)* %conv2, align 4
  %4 = load i64, i64 addrspace(4)* %val.casted1.ascast, align 8

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2), "QUAL.OMP.MAP.TOFROM"(i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @val to i32 addrspace(4)*), i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @val to i32 addrspace(4)*), i64 4, i64 547, i8* null, i8* null) ]
  %6 = load i32, i32 addrspace(4)* addrspacecast (i32 addrspace(1)* @val to i32 addrspace(4)*), align 4
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str.1 to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %6) #5
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]

  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3) ]
  %call3 = call spir_func i32 @f1() #5
  %call4 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str.1 to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %call3) #5
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TARGET"() ]

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 4) ]
  %call5 = call spir_func i32 @f2() #5
  %call6 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str.1 to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %call5) #5
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET"() ]

  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 5) ]
  %call7 = call spir_func i64 @f3() #5
  %call8 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([5 x i8], [5 x i8] addrspace(4)* addrspacecast ([5 x i8] addrspace(1)* @.str.2 to [5 x i8] addrspace(4)*), i64 0, i64 0), i64 %call7) #5
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: convergent noinline nounwind optnone
define hidden spir_func i32 @f1() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  ret i32 111
}

; Function Attrs: convergent noinline nounwind optnone
define hidden spir_func i32 @f2() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %call = call spir_func i32 @omp_get_num_threads() #5
  ret i32 %call
}

; Function Attrs: convergent nounwind
declare spir_func i32 @omp_get_num_threads() #1

; Function Attrs: convergent noinline nounwind optnone
define hidden spir_func i64 @f3() #0 {
entry:
  %retval = alloca i64, align 8
  %retval.ascast = addrspacecast i64* %retval to i64 addrspace(4)*
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 (...)* bitcast (i32 ()* @f2 to i32 (...)*)) #6
  ret i64 ptrtoint (i32 ()* @f2 to i64)
}

; Function Attrs: convergent
declare spir_func i32 @printf(i8 addrspace(4)*, ...) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { convergent nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #2 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind }
attributes #6 = { convergent }

!omp_offload.info = !{!0, !1, !2, !3, !4, !5}
!llvm.module.flags = !{!6, !7, !8, !9, !10}
!opencl.used.extensions = !{!11}
!opencl.used.optional.core.features = !{!11}
!opencl.compiler.options = !{!11}

!0 = !{i32 0, i32 66313, i32 47064951, !"_Z4main", i32 16, i32 2, i32 0}
!1 = !{i32 0, i32 66313, i32 47064951, !"_Z4main", i32 25, i32 5, i32 0}
!2 = !{i32 0, i32 66313, i32 47064951, !"_Z4main", i32 13, i32 1, i32 0}
!3 = !{i32 0, i32 66313, i32 47064951, !"_Z4main", i32 22, i32 4, i32 0}
!4 = !{i32 0, i32 66313, i32 47064951, !"_Z4main", i32 19, i32 3, i32 0}
!5 = !{i32 1, !"_Z3val", i32 0, i32 0, i32 addrspace(1)* @val}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{i32 7, !"openmp", i32 50}
!8 = !{i32 7, !"openmp-device", i32 50}
!9 = !{i32 7, !"PIC Level", i32 2}
!10 = !{i32 7, !"frame-pointer", i32 2}
!11 = !{}
