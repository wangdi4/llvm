; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output 2>&1 | FileCheck %s

; Verify that the smin blob is not broken down using a multiplier of 2 like this-
; 2 * smin(50, (1 + (-1 * %a.030.out)))

; The transformation is invalid if operations inside smin overflow.

;       BEGIN REGION { }
; CHECK:      + DO i1 = 0, 49999, 1   <DO_LOOP>
;             |   %a.030.out = %a.030;
; CHECK:      |   %a.030 = 0  -  smin(100, (2 + (-2 * %a.030.out)));
; CHECK:      + END LOOP
;       END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@n = dso_local local_unnamed_addr global i32 50, align 4

define dso_local i32 @main() local_unnamed_addr {
entry:
  %0 = load i32, ptr @n, align 4
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %i.031 = phi i32 [ 0, %entry ], [ %inc12, %for.body ]
  %a.030 = phi i32 [ %0, %entry ], [ %sub9, %for.body ]
  %inc = shl i32 %a.030, 1
  %mul5 = sub i32 2, %inc
  %1 = icmp slt i32 %mul5, 100
  %spec.store.select18 = select i1 %1, i32 %mul5, i32 100
  %sub9 = sub nsw i32 0, %spec.store.select18
  %inc12 = add nuw nsw i32 %i.031, 1
  %exitcond = icmp eq i32 %inc12, 50000
  br i1 %exitcond, label %if.end17, label %for.body

if.end17:                                         ; preds = %for.body
  %a.030.lcssa = phi i32 [ %a.030, %for.body ]
  ret i32 %a.030.lcssa
}
