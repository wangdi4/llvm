; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 | FileCheck %s
;
;
; LIT test to check that we do not hit an assert when we see a constant aggregate.
; Incoming HIR looks like the following:
;
;         + DO i1 = 0, 15, 1   <DO_LOOP>
;         |   %doubledouble = zeroinitializer;
;         |   if (%l1 == i1)
;         |   {
;         |      %doubledouble = { double 1.000000e+00, double 0.000000e+00 };
;         |   }
;         |   %double1 = extractvalue %doubledouble, 1;
;         + END LOOP
;
; We were hitting an assertion fail when dealing with the ConstantAggregate blob
; { double 1.000000e+00, double 0.000000e+00 }. Note that the loop does not get
; vectorized currently due to check for isVectorizableTy.
;
; CHECK: DO i1 = 0, 15, 1   <DO_LOOP>
; CHECK: END LOOP
;
define void @foo(ptr %lp, i64 %l1) {
entry:
  br label %for.header

for.header:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %if.end ]
  %cmp = icmp eq i64 %l1, %iv
  br i1 %cmp, label %if.then, label %if.end

if.then:
  br label %if.end

if.end:
  %doubledouble = phi { double, double } [ { double 1.0, double 0.0 }, %if.then ], [ zeroinitializer, %for.header ]
  %double1 = extractvalue { double, double } %doubledouble, 1
  %iv.inc = add nuw nsw i64 %iv, 1
  %cmp2 = icmp eq i64 %iv.inc, 16
  br i1 %cmp2, label %for.end, label %for.header

for.end:
  ret void
}