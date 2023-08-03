; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; #include <omp.h>
; #include <stdio.h>
;
; class C {
; public:
;   int x;
;   short y;
;   C() : x(111), y(111){};
;   ~C() = default;
; };
;
; void foo() {
;   C c;
;   c.x = 222;
; #pragma omp target private(c)
;   {
; //  if (c.x != 111) printf("failed check 1. c.x = %d. Expected = %d\n", c.x, 111);
;     c.x = 333;
;
; #pragma omp parallel private(c)
;     {
; //    if (omp_get_thread_num() == 0 && c.x != 111) printf("failed check 2. c.x = %d. Expected = %d\n", c.x, 111);
;       c.x = 444;
;     }
; //  if (c.x != 333) printf("failed check 3. c.x = %d. Expected = %d\n", c.x, 333);
;   }
; //  if (c.x != 222) printf("failed check 4. c.x = %d. Expected = %d\n", c.x, 222);
; }
;
; //int main() { foo(); }

; Check for the private copy of @c for the target construct
; CHECK: [[C_TGT:@c.*__global]] = internal addrspace(1) global %class.C zeroinitializer
; CHECK-DAG: call spir_func ptr addrspace(4) @_ZTS1C.omp.def_constr(ptr addrspace(4) addrspacecast (ptr addrspace(1) [[C_TGT]] to ptr addrspace(4)))
; CHECK-DAG: store i32 333, ptr addrspace(1) [[C_TGT]], align 4
; CHECK-DAG: call spir_func void @_ZTS1C.omp.destr(ptr addrspace(4) addrspacecast (ptr addrspace(1) [[C_TGT]] to ptr addrspace(4)))

; Check for the private copy of @c for the parallel construct
; CHECK-DAG: [[C_PAR:%c.*]] = alloca %class.C, align 8
; CHECK-DAG: {{%[^ ]+}} = call spir_func ptr addrspace(4) @_ZTS1C.omp.def_constr(ptr addrspace(4) [[C_PAR_CAST:%[^, ]+]])
; CHECK-DAG: [[C_PAR_CAST]] = addrspacecast ptr [[C_PAR]] to ptr addrspace(4)
; CHECK-DAG: store i32 444, ptr [[C_PAR]], align 4
; CHECK-DAG: call spir_func void @_ZTS1C.omp.destr(ptr addrspace(4) [[C_PAR_CAST]])

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.C = type <{ i32, i16, [2 x i8] }>

define hidden spir_func void @_Z3foov() {
entry:
  %c = alloca %class.C, align 4
  %c.ascast = addrspacecast ptr %c to ptr addrspace(4)
  call spir_func void @_ZN1CC2Ev(ptr addrspace(4) %c.ascast)
  store i32 222, ptr addrspace(4) %c.ascast, align 4

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr addrspace(4) %c.ascast, %class.C zeroinitializer, i32 1, ptr addrspace(4) (ptr addrspace(4))* @_ZTS1C.omp.def_constr, ptr @_ZTS1C.omp.destr) ]

  store i32 333, ptr addrspace(4) %c.ascast, align 4

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr addrspace(4) %c.ascast, %class.C zeroinitializer, i32 1, ptr addrspace(4) (ptr addrspace(4))* @_ZTS1C.omp.def_constr, ptr @_ZTS1C.omp.destr) ]

  store i32 444, ptr addrspace(4) %c.ascast, align 4

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare hidden spir_func void @_ZN1CC2Ev(ptr addrspace(4) %this) unnamed_addr align 2
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func ptr addrspace(4) @_ZTS1C.omp.def_constr(ptr addrspace(4) %0)
declare void @_ZTS1C.omp.destr(ptr addrspace(4) %0)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2055, i32 150400248, !"_Z3foov", i32 15, i32 0, i32 0}
