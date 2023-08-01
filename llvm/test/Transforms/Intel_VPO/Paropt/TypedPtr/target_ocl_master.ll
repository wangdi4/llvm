; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s

; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; GPU-offload test for Master. The test is created by compiling
; the C test below with:  icx -O0 -fiopenmp -fopenmp-targets=spir64 
; int main() {
;   int aaa = 10;
;   #pragma omp target map(tofrom:aaa)
;   {
;     #pragma omp master
;     aaa = aaa+2;
;   }
;   printf("aaa=%d",aaa);
;   return 0;
; }

; The GPU-offloading compilation will:
;
; 1. Remove the fence acquire/release that Clang added to the Master region
; CHECK-NOT: fence acquire
; CHECK-NOT: fence release
;
; 2. Emit the begin masked with tid = 0  and filter = 0  and emit end masked with tid =0.
; CHECK: %{{[0-9]+}} = call spir_func  i32 @__kmpc_masked(%struct.ident_t addrspace(4)* addrspacecast (%struct.ident_t addrspace(1)* @{{.*}} to %struct.ident_t addrspace(4)*), i32 0, i32 0)
; CHECK: call spir_func void @__kmpc_end_masked(%struct.ident_t addrspace(4)* addrspacecast (%struct.ident_t addrspace(1)* @{{.*}} to %struct.ident_t addrspace(4)*), i32 0)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr constant [7 x i8] c"aaa=%d\00", align 1

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %aaa = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 10, i32* %aaa, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(i32* %aaa) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.MASTER"() ]
  fence acquire
  %2 = load i32, i32* %aaa, align 4
  %add = add nsw i32 %2, 2
  store i32 %add, i32* %aaa, align 4
  fence release
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.MASTER"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %3 = load i32, i32* %aaa, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.str, i32 0, i32 0), i32 %3)
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(i8*, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 59, i32 -677974782, !"main", i32 4, i32 0, i32 0}
