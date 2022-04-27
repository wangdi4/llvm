; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes="vpo-cfg-restructuring,vpo-paropt-prepare" -S %s | FileCheck %s
;
; Test src:
; short x = 10;
; long v = 2;
; double e = 1.0;
;
; void foo (void)
; {
; #pragma omp atomic capture
;    { v = x; x = e; }
; }
;
; ModuleID = 'atomic_swap.c'
source_filename = "atomic_swap.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = dso_local global i16 10, align 2
@v = dso_local global i64 2, align 8
@e = dso_local global double 1.000000e+00, align 8

; Function Attrs: nounwind uwtable
define dso_local void @foo() #0 {
entry:
; CHECK-NOT: %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.ATOMIC"(), "QUAL.OMP.CAPTURE"() ]

  fence acquire
  %1 = load i16, i16* @x, align 2
  %conv = sext i16 %1 to i64
  store i64 %conv, i64* @v, align 8
  %2 = load double, double* @e, align 8
  %conv1 = fptosi double %2 to i16
  store i16 %conv1, i16* @x, align 2
  fence release

; CHECK: %[[EXPR:[0-9]+]] = load double, double* @e, align 8
; CHECK-NEXT: %[[EXPRCAST:conv[0-9]*]] = fptosi double %[[EXPR]] to i16
; CHECK:  %[[RET:[0-9]+]] = call i16 @__kmpc_atomic_fixed2_swp({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i16* @x, i16 %[[EXPRCAST]])
; CHECK-NEXT: %[[RETCAST:[a-zA-Z._0-9]+]] = sext i16 %[[RET]] to i64
; CHECK-NEXT: store i64 %[[RETCAST]], i64* @v

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
; CHECK-NOT: call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.ATOMIC"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
