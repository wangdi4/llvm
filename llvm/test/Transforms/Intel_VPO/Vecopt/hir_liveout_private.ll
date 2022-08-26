; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

; LIT test to check code generated for liveout private.
;
define i64 @foo(i64* nocapture %larr) {
; CHECK:                    BEGIN REGION { modified }
; CHECK-NEXT:                     + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:                     |   [[DOTVEC0:%.*]] = (<4 x i64>*)([[LARR0:%.*]])[i1]
; CHECK-NEXT:                     |   (<4 x i64>*)([[LARR0]])[i1] = i1 + <i64 0, i64 1, i64 2, i64 3> + [[DOTVEC0]]
; CHECK-NEXT:                     + END LOOP
; CHECK:                          [[TMP0:%.*]] = extractelement [[DOTVEC0]],  3
; CHECK:                    END REGION
;
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %l1.010 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %larr, i64 %l1.010
  %0 = load i64, i64* %arrayidx, align 8
  %add = add nsw i64 %0, %l1.010
  store i64 %add, i64* %arrayidx, align 8
  %inc = add nuw nsw i64 %l1.010, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret i64 %0
}
