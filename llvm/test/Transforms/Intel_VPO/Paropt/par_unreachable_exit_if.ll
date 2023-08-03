; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,simplifycfg,loop-simplify,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:

; a() __attribute__((__noreturn__));
; namespace b {
; c() {
; #pragma omp parallel if (c)
;   a();
; }
; } // namespace b
;
; Check that we are able to generate code for parallel if(...) for the region
; without crashing.
; CHECK:         br i1 true, label {{%.*}}, label {{%.*}}
; CHECK:       if.then:
; CHECK-NEXT:    call void {{.*}} @__kmpc_fork_call({{.*}})
; CHECK:       if.else:
; CHECK:         call void @__kmpc_serialized_parallel({{.*}})
; CHECK:         call void @[[OUTLINED_FUNCTION:_ZN1b1cEv.DIR.OMP.PARALLEL[^(]*]]({{.*}})
; CHECK:         call void @__kmpc_end_serialized_parallel({{.*}})

; CHECK:       define internal void @[[OUTLINED_FUNCTION]]({{.*}})
; CHECK:         %call = call i32 @_Z1av()
; CHECK:         unreachable

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @_ZN1b1cEv() {
entry:
  %retval = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.IF"(i1 true) ]
  %call = call i32 @_Z1av()
  unreachable

dummy:                                            ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %1 = load i32, ptr %retval, align 4
  ret i32 %1
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @_Z1av()
