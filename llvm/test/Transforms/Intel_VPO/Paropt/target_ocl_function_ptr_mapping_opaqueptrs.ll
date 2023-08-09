; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
; //extern void foo(void);
; void bar() {
;   void (*fptr)(void) ;// = foo;
;   short *sptr;
; #pragma omp target firstprivate(fptr, sptr)
;   fptr();
; }

; Ensure that the kernel parameter for fptr is `i64`, and that for sptr is
; `ptr addrspace(1)`.

; CHECK: define{{.*}} spir_kernel void @__omp_offloading_10309_2d8ecec__Z3bar_l5(i64 %fptr.val, ptr addrspace(1) %sptr.val)

; CHECK: [[FPTR_CAST:%.+]] = inttoptr i64 %fptr.val to ptr
; CHECK: [[SPTR_CAST:%.+]] = addrspacecast ptr addrspace(1) %sptr.val to ptr addrspace(4)

; CHECK: store ptr [[FPTR_CAST]], ptr addrspace(1) @fptr.map.ptr.tmp.ascast.priv.__global, align 8
; CHECK: [[SPTR_CAST2:%.+]] = addrspacecast ptr addrspace(4) [[SPTR_CAST]] to ptr addrspace(1)
; CHECK: store ptr addrspace(1) [[SPTR_CAST2]], ptr addrspace(1) @sptr.map.ptr.tmp.ascast.priv.__global, align 8

; CHECK: [[FPTR_VAL:%.+]] = load ptr, ptr addrspace(1) @fptr.map.ptr.tmp.ascast.priv.__global, align 8
; CHECK: call spir_func void [[FPTR_VAL]]()

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent noinline nounwind optnone
define protected spir_func void @bar() {
entry:
  %fptr = alloca ptr, align 8
  %sptr = alloca ptr addrspace(4), align 8
  %fptr.map.ptr.tmp = alloca ptr, align 8
  %sptr.map.ptr.tmp = alloca ptr addrspace(4), align 8
  %fptr.ascast = addrspacecast ptr %fptr to ptr addrspace(4)
  %sptr.ascast = addrspacecast ptr %sptr to ptr addrspace(4)
  %fptr.map.ptr.tmp.ascast = addrspacecast ptr %fptr.map.ptr.tmp to ptr addrspace(4)
  %sptr.map.ptr.tmp.ascast = addrspacecast ptr %sptr.map.ptr.tmp to ptr addrspace(4)

  %fptr.val = load ptr, ptr addrspace(4) %fptr.ascast, align 8
  %sptr.val = load ptr addrspace(4), ptr addrspace(4) %sptr.ascast, align 8

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM:FPTR"(ptr %fptr.val, ptr %fptr.val, i64 0, i64 32, ptr null, ptr null),
    "QUAL.OMP.MAP.TOFROM"(ptr addrspace(4) %sptr.val, ptr addrspace(4) %sptr.val, i64 0, i64 32, ptr null, ptr null),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %fptr.map.ptr.tmp.ascast, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %sptr.map.ptr.tmp.ascast, ptr addrspace(4) null, i32 1) ]

  store ptr %fptr.val, ptr addrspace(4) %fptr.map.ptr.tmp.ascast, align 8
  store ptr addrspace(4) %sptr.val, ptr addrspace(4) %sptr.map.ptr.tmp.ascast, align 8

  %1 = load ptr, ptr addrspace(4) %fptr.map.ptr.tmp.ascast, align 8
  call spir_func void %1()
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 47770860, !"_Z3bar", i32 5, i32 0, i32 0}
