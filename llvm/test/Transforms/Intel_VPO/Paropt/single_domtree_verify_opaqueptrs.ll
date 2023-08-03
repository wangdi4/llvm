; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-prepare -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -S %s 2>&1 | FileCheck %s

; Test src:
;
; void b() {
; #pragma omp master
;   ;
; }

; CHECK-NOT: DominatorTree update failed after Single codegen.
; CHECK: call i32 @__kmpc_single({{.*}})
; CHECK: call void @__kmpc_end_single({{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

define dso_local void @b() {
entry:
  br label %region.entry

region.entry:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  br label %body

body:                                         ; preds = %region.entry
  fence acquire
  fence release
  br label %region.exit

region.exit:                                  ; preds = %body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SINGLE"() ]
  br label %exit

exit:                                         ; preds = %region.exit
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!llvm.module.flags = !{!0}
!0 = !{i32 1, !"wchar_size", i32 4}
