; RUN: opt -opaque-pointers=0 -passes="instcombine" %s -S | FileCheck %s
; CMPLRLLVM-24644: Instcombine should not remove any bitcasts that are
; used to match parameter types in callback functions
; These are identified by !callback MD on declare statements.

; CHECK: %STF_MODULE_entry = bitcast double* %STF_MODULE
; CHECK: call{{.*}}kmpc_fork_call{{.*}}%STF_MODULE_entry

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t.10.56.102.148.194.240.286.332.477 = type { i32, i32, i32, i32, i8* }

@.kmpc_loc.0.0.179 = external dso_local unnamed_addr global %struct.ident_t.10.56.102.148.194.240.286.332.477

define void @cfc_mod_mp_cfc_set_sflux_(double* %STF_MODULE) local_unnamed_addr #0 {
alloca_6:
  %STF_MODULE_entry = bitcast double* %STF_MODULE to [58 x [2 x [8 x [54 x double]]]]*
  call void (%struct.ident_t.10.56.102.148.194.240.286.332.477*, i32, void (i32*, i32*, ...)*, ...) @__kmpc_fork_call(%struct.ident_t.10.56.102.148.194.240.286.332.477* @.kmpc_loc.0.0.179, i32 16, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, [58 x [2 x [8 x [54 x double]]]]*)* @cfc_mod_mp_cfc_set_sflux_.DIR.OMP.PARALLEL.LOOP.21773.split1776 to void (i32*, i32*, ...)*), [58 x [2 x [8 x [54 x double]]]]* %STF_MODULE_entry)
  ret void
}

; Function Attrs: nounwind
declare !callback !1 void @__kmpc_fork_call(%struct.ident_t.10.56.102.148.194.240.286.332.477*, i32, void (i32*, i32*, ...)*, ...)

!1 = !{!2}
!2 = !{i64 2, i64 -1, i64 -1, i1 true}


declare dso_local void @cfc_mod_mp_cfc_set_sflux_.DIR.OMP.PARALLEL.LOOP.21773.split1776(i32*, i32*, [58 x [2 x [8 x [54 x double]]]]*)  #0

attributes #0 = { "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }

!nvvm.annotations = !{}
