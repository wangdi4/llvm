; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; Test src:
;
; #include <stdio.h>
;
; int main(void)
; {
;   int x = 0;
;   int y = 110;
;
; #pragma omp target map(tofrom:x) map(to: y)
;   {
; #pragma omp atomic read
;     x = y;
; #pragma omp atomic write
;     x = 100;
; #pragma omp atomic
;     x = x + 1;
;   }
;
;   printf("x = %d\n", (int) x);
;   return 0;
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

@.str = private unnamed_addr constant [8 x i8] c"x = %d\0A\00", align 1

define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %atomic-temp = alloca i32, align 4
  %atomic-temp1 = alloca i32, align 4
  %atomic-temp2 = alloca i32, align 4
  %atomic-temp3 = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 0, ptr %x, align 4
  store i32 110, ptr %y, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %x),
    "QUAL.OMP.MAP.TO"(ptr %y),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %atomic-temp, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %atomic-temp1, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %atomic-temp2, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %atomic-temp3, i32 0, i32 1) ]


  call void @__atomic_load(i64 4, ptr %y, ptr %atomic-temp, i32 0)
; CHECK: call void @__kmpc_atomic_load(i64 4, ptr addrspace(4) {{[^ ]+}}, ptr addrspace(4) {{[^ ]+}}, i32 0)

  %1 = load i32, ptr %atomic-temp, align 4
  store i32 %1, ptr %x, align 4
  store i32 100, ptr %atomic-temp1, align 4

  call void @__atomic_store(i64 4, ptr %x, ptr %atomic-temp1, i32 0)
; CHECK: call void @__kmpc_atomic_store(i64 4, ptr addrspace(4) {{[^ ]+}}, ptr addrspace(4) {{[^ ]+}}, i32 0)


  call void @__atomic_load(i64 4, ptr %x, ptr %atomic-temp2, i32 0)
; CHECK: call void @__kmpc_atomic_load(i64 4, ptr addrspace(4) {{[^ ]+}}, ptr addrspace(4) {{[^ ]+}}, i32 0)

  br label %atomic_cont

atomic_cont:                                      ; preds = %atomic_cont, %entry
  %2 = load i32, ptr %atomic-temp2, align 4
  %add = add nsw i32 %2, 1
  store i32 %add, ptr %atomic-temp3, align 4

  %call = call zeroext i1 @__atomic_compare_exchange(i64 4, ptr %x, ptr %atomic-temp2, ptr %atomic-temp3, i32 0, i32 0)
; CHECK: %call = call zeroext i1 @__kmpc_atomic_compare_exchange(i64 4, ptr addrspace(4) {{[^ ]+}}, ptr addrspace(4) {{[^ ]+}}, ptr addrspace(4) {{[^ ]+}}, i32 0, i32 0)

  br i1 %call, label %atomic_exit, label %atomic_cont

atomic_exit:                                      ; preds = %atomic_cont
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %3 = load i32, ptr %x, align 4
  %call4 = call spir_func i32 (ptr, ...) @printf(ptr @.str, i32 %3)
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local void @__atomic_load(i64, ptr, ptr, i32)
declare dso_local void @__atomic_store(i64, ptr, ptr, i32)
declare dso_local i1 @__atomic_compare_exchange(i64, ptr, ptr, ptr, i32, i32)
declare dso_local spir_func i32 @printf(ptr, ...)

; CHECK: declare dso_local void @__kmpc_atomic_load(i64, ptr addrspace(4), ptr addrspace(4), i32)
; CHECK: declare dso_local void @__kmpc_atomic_store(i64, ptr addrspace(4), ptr addrspace(4), i32)
; CHECK: declare dso_local i1 @__kmpc_atomic_compare_exchange(i64, ptr addrspace(4), ptr addrspace(4), ptr addrspace(4), i32, i32)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 2065, i32 561966718, !"main", i32 10, i32 0, i32 0}
