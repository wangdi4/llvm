; RUN: opt -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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
; CHECK-LABEL: define{{.*}}void @__omp_offloading{{[^ ]*}}foo{{[^ ]*}}(ptr %c1)
; CHECK: %[[C1_NEW:c1[^ ]*]] = alloca %class.C, align 4
; CHECK: call void @_ZTS1C.omp.copy_constr(ptr %[[C1_NEW]], ptr %c1)
; CHECK: call void @_ZTS1C.omp.destr(ptr %[[C1_NEW]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%class.C = type { i32 }

; Function Attrs: mustprogress noinline uwtable
define dso_local void @_Z3foov() {
entry:
  %c1 = alloca %class.C, align 4
  call void @_ZN1CC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %c1)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr %c1, %class.C zeroinitializer, i32 1, ptr @_ZTS1C.omp.copy_constr, ptr @_ZTS1C.omp.destr) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  call void @_ZN1CD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %c1)
  ret void
}

declare void @_ZN1CC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this)
declare token @llvm.directive.region.entry() 
declare void @llvm.directive.region.exit(token)
declare void @_ZTS1C.omp.copy_constr(ptr noundef %0, ptr noundef %1)
declare void @_ZN1CC2ERKS_(ptr noundef nonnull align 4 dereferenceable(4) %this, ptr noundef nonnull align 4 dereferenceable(4) %c1)
declare void @_ZTS1C.omp.destr(ptr noundef %0)
declare void @_ZN1CD2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66312, i32 228084441, !"_Z3foov", i32 14, i32 0, i32 0, i32 0}
