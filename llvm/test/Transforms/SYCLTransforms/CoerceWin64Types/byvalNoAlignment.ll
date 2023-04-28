; This test checks alignment information is generated for byval argument without align info
; RUN: opt -passes='debugify,sycl-kernel-coerce-win64-types,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-coerce-win64-types' -S %s | FileCheck %s

%struct.D = type { i64 }
%struct.E = type { i32, i64 }

; CHECK: define dso_local spir_func void @foo(i64 %0, ptr %1) #0 {
; CHECK-NEXT:  %{{[0-9]+}} = alloca %struct.E, align 8
; CHECK-NEXT:  %{{[0-9]+}} = alloca %struct.D, align 8
; CHECK-NEXT:  store i64 %0, ptr %{{[0-9]+}}, align 8
; CHECK-NEXT:  %{{[0-9]+}} = load i64, ptr %{{[0-9]+}}, align 8
define dso_local spir_func void @foo(ptr byval(%struct.D) %0, ptr byval(%struct.E) %1) #0 {
  call spir_func void @foo(ptr byval(%struct.D) %0, ptr byval(%struct.E) %1)
  ret void
}

attributes #0 = { convergent noinline norecurse nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

; DEBUGIFY-NOT: WARNING
