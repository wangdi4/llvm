; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; void foo() {
;   double x = 0.0;
;   double *out = &x;
; #pragma omp target data map(tofrom:out[0:1])
; #pragma omp target firstprivate(out)
;   out[0] = 123.0;
; }

; Check that 'out' is passed as a pointer vs pointer-to-pointer
; to the target region.

; CHECK: @.offload_maptypes ={{.*}}[1 x i64] [i64 32]
; CHECK: define internal void @__omp_offloading_804_52009e2_foo_l10(ptr{{.*}})

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

; Function Attrs: noinline nounwind uwtable
define dso_local void @foo() {
entry:
  %x = alloca double, align 8
  %out = alloca ptr, align 8
  store double 0.000000e+00, ptr %x, align 8
  store ptr %x, ptr %out, align 8
  %0 = load ptr, ptr %out, align 8
  %arrayidx = getelementptr inbounds double, ptr %0, i64 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(ptr %out, ptr %out, i64 8),
    "QUAL.OMP.MAP.TOFROM:AGGR"(ptr %out, ptr %arrayidx, i64 8) ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %out, ptr null, i32 1) ]

  %3 = load ptr, ptr %out, align 8
  %arrayidx1 = getelementptr inbounds double, ptr %3, i64 0
  store double 1.230000e+02, ptr %arrayidx1, align 8
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.DATA"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 2052, i32 85985762, !"foo", i32 10, i32 0, i32 0}
