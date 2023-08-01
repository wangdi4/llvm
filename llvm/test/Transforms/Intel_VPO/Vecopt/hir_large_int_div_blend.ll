;
; RUN: opt -disable-output -hir-allow-large-integers -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' < %s 2>&1 | FileCheck %s
;
; LIT test to check that we do not crash in HIR vectorizer when trying to
; blend masked divide with safe values for integer type larger than 64 bits.
; Here the constant 18446744073709551616 does not fit in 64 bits and was
; causing an assertion fail when calling getSExtValue().
;
; Incoming HIR:
;
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
;       |   %0 = (%a)[i1];
;       |   if (%0 < 1024)
;       |   {
;       |      %div = %0  /  18446744073709551616;
;       |      (%a)[i1] = %div;
;       |   }
;       + END LOOP
;
; CHECK:        BEGIN REGION { modified }
; CHECK-NEXT:          + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:         |   %.vec = (<4 x i128>*)(%a)[i1];
; CHECK-NEXT:         |   %.vec1 = %.vec < 1024;
; CHECK-NEXT:         |   %.vec2 = %.vec  /  18446744073709551616;
; CHECK-NEXT:         |   (<4 x i128>*)(%a)[i1] = %.vec2, Mask = @{%.vec1};
; CHECK-NEXT:         + END LOOP
; CHECK-NEXT:   END REGION
;
define void @foo(ptr  %a) {
entry:
  br label %for.body

for.body:
  %i.06 = phi i64 [ 0, %entry ], [ %inc, %if.end ]
  %arrayidx = getelementptr inbounds i128, ptr %a, i64 %i.06
  %0 = load i128, ptr %arrayidx, align 16
  %cmp = icmp slt i128 %0, 1024
  br i1 %cmp, label %if.then, label %if.end

if.then:
  %div = sdiv i128 %0, 18446744073709551616
  store i128 %div, ptr %arrayidx, align 16
  br label %if.end

if.end:
  %inc = add nuw nsw i64 %i.06, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  ret void
}
