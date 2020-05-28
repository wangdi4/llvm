; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" -disable-output 2>&1 | FileCheck %s

; The test case compfailed as we incorrectly tried to reorder use of
; %hir.de.ssa.copy1.out in %or before the definition of %2 (which is also the
; rval of %hir.de.ssa.copy1.out).
; This is because we were not including RvalDefInst in the legality checks.

; CHECK: + DO i1 = 0, 1431655765 * %.pr + -1, 1   <DO_LOOP>
; CHECK: |   %hir.de.ssa.copy1.out = %2;
; CHECK: |   %2 = %2  >>  7;
; CHECK: |   %or = %2 + -1 * %hir.de.ssa.copy1.out  |  7;
; CHECK: |   (@a)[0] = %or;
; CHECK: |   %3 = (%0)[0];
; CHECK: |   %2 = %2  +  %3;
; CHECK: |   (@d)[0] = %2;
; CHECK: |   (@c)[0] = 3 * i1 + %.pr + 3;
; CHECK: + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@c = common dso_local local_unnamed_addr global i32 0, align 4
@d = common dso_local local_unnamed_addr global i8 0, align 1
@a = common dso_local local_unnamed_addr global i8 0, align 1
@b = common dso_local local_unnamed_addr global i32* null, align 8

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  %.pr = load i32, i32* @c, align 4
  %tobool8 = icmp eq i32 %.pr, 0
  br i1 %tobool8, label %for.end, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %0 = load i32*, i32** @b, align 8
  %.pre = load i8, i8* @d, align 1
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %1 = phi i32 [ %.pr, %for.body.lr.ph ], [ %add6, %for.body ]
  %2 = phi i8 [ %.pre, %for.body.lr.ph ], [ %conv5, %for.body ]
  %shr = ashr i8 %2, 7
  %sub = sub i8 %shr, %2
  %or = or i8 %sub, 7
  store i8 %or, i8* @a, align 1
  %3 = load i32, i32* %0, align 4
  %4 = trunc i32 %3 to i8
  %conv5 = add i8 %shr, %4
  store i8 %conv5, i8* @d, align 1
  %add6 = add i32 %1, 3
  store i32 %add6, i32* @c, align 4
  %tobool = icmp eq i32 %add6, 0
  br i1 %tobool, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  ret void
}

