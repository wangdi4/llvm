; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 | FileCheck %s

; Verify that we remove the empty loop after the load is removed by temp cleanup.
; Note that the loop is not doing anything other than traversing the string @.str.94.

; CHECK: Dump Before 

; CHECK:          BEGIN REGION { }
; CHECK:             + DO i1 = 0, 1, 1   <DO_LOOP>
; CHECK:             |   %0 = (@.str.94)[0][i1 + 1];
; CHECK:             + END LOOP
; CHECK:          END REGION

; CHECK: Dump After 

; CHECK:          BEGIN REGION { }
; CHECK-NEXT:     END REGION


; ModuleID = 'bugpoint-reduced-simplified.bc'
source_filename = "bugpoint-output-a3f490f.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str.94 = private unnamed_addr constant [3 x i8] c"<%\00", align 1

; Function Attrs: norecurse nounwind uwtable
define void @main() local_unnamed_addr {
entry:
  br label %for.inc.i.i

for.inc.i.i:                                      ; preds = %for.inc.i.i, %entry
  %s2.addr.0.i.i9591070 = phi i8* [ getelementptr inbounds ([3 x i8], [3 x i8]* @.str.94, i64 0, i64 0), %entry ], [ %incdec.ptr4.i.i, %for.inc.i.i ]
  %incdec.ptr4.i.i = getelementptr inbounds i8, i8* %s2.addr.0.i.i9591070, i64 1
  %0 = load i8, i8* %incdec.ptr4.i.i, align 1
  %cmp3.i.i = icmp eq i8 %0, 0
  br i1 %cmp3.i.i, label %for.body.i.i23.preheader, label %for.inc.i.i

for.body.i.i23.preheader:                         ; preds = %for.inc.i.i
  ret void
}

