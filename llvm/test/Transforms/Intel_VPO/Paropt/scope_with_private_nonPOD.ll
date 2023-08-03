; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src: codegen for the scope construct -- with private nonPOD

;class A
;{
;public:
;  A();
;};

;void bar();
;void foo() {
;  A aaa;
;  #pragma omp scope private(aaa)
;  {
;    bar();
;  }
;}

; Note: The test IR is obtained from Frontend.

; CHECK: %aaa.priv = alloca %class.A, align 8
; CHECK: call void @__kmpc_scope(ptr {{.*}}, i32 {{.*}}, ptr null)
; CHECK-NEXT: %1 = call ptr @_ZTS1A.omp.def_constr(ptr %aaa.priv)
; CHECK: call void @_ZTS1A.omp.destr(ptr %aaa.priv)
; CHECK: call void @__kmpc_end_scope(ptr {{.*}}, i32 {{.*}}, ptr null)
; CHECK:  call void @__kmpc_barrier(ptr {{.*}}, i32 {{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.A = type { i8 }

define dso_local void @_Z3foov() {
entry:
  %aaa = alloca %class.A, align 1
  call void @_ZN1AC1Ev(ptr noundef nonnull align 1 dereferenceable(1) %aaa)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCOPE"(),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr %aaa, %class.A zeroinitializer, i32 1, ptr @_ZTS1A.omp.def_constr, ptr @_ZTS1A.omp.destr) ]

  call void @_Z3barv() #2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SCOPE"() ]

  ret void
}

declare dso_local void @_ZN1AC1Ev(ptr noundef nonnull align 1 dereferenceable(1))
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare ptr @_ZTS1A.omp.def_constr(ptr noundef %0)
declare void @_ZTS1A.omp.destr(ptr noundef %0)
declare dso_local void @_Z3barv()
