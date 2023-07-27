; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; #pragma omp declare target
; int my_glob[100];
;
; void foo(char *);
; #pragma omp end declare target
;
; void bar() {
;   int i;
; #pragma omp target map(my_glob[:10])
;   foo((char *)my_glob);
; }

; Check that @my_glob reference inside the region is correctly replaced
; with the argument of the outline function:
; CHECK: @my_glob = common dso_local
; CHECK-NOT: call{{.*}}@my_glob
; CHECK-NOT: %{{.*}} = {{.*}}@my_glob

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@my_glob = common dso_local target_declare addrspace(1) global [100 x i32] zeroinitializer, align 4

define dso_local spir_func void @bar() {
entry:
  %i = alloca i32, align 4
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.MAP.TOFROM:AGGRHEAD"([100 x i32] addrspace(4)* addrspacecast ([100 x i32] addrspace(1)* @my_glob to [100 x i32] addrspace(4)*), i32 addrspace(4)* getelementptr inbounds ([100 x i32], [100 x i32] addrspace(4)* addrspacecast ([100 x i32] addrspace(1)* @my_glob to [100 x i32] addrspace(4)*), i64 0, i64 0), i64 40) ]

  call spir_func void @foo(i8 addrspace(4)* addrspacecast (i8 addrspace(1)* bitcast ([100 x i32] addrspace(1)* @my_glob to i8 addrspace(1)*) to i8 addrspace(4)*))
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local spir_func void @foo(i8 addrspace(4)*)

!omp_offload.info = !{!0, !1}
!0 = !{i32 0, i32 2052, i32 85998847, !"bar", i32 9, i32 1, i32 0}
!1 = !{i32 1, !"my_glob", i32 0, i32 0, [100 x i32] addrspace(1)* @my_glob}
