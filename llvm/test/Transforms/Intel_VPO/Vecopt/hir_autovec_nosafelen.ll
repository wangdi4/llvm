; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -debug-only=parvec-analysis -print-after=hir-vec-dir-insert -disable-output -mattr=avx2 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>' -debug-only=parvec-analysis -disable-output -mattr=avx2 < %s 2>&1 | FileCheck %s
;
; LIT test to check that loop with a backward dependence of unknown distance
; is not marked as legal to vectorize.
;
; HIR before vec directive insertion
; Function: nosafelen
;
; <0>          BEGIN REGION { }
; <15>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%lp1)[i1];
; <5>                |   (%lp1)[i1] = %0 + 1;
; <8>                |   (%lp1)[i1 + %n1] = i1;
; <15>               + END LOOP
; <0>          END REGION
;
; CHECK:      8:3 (%lp1)[i1 + %n1] --> (%lp1)[i1] FLOW (*) (?)
; CHECK-NEXT: 	DV is not refinable - unsafe to vectorize
; CHECK-NEXT: 8:5 (%lp1)[i1 + %n1] --> (%lp1)[i1] OUTPUT (*) (?)
; CHECK-NEXT: 	DV is not refinable - unsafe to vectorize
;
; CHECK:                BEGIN REGION { }
; CHECK-NEXT:                 + DO i1 = 0, 1023, 1   <DO_LOOP>
; CHECK-NEXT:                 |   %0 = (%lp1)[i1];
; CHECK-NEXT:                 |   (%lp1)[i1] = %0 + 1;
; CHECK-NEXT:                 |   (%lp1)[i1 + %n1] = i1;
; CHECK-NEXT:                 + END LOOP
; CHECK-NEXT:           END REGION
;
define void @nosafelen(i64* noalias %lp1, i64 noundef %n1) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.09 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %lp1, i64 %l1.09
  %0 = load i64, i64* %arrayidx, align 8
  %add = add nsw i64 %0, 1
  store i64 %add, i64* %arrayidx, align 8
  %add1 = add nsw i64 %l1.09, %n1
  %arrayidx2 = getelementptr inbounds i64, i64* %lp1, i64 %add1
  store i64 %l1.09, i64* %arrayidx2, align 8
  %inc = add nuw nsw i64 %l1.09, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
