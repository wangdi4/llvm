; RUN: opt -passes="function(vpo-paropt-prepare)" -pass-remarks-output=%t -S %s
; RUN: FileCheck --input-file %t %s
; FIXME: This fails with new pass manager as genGlobalPrivatizationLaunderIntrin()
; inserts an empty basic block even when there is nothing to be laundered, which
; changes CFE, but it returns "Changed" as "false". This causes new PM's CFG verification
; to fail.
; COM: opt %s -passes='function(vpo-paropt-prepare)' -pass-remarks-output=%t -S
; COM: FileCheck --input-file %t %s
;
; The test does not declare a variant version of foo().
; Check that this does not cause compilation to fail.
;
; void foo() { }
; void bar() {
;   #pragma omp target variant dispatch
;   {
;      foo();
;   }
; }
;
; Check for the debug string
; CHECK: Pass:{{[ ]*}}openmp
; CHECK: Construct:{{[ ]*}}target variant dispatch
; CHECK: String:{{[ ]*}}' Could not find a matching variant function'
;

source_filename = "target_variant_dispatch_novariant.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @foo() {
entry:
  ret void
}

define dso_local void @bar() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  call void @foo() #2
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.2

DIR.OMP.END.TARGET.VARIANT.DISPATCH.2:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.2
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
