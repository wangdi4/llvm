; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S | FileCheck %s
;
; This tests checks that paropt creates offload entries for declare target
; variable with destructor in target compilation. It is based on IR from the
; following sample
;
; #pragma omp declare target
; struct S {
;   ~S() {}
; };
; S Var;
; #pragma omp end declare target

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

%struct.S = type { i8 }

$_ZN1SD2Ev = comdat any

@Var = target_declare global %struct.S zeroinitializer, align 1
@llvm.used = appending global [1 x i8*] [i8* bitcast (void ()* @__omp_offloading__32_d79701d4_Var_l5_dtor to i8*)], section "llvm.metadata"

; Check that paropt creates offload entry for the Var and its destructor.
; CHECK-DAG: @.omp_offloading.entry.{{.*}} = weak target_declare constant { i8*, i8*, i64, i32, i32 } { i8* getelementptr inbounds (%struct.S, %struct.S* @Var, i32 0, i32 0), i8* {{.*}}, i32 0, i32 0), i64 1, i32 0, i32 0 }, section "omp_offloading_entries"
; CHECK-DAG: @.omp_offloading.entry.{{.*}} = weak target_declare constant { i8*, i8*, i64, i32, i32 } { i8* bitcast (void ()* @__omp_offloading__32_d79701d4_Var_l5_dtor to i8*), i8* {{.*}}, i32 0, i32 0), i64 0, i32 4, i32 0 }, section "omp_offloading_entries"

define internal void @__omp_offloading__32_d79701d4_Var_l5_dtor() section ".text.startup" {
entry:
  call void @_ZN1SD2Ev(%struct.S* @Var)
  ret void
}

define linkonce_odr void @_ZN1SD2Ev(%struct.S* %this) unnamed_addr #0 comdat align 2 {
entry:
  ret void
}

attributes #0 = { "openmp-target-declare"="true" }

; Targets compilation should not have offload registration code.
; CHECK-NOT: call i32 @__tgt_register_lib
; CHECK-NOT: call i32 @__tgt_unregister_lib

; Check that offload metadata is removed after outlining.
; CHECK-NOT: !omp_offload.info
!omp_offload.info = !{!0, !1}
!0 = !{i32 1, !"Var", i32 0, i32 0}
!1 = !{i32 0, i32 50, i32 -677969452, !"__omp_offloading__32_d79701d4_Var_l5_dtor", i32 5, i32 1, i32 4}
