; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s

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
; CHECK: define internal void @__omp_offloading_804_52009e2_foo_l10(double*{{.*}})
; ModuleID = 'fp.c'
source_filename = "fp.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %x = alloca double, align 8
  %out = alloca double*, align 8
  store double 0.000000e+00, double* %x, align 8
  store double* %x, double** %out, align 8
  %0 = load double*, double** %out, align 8
  %arrayidx = getelementptr inbounds double, double* %0, i64 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.MAP.TOFROM:AGGRHEAD"(double** %out, double** %out, i64 8), "QUAL.OMP.MAP.TOFROM:AGGR"(double** %out, double* %arrayidx, i64 8) ]
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(double** %out) ]
  %3 = load double*, double** %out, align 8
  %arrayidx1 = getelementptr inbounds double, double* %3, i64 0
  store double 1.230000e+02, double* %arrayidx1, align 8
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2052, i32 85985762, !"foo", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
