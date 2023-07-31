; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir-framework>,hir-memory-reduction-sinking,print<hir-framework>" 2>&1 < %s | FileCheck %s
;
;
; Verify that we are able to handle FSub reduction correctly by adding the
; subtractive element inside the loop.

; Dump Before-

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %add7 = (@A)[0][5]  -  %t;
; CHECK: |   (@A)[0][5] = %add7;
; CHECK: + END LOOP

; Dump After
; CHECK: modified

; CHECK:   %tmp = 0.000000e+00;
; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   %tmp = %tmp  +  %t;
; CHECK: + END LOOP
; CHECK:   %add7 = (@A)[0][5]  -  %tmp;
; CHECK:   (@A)[0][5] = %add7;


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x float] zeroinitializer, align 16

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local void @foo(float %t) local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ld = load float, ptr getelementptr inbounds ([100 x float], ptr @A, i64 0, i64 5), align 4
  %add7 = fsub fast float %ld, %t
  store float %add7, ptr getelementptr inbounds ([100 x float], ptr @A, i64 0, i64 5), align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}

