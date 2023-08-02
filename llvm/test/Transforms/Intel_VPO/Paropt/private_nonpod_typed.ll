; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s 2>&1 | FileCheck %s

; Test src:
;
; struct NP {
;   NP(); ~NP();
; };
; void foo() {
;   NP A;
; #pragma omp parallel private(A)
;   (void)A;
; }

; Check that constructor/destructor are not invoked in a loop, since
; only one element of type %struct.NP is privatized:
; CHECK: define internal void @_Z3foov.DIR.OMP.PARALLEL
; CHECK: [[A_PRIV:%[A-Za-z0-9._]+]] = alloca %struct.NP
; CHECK: call ptr @_ZTS2NP.omp.def_constr(ptr [[A_PRIV]])
; CHECK: call void @_ZTS2NP.omp.destr(ptr [[A_PRIV]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.NP = type { i8 }

define dso_local void @_Z3foov() {
entry:
  %A = alloca %struct.NP, align 1
  call void @_ZN2NPC1Ev(ptr noundef nonnull align 1 dereferenceable(1) %A)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr %A, %struct.NP zeroinitializer, i32 1, ptr @_ZTS2NP.omp.def_constr, ptr @_ZTS2NP.omp.destr) ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  call void @_ZN2NPD1Ev(ptr noundef nonnull align 1 dereferenceable(1) %A)
  ret void
}

declare dso_local void @_ZN2NPC1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare ptr @_ZTS2NP.omp.def_constr(ptr noundef %0)

declare void @_ZTS2NP.omp.destr(ptr noundef %0)

declare dso_local void @_ZN2NPD1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr
