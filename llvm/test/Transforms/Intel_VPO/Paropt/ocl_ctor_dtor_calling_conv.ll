; RUN: opt -bugpoint-enable-legacy-pm -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Test src:
;
; #pragma omp declare target
; class C {
; public:
;   C() {};
;   ~C() {};
; };
; C c;
; #pragma omp end declare target

; Check that FE generated offload entries for calling constructors/destructors
; are marked as spir_kernel:
; CHECK-DAG: define{{.*}}spir_kernel{{.*}}void @__omp_offloading__35_8c81a1e1_c_l7_ctor()
; CHECK-DAG: define{{.*}}spir_kernel{{.*}}void @__omp_offloading__35_8c81a1e1_c_l7_dtor()
; CHECK-DAG: define{{.*}}void @_ZN1CC2Ev
; CHECK-DAG: define{{.*}}void @_ZN1CD2Ev

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%class.C = type { i8 }

$_ZN1CC2Ev = comdat any

$_ZN1CD2Ev = comdat any

@c = protected target_declare addrspace(1) global %class.C zeroinitializer, align 1

; Function Attrs: nounwind
define spir_func void @__omp_offloading__35_8c81a1e1_c_l7_ctor() #0 section ".text.startup" {
entry:
  call spir_func void @_ZN1CC2Ev(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1) addrspacecast (ptr addrspace(1) @c to ptr addrspace(4))) #2
  ret void
}

; Function Attrs: convergent noinline nounwind optnone
define linkonce_odr protected spir_func void @_ZN1CC2Ev(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1) %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca ptr addrspace(4), align 8
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  ret void
}

; Function Attrs: nounwind
define spir_func void @__omp_offloading__35_8c81a1e1_c_l7_dtor() #0 section ".text.startup" {
entry:
  call spir_func void @_ZN1CD2Ev(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1) addrspacecast (ptr addrspace(1) @c to ptr addrspace(4))) #3
  ret void
}

; Function Attrs: convergent noinline nounwind optnone
define linkonce_odr protected spir_func void @_ZN1CD2Ev(ptr addrspace(4) noundef align 1 dereferenceable_or_null(1) %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca ptr addrspace(4), align 8
  %this.addr.ascast = addrspacecast ptr %this.addr to ptr addrspace(4)
  store ptr addrspace(4) %this, ptr addrspace(4) %this.addr.ascast, align 8
  %this1 = load ptr addrspace(4), ptr addrspace(4) %this.addr.ascast, align 8
  ret void
}

attributes #0 = { nounwind "min-legal-vector-width"="0" "openmp-target-declare"="true" }
attributes #1 = { convergent noinline nounwind optnone "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #2 = { convergent }
attributes #3 = { convergent nounwind }

!omp_offload.info = !{!0, !1, !2}
!llvm.module.flags = !{!3, !4, !5, !6, !7}
!opencl.compiler.options = !{!8}

!0 = !{i32 0, i32 53, i32 -1937661471, !"__omp_offloading__35_8c81a1e1_c_l7_ctor", i32 7, i32 1, i32 2}
!1 = !{i32 0, i32 53, i32 -1937661471, !"__omp_offloading__35_8c81a1e1_c_l7_dtor", i32 7, i32 2, i32 4}
!2 = !{i32 1, !"_Z1c", i32 0, i32 0, ptr addrspace(1) @c}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 7, !"openmp", i32 51}
!5 = !{i32 7, !"openmp-device", i32 51}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
!8 = !{}
