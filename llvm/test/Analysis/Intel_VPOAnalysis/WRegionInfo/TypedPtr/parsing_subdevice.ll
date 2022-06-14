; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s
;
; Test src: Input IR was hand modified because FE does not yet handle subdevice
; void foo() {
; double x = 0.0;
; double *out = &x;
; #pragma omp target data map(tofrom:out[0:1]) device(2)
;    out[0] = 123.0;
; }
;
; Check that subdevice was parsed
; Check for the debug string.
; CHECK: SUBDEVICE clause (size=1): SUBDEVICE({{.*}}, {{.*}}, {{.*}}, {{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %x = alloca double, align 8
  %out = alloca double*, align 8
  store double 0.000000e+00, double* %x, align 8
  store double* %x, double** %out, align 8
  %0 = load double*, double** %out, align 8
  %1 = load double*, double** %out, align 8
  %arrayidx = getelementptr inbounds double, double* %1, i64 0
  br label %DIR.OMP.TARGET.DATA.1

DIR.OMP.TARGET.DATA.1:                            ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.SUBDEVICE"(i32 1, i32 2, i32 3, i32 4), "QUAL.OMP.MAP.TOFROM"(double* %0, double* %arrayidx, i64 8, i64 35) ]
  br label %DIR.OMP.TARGET.DATA.2

DIR.OMP.TARGET.DATA.2:                            ; preds = %DIR.OMP.TARGET.DATA.1
  %3 = load double*, double** %out, align 8
  %ptridx = getelementptr inbounds double, double* %3, i64 0
  store double 1.230000e+02, double* %ptridx, align 8
  br label %DIR.OMP.END.TARGET.DATA.3

DIR.OMP.END.TARGET.DATA.3:                        ; preds = %DIR.OMP.TARGET.DATA.2
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.DATA"() ]
  br label %DIR.OMP.END.TARGET.DATA.4

DIR.OMP.END.TARGET.DATA.4:                        ; preds = %DIR.OMP.END.TARGET.DATA.3
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
