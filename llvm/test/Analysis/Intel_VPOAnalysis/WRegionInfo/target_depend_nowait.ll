; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -disable-output %s 2>&1 | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; __attribute__((noinline)) void foo() {
;   double w = 1.0;
;   long x = 2;
;   short y = 3;
;
; #pragma omp target private(w) firstprivate(x) map(tofrom:y) depend(out:y) nowait
;       {
;         y = 10;
;       }
;   printf("y = %d\n", y);
; }

; CHECK: BEGIN TASK ID=1
; CHECK: TARGET_TASK: true
; CHECK: BEGIN TARGET ID=2
; CHECK: END TARGET ID=2
; CHECK: END TASK ID=1

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.kmp_depend_info = type { i64, i64, i8 }

@.str = private unnamed_addr addrspace(1) constant [8 x i8] c"y = %d\0A\00", align 1

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @foo() #0 {
entry:
  %w = alloca double, align 8
  %x = alloca i64, align 8
  %y = alloca i16, align 2
  %.dep.arr.addr = alloca [1 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  %w.ascast = addrspacecast ptr %w to ptr addrspace(4)
  %x.ascast = addrspacecast ptr %x to ptr addrspace(4)
  %y.ascast = addrspacecast ptr %y to ptr addrspace(4)
  %.dep.arr.addr.ascast = addrspacecast ptr %.dep.arr.addr to ptr addrspace(4)
  %dep.counter.addr.ascast = addrspacecast ptr %dep.counter.addr to ptr addrspace(4)
  store double 1.000000e+00, ptr addrspace(4) %w.ascast, align 8
  store i64 2, ptr addrspace(4) %x.ascast, align 8
  store i16 3, ptr addrspace(4) %y.ascast, align 2
  %0 = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr addrspace(4) %.dep.arr.addr.ascast, i64 0, i64 0
  %1 = ptrtoint ptr addrspace(4) %y.ascast to i64
  %2 = getelementptr %struct.kmp_depend_info, ptr addrspace(4) %0, i64 0
  %3 = getelementptr inbounds %struct.kmp_depend_info, ptr addrspace(4) %2, i32 0, i32 0
  store i64 %1, ptr addrspace(4) %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, ptr addrspace(4) %2, i32 0, i32 1
  store i64 2, ptr addrspace(4) %4, align 8
  %5 = getelementptr inbounds %struct.kmp_depend_info, ptr addrspace(4) %2, i32 0, i32 2
  store i8 3, ptr addrspace(4) %5, align 8
  store i64 1, ptr addrspace(4) %dep.counter.addr.ascast, align 8
  %6 = addrspacecast ptr addrspace(4) %0 to ptr
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.TARGET.TASK"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %w.ascast, double 0.000000e+00, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %x.ascast, i64 0, i32 1),
    "QUAL.OMP.DEPARRAY"(i32 1, ptr %6),
    "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %y.ascast, i16 0, i32 1) ]
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %w.ascast, double 0.000000e+00, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %x.ascast, i64 0, i32 1),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %y.ascast, ptr addrspace(4) %y.ascast, i64 2, i64 35, ptr null, ptr null) ]
  store i16 10, ptr addrspace(4) %y.ascast, align 2
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TASK"() ]
  %9 = load i16, ptr addrspace(4) %y.ascast, align 2
  %conv = sext i16 %9 to i32
  %call = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), i32 noundef %conv) #3
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
attributes #2 = { convergent "approx-func-fp-math"="true" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { convergent }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 53, i32 -1937586093, !"_Z3foo", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 51}
!3 = !{i32 7, !"openmp-device", i32 51}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
