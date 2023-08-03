; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-cg" -hir-create-function-level-region -force-hir-cg -S 2>&1 %s | FileCheck %s

; Verify that we keep @llvm.experimental.noalias.scope.decl() calls in HIR if
; they are outside loops.


; CHECK: BEGIN REGION { }
; CHECK: @llvm.experimental.noalias.scope.decl(!0);
; CHECK: ret ;
; CHECK: END REGION


; CHECK: region.{{.*}}:
; CHECK-NEXT:  tail call void @llvm.experimental.noalias.scope.decl(metadata !0)


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @test() {
entry:
  br label %bb

bb:
  tail call void @llvm.experimental.noalias.scope.decl(metadata !0)
  ret void
}

declare void @llvm.experimental.noalias.scope.decl(metadata)

!0 = !{!1}
!1 = distinct !{!1, !2, !"copy: %to"}
!2 = distinct !{!2, !"copy"}
