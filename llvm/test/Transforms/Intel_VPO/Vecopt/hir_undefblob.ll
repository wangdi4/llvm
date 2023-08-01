; LIT test to check that vectorizer does not choke on memory refs like undef[0]
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -disable-output < %s 2>&1 | FileCheck %s
;
; Incoming HIR looks like the following:
;           + DO i1 = 0, 99, 1
;           |   (%lp1)[i1] = i1;
;           |   (undef)[0] = i1;
;           + END LOOP
;
;
; CHECK:          + DO i1 = 0, 99, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:     |   (<4 x i64>*)(%lp1)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:     |   (undef)[0] = i1 + 3;
; CHECK-NEXT:     + END LOOP
;
define void @foo(ptr %lp1, ptr %lp2) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.06 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i64, ptr %lp1, i64 %l1.06
  store i64 %l1.06, ptr %arrayidx, align 8
  store i64 %l1.06, ptr undef, align 8
  %inc = add nuw nsw i64 %l1.06, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() #3
declare void @llvm.directive.region.exit(token) #3
