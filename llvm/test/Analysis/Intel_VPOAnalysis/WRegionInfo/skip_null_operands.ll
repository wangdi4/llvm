; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s
;
; int x = 1;
; void foo() {
;   #pragma omp parallel private (x)
;   x = 10;
; }
; The IR presented below is hand modified by adding a "QUAL.OMP.PRIVATE"(i32* null)
;
; check that the debug message is printed when we try to skip the privatization of a constantPointerNull
; CHECK: {{.*}} Ignoring null clause operand.
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@x = dso_local global i32 1, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  br label %DIR.OMP.PARALLEL.1

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i32* @x), "QUAL.OMP.PRIVATE"(i32* null) ]
  br label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  store i32 10, i32* @x, align 4
  br label %DIR.OMP.END.PARALLEL.4

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.PARALLEL.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.5

DIR.OMP.END.PARALLEL.5:                           ; preds = %DIR.OMP.END.PARALLEL.4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
