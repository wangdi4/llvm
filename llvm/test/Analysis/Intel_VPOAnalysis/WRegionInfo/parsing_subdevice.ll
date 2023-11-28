; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s
;
; Test src:
;
; void foo() {
;   double x = 0.0;
;   double *out = &x;
; #pragma omp target data map(tofrom                                             \
;                             : out [0:1]) device(2) subdevice(0, 2 : 8 : 3)
;   out[0] = 123.0;
; }

; Check that subdevice was parsed
; Check for the debug string.
; CHECK: SUBDEVICE clause (size=1): SUBDEVICE({{.*}}, {{.*}}, {{.*}}, {{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %x = alloca double, align 8
  %out = alloca ptr, align 8
  store double 0.000000e+00, ptr %x, align 8
  store ptr %x, ptr %out, align 8
  %0 = load ptr, ptr %out, align 8
  %1 = load ptr, ptr %out, align 8
  %arrayidx = getelementptr inbounds double, ptr %1, i64 0
  br label %DIR.OMP.TARGET.DATA.1

DIR.OMP.TARGET.DATA.1:                            ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.DEVICE"(i32 2),
    "QUAL.OMP.SUBDEVICE"(i32 0, i32 2, i32 8, i32 3),
    "QUAL.OMP.MAP.TOFROM"(ptr %0, ptr %arrayidx, i64 8, i64 3, ptr null, ptr null) ]
  br label %DIR.OMP.TARGET.DATA.2

DIR.OMP.TARGET.DATA.2:                            ; preds = %DIR.OMP.TARGET.DATA.1
  %3 = load ptr, ptr %out, align 8
  %arrayidx1 = getelementptr inbounds double, ptr %3, i64 0
  store double 1.230000e+02, ptr %arrayidx1, align 8
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

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
