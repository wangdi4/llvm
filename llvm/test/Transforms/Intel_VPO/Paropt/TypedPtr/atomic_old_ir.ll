; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes="vpo-cfg-restructuring,vpo-paropt-prepare" -S %s | FileCheck %s

; Deprecated the llvm.intel.directive* representation.
; TODO: Update this test to use llvm.directive.region.entry/exit instead.
; XFAIL: *

; CMPLRS-50452: Verify that with the old representation of OMP directives, the
; BEGIN/END directives are removed after VPO
;
; int aaa;
; void foo() {
;   #pragma omp atomic
;     aaa = aaa + 1;
; }

; ModuleID = 'atomic_old_ir_test.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@aaa = common dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() {
entry:
; CHECK-NOT: call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
; CHECK-NOT: call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.UPDATE")
; CHECK-NOT: call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  call void @llvm.intel.directive(metadata !"DIR.OMP.ATOMIC")
  call void @llvm.intel.directive.qual(metadata !"QUAL.OMP.UPDATE")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
; CHECK: call void @__kmpc_atomic_fixed4_add
  %0 = load i32, i32* @aaa, align 4
  %add = add nsw i32 %0, 1
  store i32 %add, i32* @aaa, align 4
; CHECK-NOT:  call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
; CHECK-NOT:  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.ATOMIC")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata)

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual(metadata)
