; RUN: opt -switch-to-offload -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; The test IR is a hand-modified version obtained from the following,
; to check whether a chain of addrspace casts from 1 -> 4 -> 1 can
; be handled when firstprivatizing a clause operand.
;
; The firstprivatized version of the operand would have addrspace 0,
; which could result in an illegal addrspace casting from 0 -> 4 -> 1.

; Test src:
;
; #include <stdio.h>
;
; #pragma omp begin declare target
; int x = 111;
; #pragma omp end declare target
;
; void f1() {
; #pragma omp target firstprivate(x) thread_limit(2)
; //#pragma omp parallel shared(x)
;   {
; //#pragma omp critical
;     {
; //      printf("%d %p\n", x, &x);
;       x = 222;
;     }
;   }
; }
;
; //int main() {
; //  printf("%d %p\n", x, &x);
; //  f1();
; //  printf("%d %p\n", x, &x);
; //}

; After %x1 has been firstprivatized, the addrspace of the StoreInst
; to the private copy should be 0, not 1.
; CHECK: x1.fpriv = alloca i32
; CHECK: store i32 222, ptr %x1.fpriv

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@x = protected target_declare addrspace(1) global i32 111, align 4

define protected spir_func void @f1() {
entry:
  %x4 = addrspacecast ptr addrspace(1) @x to ptr addrspace(4)
  %x1 = addrspacecast ptr addrspace(4) %x4 to ptr addrspace(1)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.FIRSTPRIVATE:WILOCAL.TYPED"(ptr addrspace(1) %x1, i32 0, i32 1),
    "QUAL.OMP.THREAD_LIMIT"(i32 2) ]

  %x14 = addrspacecast ptr addrspace(1) %x1 to ptr addrspace(4)
  %x141 = addrspacecast ptr addrspace(4) %x14 to ptr addrspace(1)
  %x1414 = addrspacecast ptr addrspace(1) %x141 to ptr addrspace(4)
  %x14141 = addrspacecast ptr addrspace(4) %x1414 to ptr addrspace(1)
  store i32 222, ptr addrspace(1) %x14141, align 4

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0, !1}
!0 = !{i32 0, i32 66313, i32 87760450, !"_Z2f1", i32 8, i32 0, i32 1, i32 0}
!1 = !{i32 1, !"_Z1x", i32 0, i32 0, ptr addrspace(1) @x}
