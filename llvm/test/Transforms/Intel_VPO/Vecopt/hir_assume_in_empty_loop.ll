; RUN: opt -hir-vec-dir-insert -hir-vplan-vec -hir-cg -vplan-force-vf=4 <%s -disable-output \
; RUN:     -print-after=hir-vplan-vec 2>&1 | FileCheck %s

; RUN: opt -passes="hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 \
; RUN:     <%s -disable-output 2>&1 | FileCheck %s

; Verify that vectorizer does not generate an empty HLIf node.
;
; Input:
; BEGIN REGION { }
;       + DO i1 = 0, %arg + -1, 1   <DO_LOOP>
;       |   @llvm.assume(undef);
;       + END LOOP
; END REGION


; CHECK-LABEL:  BEGIN REGION { modified }
; CHECK:        if ({{.*}})
; CHECK:        {
; CHECK:           + DO i1 = 0, {{.*}}, 4   <DO_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CHECK:           |   @llvm.assume(undef);
; CHECK:           + END LOOP
; CHECK:        }
; CHECK:  END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind
declare void @llvm.assume(i1)

; Function Attrs: uwtable
define void @foo(i64 %arg) {
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  tail call void @llvm.assume(i1 undef)
  %cmp2 = icmp eq i64 %indvars.iv, %arg
  br i1 %cmp2, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  br label %for.inc

for.inc:                                          ; preds = %if.then, %for.body
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %arg
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}
