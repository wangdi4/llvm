; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that %add which is the base inst of single operand phi %split43 is marked as region livein.

; CHECK: LiveIns: %add(%add), %q(%q), %i(0)

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 0, 1   <DO_LOOP>
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   (%q)[0] = %add;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @main(i32* %q) {
entry:
  %add = add i32 undef, 0
  br label %for.outer

for.outer:
  %i = phi i32 [ 0, %entry ], [ %ip, %for.end ]
  br label %for.inner

for.inner:
  br label %for.body9

for.body9:
  %and34 = phi i32 [ %add, %for.inner]
  %tobool = icmp eq i32 1, 0
  br i1 %tobool, label %for.inner, label %for.end

for.end:
  %split43 = phi i32 [ %and34, %for.body9 ]
  store i32 %split43, i32* %q
  %ip = add i32 %i, 1
  %cmp3 = icmp ult i32 %ip, 100
  br i1 %cmp3, label %for.outer, label %exit

exit:
  ret void
}

