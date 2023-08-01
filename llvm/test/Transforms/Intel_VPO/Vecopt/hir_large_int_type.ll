;
; RUN: opt -disable-output -hir-allow-large-integers -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' < %s 2>&1 | FileCheck %s
;
; LIT test to check that we do not crash in HIR vectorizer when dealing with
; integer type larger than 64 bits.
;
; CHECK:        BEGIN REGION { modified }
; CHECK-NEXT:         %red.init = 0;
; CHECK-NEXT:         %red.init.insert = insertelement %red.init,  %x.07,  0;
; CHECK-NEXT:         %phi.temp = %red.init.insert;
;
; CHECK:              + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:         |   %.vec = (<4 x i128>*)(%a)[i1];
; CHECK-NEXT:         |   %.vec1 = %.vec  >>  1;
; CHECK-NEXT:         |   %.vec2 = %.vec1  +  %phi.temp;
; CHECK-NEXT:         |   %phi.temp = %.vec2;
; CHECK-NEXT:         + END LOOP
;
; CHECK:              %x.07 = @llvm.vector.reduce.add.v4i128(%.vec2);
; CHECK-NEXT:   END REGION
;
define { i64, i64 } @foo(ptr  %a) {
entry:
  br label %for.body

for.body:
  %x.07 = phi i128 [ 0, %entry ], [ %add, %for.body ]
  %i.06 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds i128, ptr %a, i64 %i.06
  %0 = load i128, ptr %arrayidx, align 16
  %shr = ashr i128 %0, 1
  %add = add nsw i128 %shr, %x.07
  %inc = add nuw nsw i64 %i.06, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  %retval.sroa.0.0.extract.trunc = trunc i128 %add to i64
  %retval.sroa.2.0.extract.shift = lshr i128 %add, 64
  %retval.sroa.2.0.extract.trunc = trunc i128 %retval.sroa.2.0.extract.shift to i64
  %.fca.0.insert = insertvalue { i64, i64 } poison, i64 %retval.sroa.0.0.extract.trunc, 0
  %.fca.1.insert = insertvalue { i64, i64 } %.fca.0.insert, i64 %retval.sroa.2.0.extract.trunc, 1
  ret { i64, i64 } %.fca.1.insert
}
