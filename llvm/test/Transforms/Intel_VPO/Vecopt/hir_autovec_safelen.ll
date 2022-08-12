; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -debug-only=parvec-analysis -print-after=hir-vec-dir-insert -print-after=hir-vplan-vec -disable-output -mattr=avx2 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>,hir-vplan-vec,print<hir>' -debug-only=parvec-analysis -disable-output -mattr=avx2 < %s 2>&1 | FileCheck %s
;
; LIT test to check for vectorization safe length in auto vectorization. Loop is
; vectorizable with safe length 3. Check for the same.
;
; HIR before vec directive insertion:
; <0>          BEGIN REGION { }
; <22>               + DO i1 = 0, 1023, 1   <DO_LOOP>
; <3>                |   %0 = (%lp1)[i1];
; <5>                |   (%lp1)[i1] = %0 + 1;
; <8>                |   (%lp1)[i1 + 9] = i1;
; <10>               |   %1 = (%lp3)[i1];
; <12>               |   (%lp2)[i1] = %1;
; <15>               |   (%lp3)[i1 + 3] = i1;
; <22>               + END LOOP
; <0>          END REGION
;
;
; CHECK:         8:3 (%lp1)[i1 + 9] --> (%lp1)[i1] FLOW (<) (9)
; CHECK-NEXT:    	is safe to vectorize with Safelen: 9
; CHECK-NEXT:    8:5 (%lp1)[i1 + 9] --> (%lp1)[i1] OUTPUT (<) (9)
; CHECK-NEXT:    	is safe to vectorize with Safelen: 9
;
; CHECK:         15:10 (%lp3)[i1 + 3] --> (%lp3)[i1] FLOW (<) (3)
; CHECK-NEXT:    	is safe to vectorize with Safelen: 3
;
; CHECK:                BEGIN REGION { }
; CHECK-NEXT:                 %entry.region = @llvm.directive.region.entry(); [ DIR.VPO.AUTO.VEC(),  QUAL.OMP.SAFELEN(3) ]
;
; CHECK:                      + DO i1 = 0, 1023, 1   <DO_LOOP>
; CHECK-NEXT:                 |   %0 = (%lp1)[i1];
; CHECK-NEXT:                 |   (%lp1)[i1] = %0 + 1;
; CHECK-NEXT:                 |   (%lp1)[i1 + 9] = i1;
; CHECK-NEXT:                 |   %1 = (%lp3)[i1];
; CHECK-NEXT:                 |   (%lp2)[i1] = %1;
; CHECK-NEXT:                 |   (%lp3)[i1 + 3] = i1;
; CHECK-NEXT:                 + END LOOP
;
; CHECK:                      @llvm.directive.region.exit(%entry.region); [ DIR.VPO.END.AUTO.VEC() ]
; CHECK-NEXT:           END REGION
;
; CHECK:                BEGIN REGION { modified }
; CHECK-NEXT:                + DO i1 = 0, 1023, 2   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:                |   %.vec = (<2 x i64>*)(%lp1)[i1];
; CHECK-NEXT:                |   (<2 x i64>*)(%lp1)[i1] = %.vec + 1;
; CHECK-NEXT:                |   (<2 x i64>*)(%lp1)[i1 + 9] = i1 + <i64 0, i64 1>;
; CHECK-NEXT:                |   %.vec1 = (<2 x i64>*)(%lp3)[i1];
; CHECK-NEXT:                |   (<2 x i64>*)(%lp2)[i1] = %.vec1;
; CHECK-NEXT:                |   (<2 x i64>*)(%lp3)[i1 + 3] = i1 + <i64 0, i64 1>;
; CHECK-NEXT:                + END LOOP
; CHECK-NEXT:           END REGION
;
define void @safelen_3(i64* noalias %lp1, i64* noalias writeonly %lp2, i64* noalias %lp3) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.018 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, i64* %lp1, i64 %l1.018
  %0 = load i64, i64* %arrayidx, align 8
  %add = add nsw i64 %0, 1
  store i64 %add, i64* %arrayidx, align 8
  %add1 = add nuw nsw i64 %l1.018, 9
  %arrayidx2 = getelementptr inbounds i64, i64* %lp1, i64 %add1
  store i64 %l1.018, i64* %arrayidx2, align 8
  %arrayidx3 = getelementptr inbounds i64, i64* %lp3, i64 %l1.018
  %1 = load i64, i64* %arrayidx3, align 8
  %arrayidx4 = getelementptr inbounds i64, i64* %lp2, i64 %l1.018
  store i64 %1, i64* %arrayidx4, align 8
  %add5 = add nuw nsw i64 %l1.018, 3
  %arrayidx6 = getelementptr inbounds i64, i64* %lp3, i64 %add5
  store i64 %l1.018, i64* %arrayidx6, align 8
  %inc = add nuw nsw i64 %l1.018, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
