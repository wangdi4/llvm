; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S %s | FileCheck %s

; This tests checks that paropt creates offload entries for declare target
; variable with destructor in target compilation. It is based on IR from the
; following sample

; Test src:
;
; #pragma omp declare target
; struct S {
;   ~S() {}
; };
; S Var;
; #pragma omp end declare target

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

%struct.S = type { i8 }

$_ZN1SD2Ev = comdat any

@Var = protected target_declare global %struct.S zeroinitializer, align 1

; Check that paropt creates offload entry for the Var and its destructor.
; CHECK-DAG: %struct.__tgt_offload_entry = type { ptr, ptr, i64, i32, i32 }
; CHECK-DAG: @.omp_offloading.entry.{{.*}} = weak target_declare constant %struct.__tgt_offload_entry { ptr @Var, ptr {{.*}}, i64 1, i32 0, i32 0 }, section "omp_offloading_entries"
; CHECK-DAG: @.omp_offloading.entry.{{.*}} = weak target_declare constant %struct.__tgt_offload_entry { ptr @__omp_offloading__35_8d6d72f9_Var_l5_dtor, ptr {{.*}}, i64 0, i32 4, i32 0 }, section "omp_offloading_entries"

define void @__omp_offloading__35_8d6d72f9_Var_l5_dtor() #0 section ".text.startup" {
entry:
  call void @_ZN1SD2Ev(ptr noundef nonnull align 1 dereferenceable(1) @Var) #2
  ret void
}

; Function Attrs: convergent noinline nounwind optnone uwtable
define linkonce_odr protected void @_ZN1SD2Ev(ptr noundef nonnull align 1 dereferenceable(1) %this) unnamed_addr #1 comdat align 2 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  ret void
}

attributes #0 = { "min-legal-vector-width"="0" "openmp-target-declare"="true" }
attributes #1 = { convergent noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { convergent nounwind }

; Targets compilation should not have offload registration code.
; CHECK-NOT: call i32 @__tgt_register_lib
; CHECK-NOT: call i32 @__tgt_unregister_lib

; Check that offload metadata is removed after outlining.
; CHECK-NOT: !omp_offload.info
!omp_offload.info = !{!0, !1}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}

!0 = !{i32 0, i32 53, i32 -1922206983, !"__omp_offloading__35_8d6d72f9_Var_l5_dtor", i32 5, i32 1, i32 4}
!1 = !{i32 1, !"_Z3Var", i32 0, i32 0, ptr @Var}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 7, !"openmp", i32 51}
!4 = !{i32 7, !"openmp-device", i32 51}
!5 = !{i32 7, !"PIC Level", i32 2}
!6 = !{i32 7, !"uwtable", i32 2}
!7 = !{i32 7, !"frame-pointer", i32 2}
