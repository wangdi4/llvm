; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-paropt-enable-device-simd-codegen -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -switch-to-offload -vpo-paropt-enable-device-simd-codegen -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; #pragma omp declare target
; int x = 1;
; int f2() {
;   return 2;
; }
; void f1() {
;   #pragma omp target
;   x = f2();
; }
; #pragma omp end declare target
;
; int main()
; {
;   f1();
;   #pragma omp target
;   {
;     f1();
;   }
;   return 0;
; }

; This test checks that functions marked with 'omp-target-declare' attributes
; have 'omp_declare_target_simd_function' MD to enable target SIMD lowering
; for them.

; CHECK: define {{.*}} spir_func {{.*}} i32 @_Z2f2v() {{.*}} !omp_declare_target_simd_function
; CHECK: define {{.*}} spir_func void @_Z2f1v() {{.*}} !omp_declare_target_simd_function
; CHECK: define {{.*}} spir_kernel void @__omp_offloading{{.*}}main{{.*}} !omp_simd_kernel
; CHECK: call spir_func void @_Z2f1v()
; CHECK: ret
; CHECK: define {{.*}} spir_kernel void @__omp_offloading{{.*}}f1{{.*}} !omp_simd_kernel
; CHECK: ret

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

@x = protected target_declare addrspace(1) global i32 1, align 4

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define protected noundef i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  store i32 0, ptr addrspace(4) %retval.ascast, align 4
  call spir_func void @_Z2f1v() #4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 2) ]

  call spir_func void @_Z2f1v() #5
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret i32 0
}

; Function Attrs: convergent mustprogress noinline nounwind optnone
define protected spir_func noundef i32 @_Z2f2v() #1 {
entry:
  %retval = alloca i32, align 4
  %retval.ascast = addrspacecast ptr %retval to ptr addrspace(4)
  ret i32 2
}

; Function Attrs: convergent mustprogress noinline nounwind optnone
define protected spir_func void @_Z2f1v() #2 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.LIVEIN:TYPED"(ptr addrspace(4) addrspacecast (ptr addrspace(1) @x to ptr addrspace(4)), i32 0, i32 1) ]

  %call = call spir_func noundef i32 @_Z2f2v() #5
  store i32 %call, ptr addrspace(4) addrspacecast (ptr addrspace(1) @x to ptr addrspace(4)), align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { convergent mustprogress noinline nounwind optnone "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #2 = { convergent mustprogress noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }
attributes #4 = { convergent }
attributes #5 = { convergent nounwind }

!omp_offload.info = !{!0, !1, !2}
!llvm.module.flags = !{!3, !4, !5, !6, !7}
!opencl.compiler.options = !{!8}

!0 = !{i32 0, i32 64773, i32 8041939, !"_Z4main", i32 18, i32 2, i32 0}
!1 = !{i32 0, i32 64773, i32 8041939, !"_Z2f1v", i32 10, i32 1, i32 0}
!2 = !{i32 1, !"_Z1x", i32 0, i32 0, ptr addrspace(1) @x}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 7, !"openmp", i32 50}
!5 = !{i32 7, !"openmp-device", i32 50}
!6 = !{i32 8, !"PIC Level", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{}

