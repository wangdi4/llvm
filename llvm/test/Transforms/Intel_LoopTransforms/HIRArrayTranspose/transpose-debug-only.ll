; REQUIRES: asserts
; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-array-transpose" -debug-only=hir-array-transpose -disable-output 2>&1 < %s | FileCheck %s

; This test case is failing because community added support for ptrtoint in
; ScalarEvolution. The pass needs to be fixed.
; XFAIL: *

; Verify that array transpose kicks in for this test case.

; CHECK: HIR array transpose performed

;Module Before HIR; ModuleID = 'transpose.c'
source_filename = "transpose.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo() local_unnamed_addr "may_have_huge_local_malloc" {
entry:
  br label %preheader

preheader:
  %call = tail call noalias ptr @malloc(i64 80)
  %add.ptr = getelementptr inbounds i8, ptr %call, i64 8
  %intptr = ptrtoint ptr %add.ptr to i64
  %base = inttoptr i64 %intptr to ptr
  br label %for.body

for.body:                                         ; preds = %for.body, %preheader
  %indvars.iv = phi i64 [ 0, %preheader ], [ %indvars.iv.next, %for.body ]
  %t1 = mul nuw nsw i64 %indvars.iv, 5
  %t2 = add nsw i64 %t1, -2
  %arrayidx = getelementptr inbounds [10 x i64], ptr %base, i64 0, i64 %t2
  store i64 %indvars.iv, i64* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

; Function Attrs: nounwind
declare noalias ptr @malloc(i64) local_unnamed_addr

; Function Attrs: nounwind
declare void @free(ptr nocapture) local_unnamed_addr


!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 3a4f8e3753c5a4510523f6b550c786e660f95282) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm e10945e5709a2f787449ccc7a3338ed9f5f28beb)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
