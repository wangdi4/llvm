; RUN: opt -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; Test src:

; class C {
;   int x;
;
; public:
; #pragma omp declare target
;   C() : x(1) {}
;   ~C(){};
;   C(const C &c1) { x = c1.x; }
; #pragma omp end declare target
; };
;
; void foo() {
;   C c1;
; #pragma omp target firstprivate(c1)
;   ;
; }

; Check that the outlined function for the target region has a copy of %c1
; created within the function, and copy-constructor and destructor are called for it.
; CHECK-LABEL: define{{.*}}void @__omp_offloading{{[^ ]*}}foo{{[^ ]*}}(%class.C* %c1)
; CHECK: %[[C1_NEW:c1[^ ]*]] = alloca %class.C, align 4
; CHECK: call void @_ZTS1C.omp.copy_constr(%class.C* %[[C1_NEW]], %class.C* %c1)
; CHECK: call void @_ZTS1C.omp.destr(%class.C* %[[C1_NEW]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%class.C = type { i32 }

$_ZN1CC2Ev = comdat any
$_ZN1CC2ERKS_ = comdat any
$_ZN1CD2Ev = comdat any

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: noinline optnone uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %c1 = alloca %class.C, align 4
  call void @_ZN1CC2Ev(%class.C* %c1)

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%class.C* %c1, void (%class.C*, %class.C*)* @_ZTS1C.omp.copy_constr, void (%class.C*)* @_ZTS1C.omp.destr), "QUAL.OMP.MAP.TO"(%class.C* %c1, %class.C* %c1, i64 4, i64 161) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  call void @_ZN1CD2Ev(%class.C* %c1) #2
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
declare dso_local void @_ZN1CC2Ev(%class.C* %this) unnamed_addr #1 align 2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline uwtable
declare void @_ZTS1C.omp.copy_constr(%class.C* %0, %class.C* %1) #3

; Function Attrs: noinline nounwind optnone uwtable
declare dso_local void @_ZN1CC2ERKS_(%class.C* %this, %class.C* nonnull align 4 dereferenceable(4) %c1) unnamed_addr #1 align 2

; Function Attrs: noinline uwtable
declare void @_ZTS1C.omp.destr(%class.C* %0) #3 section ".text.startup"

; Function Attrs: noinline nounwind optnone uwtable
declare dso_local void @_ZN1CD2Ev(%class.C* %this) unnamed_addr #1 align 2

; Function Attrs: noinline uwtable
declare void @.omp_offloading.requires_reg() #3 section ".text.startup"

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) #2

attributes #0 = { noinline optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}

!0 = !{i32 0, i32 2055, i32 151590633, !"_Z3foov", i32 14, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
