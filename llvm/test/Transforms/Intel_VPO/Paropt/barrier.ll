; RUN: opt -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes=vpo-paropt-prepare -S %s | FileCheck %s

; ModuleID = 'barrier_test.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  br label %DIR.OMP.BARRIER.1

; CHECK-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
; CHECK: call void @__kmpc_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; CHECK-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) [ "DIR.OMP.END.BARRIER"() ]

DIR.OMP.BARRIER.1:                                ; preds = %entry
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.BARRIER"() ]
  br label %DIR.OMP.END.BARRIER.2

DIR.OMP.END.BARRIER.2:                            ; preds = %DIR.OMP.BARRIER.1
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0"}
