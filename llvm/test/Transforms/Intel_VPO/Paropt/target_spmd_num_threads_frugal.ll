; REQUIRES: asserts

; RUN: opt  -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-simulate-get-num-threads-frugally=true -debug-only=vpo-paropt-target -S < %s 2>&1 | FileCheck %s -check-prefixes=DEFAULT,ALL
; RUN: opt < %s  -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-simulate-get-num-threads-frugally=true -debug-only=vpo-paropt-target -S 2>&1 | FileCheck %s -check-prefixes=DEFAULT,ALL

; RUN: opt  -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-paropt-simulate-get-num-threads-frugally=false -debug-only=vpo-paropt-target -S < %s 2>&1 | FileCheck %s -check-prefixes=NOFRUGAL,ALL
; RUN: opt < %s  -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -vpo-paropt-simulate-get-num-threads-frugally=false -debug-only=vpo-paropt-target -S 2>&1 | FileCheck %s -check-prefixes=NOFRUGAL,ALL

; Test src:

; #include <omp.h>
; #include <stdio.h>
;
; #pragma omp declare target
; int val = 0;
;
; int f1() { return 111; }
; int f2() { return omp_get_num_threads(); }
; int f3(void);
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
;   printf("%d\n", omp_get_num_threads()); // 1
;
; #pragma omp target
;   printf("%d\n", f2()); // 1
;
; #pragma omp target
;   printf("%d\n", f3());
;
;   return 0;
; }

; Check that we emit the spmd_push/pop calls for the first three regions, but
; not the last three.
; DEFAULT:  collectOmpNumThreadsCallerInfo: @main may call omp_get_num_threads.
; DEFAULT:  collectOmpNumThreadsCallerInfo: @f2 may call omp_get_num_threads.

; DEFAULT:  mayCallOmpGetNumThreads: Didn't find any potential caller of omp_get_num_threads in the region.
; DEFAULT:  mayCallOmpGetNumThreads: Region #1 (target) may call omp_get_num_threads: No.
; NOFRUGAL: mayCallOmpGetNumThreads: Region #1 (target) may call omp_get_num_threads: Yes.

; DEFAULT:  mayCallOmpGetNumThreads: Ignoring the call to library function 'printf' from the region.
; DEFAULT:  mayCallOmpGetNumThreads: Didn't find any potential caller of omp_get_num_threads in the region.
; DEFAULT:  mayCallOmpGetNumThreads: Region #2 (target) may call omp_get_num_threads: No.
; NOFRUGAL: mayCallOmpGetNumThreads: Region #2 (target) may call omp_get_num_threads: Yes.

; DEFAULT:  mayCallOmpGetNumThreads: The region calls @f1.
; DEFAULT:  mayCallOmpGetNumThreads: Ignoring the call to library function 'printf' from the region.
; DEFAULT:  mayCallOmpGetNumThreads: Didn't find any potential caller of omp_get_num_threads in the region.
; DEFAULT:  mayCallOmpGetNumThreads: Region #3 (target) may call omp_get_num_threads: No.
; NOFRUGAL: mayCallOmpGetNumThreads: Region #3 (target) may call omp_get_num_threads: Yes.

; DEFAULT:  mayCallOmpGetNumThreads: The region calls omp_get_num_threads directly.
; ALL:      mayCallOmpGetNumThreads: Region #4 (target) may call omp_get_num_threads: Yes.

; DEFAULT:  mayCallOmpGetNumThreads: The region calls @f2.
; DEFAULT:  mayCallOmpGetNumThreads: @f2 may call omp_get_num_threads.
; ALL:      mayCallOmpGetNumThreads: Region #5 (target) may call omp_get_num_threads: Yes.

; DEFAULT:  mayCallOmpGetNumThreads: The region calls @f3.
; DEFAULT:  mayCallOmpGetNumThreads: @f3 does not have an exact definition. It may call omp_get_num_threads.
; ALL:      mayCallOmpGetNumThreads: Region #6 (target) may call omp_get_num_threads: Yes.

; ALL: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; DEFAULT-NOT: call spir_func void @__kmpc_spmd_{{.*}}_num_threads
; NOFRUGAL:    call spir_func void @__kmpc_spmd_push_num_threads(i32 1)
; NOFRUGAL:    call spir_func void @__kmpc_spmd_pop_num_threads()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]

