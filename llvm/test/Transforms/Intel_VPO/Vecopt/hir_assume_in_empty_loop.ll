; RUN: opt -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -vplan-force-vf=4 <%s -disable-output \
; RUN:     -print-after=VPlanDriverHIR 2>&1 | FileCheck %s

; RUN: opt -passes="hir-vec-dir-insert,vplan-driver-hir,print<hir>" -vplan-force-vf=4 \
; RUN:     <%s -disable-output 2>&1 | FileCheck %s

; Verify that vectorizer does not generate an empty HLIf node.
;
; Input:
; BEGIN REGION { }
;       + DO i1 = 0, %arg + -1, 1   <DO_LOOP>
;       |   @llvm.assume(undef);
;       + END LOOP
; END REGION
;
; And vectorizer was making the following for the loop:
;
;      %tgu = %arg/u4;
;      if (0 <u 4 * %tgu)   // This empty HLIf
;      {                    // was causing an assert in HIR verificaiton.
;      }
;
;      + DO i1 = 4 * %tgu, %arg + -1, 1
;      |   @llvm.assume(undef);
;      + END LOOP

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind
declare void @llvm.assume(i1)

; Function Attrs: uwtable
define void @foo(i64 %arg) {
; CHECK-LABEL: BEGIN REGION { modified }
; The first loop:
; CHECK-NEXT:    %tgu
; CHECK-NEXT: <{{[0-9]*}}>
; CHECK-NEXT:    DO i1 = 4 * %tgu{{.*}}, 1  <DO_LOOP>
; CHECK-NEXT:      @llvm.assume(undef)
; CHECK-NEXT:    END LOOP
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
