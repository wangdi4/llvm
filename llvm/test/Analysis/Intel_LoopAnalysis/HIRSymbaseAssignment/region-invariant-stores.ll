; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -aa-pipeline=basic-aa -hir-details -hir-allow-loop-materialization-regions=true -disable-output 2>&1 | FileCheck %s

; Verify that the consecutive stores in region created for materialization
; candidate has different symbases.

; CHECK: BEGIN REGION

; CHECK: EntryBB: %entry.split

; CHECK: (%A)[1] = %t;
; CHECK-NEXT: (LINEAR ptr %A)[i64 1] inbounds  {sb:[[FIRSTSB:[0-9]+]]}

; CHECK: (%A)[2] = %t;
; CHECK-NOT: {sb:[[FIRSTSB]]}

; CHECK: ret ;

; CHECK: END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo(ptr nocapture %A, i32 %t) {
entry:
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 1
  store i32 %t, ptr %arrayidx, align 4
  %arrayidx1 = getelementptr inbounds i32, ptr %A, i64 2
  store i32 %t, ptr %arrayidx1, align 4
  ret void
}

