; Test VPValue based code generation
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -hir-cg -vplan-force-vf=4  -disable-output -print-after=VPlanDriverHIR -enable-vp-value-codegen-hir %s 2>&1 | FileCheck %s
; CHECK:        DO i1 = 0, 99, 4
; CHECK-NEXT:         (<4 x i64>*)(@ip)[0][i1] = i1 + <i64 0, i64 1, i64 2, i64 3>  +  2
; CHECK:        END LOOP
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ip = common dso_local local_unnamed_addr global [100 x i64] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable writeonly
define dso_local void @foo() local_unnamed_addr #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %index.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %add = add nuw nsw i64 %index.05, 2
  %arrayidx = getelementptr inbounds [100 x i64], [100 x i64]* @ip, i64 0, i64 %index.05
  store i64 %add, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %index.05, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
