; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-optimize-data-sharing -S %s | FileCheck %s
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring,vpo-paropt-optimize-data-sharing)' -S %s | FileCheck %s

; Test src:

; #include <stdio.h>
; void foo() {
;   short *p1, *p2;
;   p1 = p2  = (short *)111;
; #pragma omp target private(p1) map(p2)
;   {
;     p1 = p2  = (short *)222;
;     printf("%ld %ld\n", (long)p1, (long)p2);
;   }
; }

; The IR was hand-modified to add a map(with type PRIVATE) for p1, and a
; PRIVATE clause for p2.

; We check that optimize-data-sharing pass can mark p1 as WILOCAL, but leaves
; p2 as-is, because the maptype for p1 has the PRIVATE bit, but the maptype for
; p2 does not.
; CHECK:      "QUAL.OMP.PRIVATE:TYPED.WILOCAL"(ptr addrspace(4) %p1.ascast, ptr addrspace(4) null, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %p2.ascast, ptr addrspace(4) null, i32 1)

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr addrspace(1) constant [9 x i8] c"%ld %ld\0A\00", align 1

define protected spir_func void @foo() {
entry:
  %p1 = alloca ptr addrspace(4), align 8
  %p2 = alloca ptr addrspace(4), align 8
  %p1.ascast = addrspacecast ptr %p1 to ptr addrspace(4)
  %p2.ascast = addrspacecast ptr %p2 to ptr addrspace(4)
  store ptr addrspace(4) inttoptr (i64 111 to ptr addrspace(4)), ptr addrspace(4) %p2.ascast, align 8
  store ptr addrspace(4) inttoptr (i64 111 to ptr addrspace(4)), ptr addrspace(4) %p1.ascast, align 8

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %p1.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %p2.ascast, ptr addrspace(4) null, i32 1),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %p1.ascast, ptr addrspace(4) %p1.ascast, i64 8, i64 160, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %p2.ascast, ptr addrspace(4) %p2.ascast, i64 8, i64 35, ptr null, ptr null) ]

  store ptr addrspace(4) inttoptr (i64 222 to ptr addrspace(4)), ptr addrspace(4) %p2.ascast, align 8
  store ptr addrspace(4) inttoptr (i64 222 to ptr addrspace(4)), ptr addrspace(4) %p1.ascast, align 8
  %1 = load ptr addrspace(4), ptr addrspace(4) %p1.ascast, align 8
  %2 = ptrtoint ptr addrspace(4) %1 to i64
  %3 = load ptr addrspace(4), ptr addrspace(4) %p2.ascast, align 8
  %4 = ptrtoint ptr addrspace(4) %3 to i64
  %call = call spir_func i32 (ptr addrspace(4), ...) @printf(ptr addrspace(4) noundef addrspacecast (ptr addrspace(1) @.str to ptr addrspace(4)), i64 noundef %2, i64 noundef %4)

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare spir_func i32 @printf(ptr addrspace(4) noundef, ...)
