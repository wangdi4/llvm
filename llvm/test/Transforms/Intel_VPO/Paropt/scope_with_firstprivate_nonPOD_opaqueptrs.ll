; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This test checks codegen for the 'scope' construct -- with 'firstprivate' nonPOD.
; The source IR was hand modified because front end does not yet support the 'firstprivate' clause with 'scope' construct.

; Test src:

; class A
; {
; public:
;   A();
; };
;
; void bar();
; void foo() {
;   A aaa;
;   #pragma omp scope firstprivate(aaa)
;   {
;     bar();
;   }
; }

; CHECK: %aaa.fpriv = alloca %class.A, align 8
; CHECK: call void @__kmpc_scope(
; CHECK: call void @_ZTS1A.omp.copy_constr(ptr %aaa.fpriv, ptr %aaa)
; CHECK: call void @_ZTS1A.omp.destr(ptr %aaa.fpriv)
; CHECK: call void @__kmpc_end_scope(
; CHECK: call void @__kmpc_barrier(

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.A = type { i8 }

; Function Attrs: mustprogress noinline optnone uwtable
define dso_local void @_Z3foov() {
entry:
  %aaa = alloca %class.A, align 1
  call void @_ZN1AC1Ev(ptr noundef nonnull align 1 dereferenceable(1) %aaa)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCOPE"(),
    "QUAL.OMP.FIRSTPRIVATE:NONPOD.TYPED"(ptr %aaa, %class.A zeroinitializer, i32 1, ptr @_ZTS1A.omp.copy_constr, ptr @_ZTS1A.omp.destr) ]

  call void @_Z3barv() #2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SCOPE"() ]

  ret void
}

declare dso_local void @_ZN1AC1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

; Function Attrs: noinline uwtable
define internal void @_ZTS1A.omp.copy_constr(ptr noundef %0, ptr noundef %1) {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr, align 8
  %3 = load ptr, ptr %.addr1, align 8
  ret void
}

; Function Attrs: noinline uwtable
define internal void @_ZTS1A.omp.destr(ptr noundef %0) section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  ret void
}

declare dso_local void @_Z3barv()
