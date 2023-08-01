; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-target-inline -S %s | FileCheck %s
; RUN: opt -passes="vpo-paropt-target-inline" -S %s | FileCheck %s

; Check that vpo-paropt-target-inline pass adds alwaysinline attribute to all
; functions called from target region unless they have noinline attribute.

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

define hidden spir_func void @func1() {
; CHECK: define hidden spir_func void @func1() {
entry:
  ret void
}

define hidden spir_func void @func2() {
; CHECK: define hidden spir_func void @func2() #[[ALWAYSINLINE:[0-9]+]]
entry:
  ret void
}

; Function Attrs: noinline
define hidden spir_func void @func3() #0 {
; CHECK: define hidden spir_func void @func3() #[[NOINLINE:[0-9]+]]
entry:
  ret void
}

define hidden spir_func void @func4() {
; CHECK: define hidden spir_func void @func4() #[[ALWAYSINLINE]]
entry:
  call spir_func void @func3()
  ret void
}

define hidden spir_func void @func5() {
; CHECK: define hidden spir_func void @func5() #[[ALWAYSINLINE]]
entry:
  call spir_func void @func4()
  ret void
}

define hidden spir_func void @func6() {
; CHECK: define hidden spir_func void @func6() #[[ALWAYSINLINE]]
entry:
  ret void
}

define hidden spir_func void @func7() {
; CHECK: define hidden spir_func void @func7() {
entry:
  ret void
}

define hidden spir_func void @test() {
entry:
  call spir_func void @func1()
  br label %target

target:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  call spir_func void @func2()
  br label %call

call:
  call spir_func void @func5()
  br label %exit

exit:
  call spir_func void @func6()
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  call spir_func void @func7()
  ret void
}

declare token @llvm.directive.region.entry() #1

declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline }
attributes #1 = { nounwind }

; CHECK-DAG: attributes #[[NOINLINE]] = { noinline }
; CHECK-DAG: attributes #[[ALWAYSINLINE]] = { alwaysinline }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 88, i32 -671247963, !"_Z4test", i32 13, i32 0, i32 0}
