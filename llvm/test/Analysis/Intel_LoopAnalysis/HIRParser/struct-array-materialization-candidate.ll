; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -hir-allow-loop-materialization-regions=true -disable-output  2>&1 | FileCheck %s

; Verify that we create a region for a bblock with stores to consecutive array
; locations where the array is a structure field.
; This is a valid loop materialization candidate.

; CHECK: BEGIN REGION

; CHECK: (%a)[0].1[0] = %n;
; CHECK: (%a)[0].1[1] = %n;
; CHECK: ret ;

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S = type { double, [10 x double] }

; Function Attrs: norecurse nounwind uwtable
define dso_local void @Vsub(ptr %a, double %n) local_unnamed_addr #0 {
entry:
  %gep = getelementptr inbounds %struct.S, ptr %a, i64 0, i32 1, i64 0
  store double %n, ptr %gep, align 8
  %gep1 = getelementptr inbounds %struct.S, ptr %a, i64 0, i32 1, i64 1
  store double %n, ptr %gep1, align 8
  ret void
}