; ALL: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; DEFAULT-NOT: call spir_func void @__kmpc_spmd_{{.*}}_num_threads
; NOFRUGAL:    call spir_func void @__kmpc_spmd_push_num_threads(i32 1)
; NOFRUGAL:    call spir_func void @__kmpc_spmd_pop_num_threads()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]

; ALL: call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; DEFAULT-NOT: call spir_func void @__kmpc_spmd_{{.*}}_num_threads
; NOFRUGAL:    call spir_func void @__kmpc_spmd_push_num_threads(i32 1)
; NOFRUGAL:    call spir_func void @__kmpc_spmd_pop_num_threads()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]

; ALL:         call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; ALL:         call spir_func void @__kmpc_spmd_push_num_threads(i32 1)
; ALL:         call spir_func void @__kmpc_spmd_pop_num_threads()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]

; ALL:         call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; ALL:         call spir_func void @__kmpc_spmd_push_num_threads(i32 1)
; ALL:         call spir_func void @__kmpc_spmd_pop_num_threads()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]

; ALL:         call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(){{.*}}]
; ALL:         call spir_func void @__kmpc_spmd_push_num_threads(i32 1)
; ALL:         call spir_func void @__kmpc_spmd_pop_num_threads()
; ALL:         call void @llvm.directive.region.exit({{.*}}) [ "DIR.OMP.END.TARGET"() ]

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@val = hidden target_declare addrspace(1) global i32 0, align 4
@.str = private unnamed_addr addrspace(1) constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone
define hidden i32 @main() #2 {
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
  %call = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %6) #5
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]

  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 3) ]
  %call3 = call spir_func i32 @f1() #5
  %call4 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %call3) #5
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TARGET"() ]

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 4) ]
  %call5 = call spir_func i32 @omp_get_num_threads() #5
  %call6 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %call5) #5
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET"() ]

  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 5) ]
  %call7 = call spir_func i32 @f2() #5
  %call8 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %call7) #5
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.TARGET"() ]

  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 6) ]
  %call9 = call spir_func i32 @f3() #5
  %call10 = call spir_func i32 (i8 addrspace(4)*, ...) @printf(i8 addrspace(4)* getelementptr inbounds ([4 x i8], [4 x i8] addrspace(4)* addrspacecast ([4 x i8] addrspace(1)* @.str to [4 x i8] addrspace(4)*), i64 0, i64 0), i32 %call9) #5
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.TARGET"() ]

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

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: convergent
declare spir_func i32 @printf(i8 addrspace(4)*, ...) #4

; Function Attrs: convergent
declare spir_func i32 @f3() #4

attributes #0 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { convergent nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #2 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }
attributes #4 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #5 = { convergent nounwind }

!omp_offload.info = !{!0, !1, !2, !3, !4, !5, !6}
!llvm.module.flags = !{!7, !8, !9, !10, !11}
!opencl.used.extensions = !{!12}
!opencl.used.optional.core.features = !{!12}
!opencl.compiler.options = !{!12}

!0 = !{i32 0, i32 66313, i32 47064949, !"_Z4main", i32 28, i32 6, i32 0}
!1 = !{i32 0, i32 66313, i32 47064949, !"_Z4main", i32 16, i32 2, i32 0}
!2 = !{i32 0, i32 66313, i32 47064949, !"_Z4main", i32 25, i32 5, i32 0}
!3 = !{i32 0, i32 66313, i32 47064949, !"_Z4main", i32 13, i32 1, i32 0}
!4 = !{i32 0, i32 66313, i32 47064949, !"_Z4main", i32 22, i32 4, i32 0}
!5 = !{i32 0, i32 66313, i32 47064949, !"_Z4main", i32 19, i32 3, i32 0}
!6 = !{i32 1, !"_Z3val", i32 0, i32 0, i32 addrspace(1)* @val}
!7 = !{i32 1, !"wchar_size", i32 4}
!8 = !{i32 7, !"openmp", i32 50}
!9 = !{i32 7, !"openmp-device", i32 50}
!10 = !{i32 7, !"PIC Level", i32 2}
!11 = !{i32 7, !"frame-pointer", i32 2}
!12 = !{}
