; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Original code:
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
; CHECK-DAG: define{{.*}}spir_kernel{{.*}}void @__omp_offloading__804_5200f06_c_l7_ctor()
; CHECK-DAG: define{{.*}}spir_kernel{{.*}}void @__omp_offloading__804_5200f06_c_l7_dtor()
; CHECK-DAG: define{{.*}}void @_ZN1CC2Ev
; CHECK-DAG: define{{.*}}void @_ZN1CD2Ev

; ModuleID = 'ocl_ctor_dtor_calling_conv.cpp'
source_filename = "ocl_ctor_dtor_calling_conv.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

%class.C = type { i8 }

$_ZN1CC2Ev = comdat any

$_ZN1CD2Ev = comdat any

@c = dso_local target_declare global %class.C zeroinitializer, align 1
@llvm.used = appending global [2 x i8*] [i8* bitcast (void ()* @__omp_offloading__804_5200f06_c_l7_ctor to i8*), i8* bitcast (void ()* @__omp_offloading__804_5200f06_c_l7_dtor to i8*)], section "llvm.metadata"

; Function Attrs: noinline nounwind uwtable
define internal void @__omp_offloading__804_5200f06_c_l7_ctor() #0 {
entry:
  call void @_ZN1CC2Ev(%class.C* @c)
  ret void
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local void @_ZN1CC2Ev(%class.C* %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca %class.C*, align 8
  store %class.C* %this, %class.C** %this.addr, align 8
  %this1 = load %class.C*, %class.C** %this.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @__omp_offloading__804_5200f06_c_l7_dtor() #0 {
entry:
  call void @_ZN1CD2Ev(%class.C* @c)
  ret void
}

; Function Attrs: noinline nounwind uwtable
define linkonce_odr dso_local void @_ZN1CD2Ev(%class.C* %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca %class.C*, align 8
  store %class.C* %this, %class.C** %this.addr, align 8
  %this1 = load %class.C*, %class.C** %this.addr, align 8
  ret void
}

attributes #0 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0, !1, !2}
!llvm.module.flags = !{!3}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!5}

!0 = !{i32 0, i32 2052, i32 85987078, !"__omp_offloading__804_5200f06_c_l7_ctor", i32 7, i32 1, i32 2}
!1 = !{i32 0, i32 2052, i32 85987078, !"__omp_offloading__804_5200f06_c_l7_dtor", i32 7, i32 2, i32 4}
!2 = !{i32 1, !"c", i32 0, i32 0, %class.C* @c}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{}
!5 = !{!"clang version 9.0.0"}
