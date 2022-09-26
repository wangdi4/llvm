; LIT test to check vector code generation of a uniform call under mask.
; The test checks that we do a scalar call under appropriate if condition.
;
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s


; Scalar HIR:
;    DO i1 = 0, 99, 1   <DO_LOOP>
;      %0 = (%arr)[i1];
;      if (i1 > %0)
;      {
;         @llvm.assume(%uni);
;      }
;    END LOOP
;
define void @foo(i1 %uni, i64* nocapture readonly %arr) {
; CHECK:        + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:   |   %.vec = (<4 x i64>*)(%arr)[i1];
; CHECK-NEXT:   |   %.vec1 = i1 + <i64 0, i64 1, i64 2, i64 3> > %.vec;
; CHECK-NEXT:   |   %0 = bitcast.<4 x i1>.i4(%.vec1);
; CHECK-NEXT:   |   %cmp = %0 != 0;
; CHECK-NEXT:   |   if (%cmp == 1)
; CHECK-NEXT:   |   {
; CHECK-NEXT:   |      @llvm.assume(%uni);
; CHECK-NEXT:   |   }
; CHECK-NEXT:   + END LOOP
;
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %l1.09 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds i64, i64* %arr, i64 %l1.09
  %0 = load i64, i64* %arrayidx, align 8
  %cmp1 = icmp sgt i64 %l1.09, %0
  br i1 %cmp1, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  tail call void @llvm.assume(i1 %uni)
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %inc = add nuw nsw i64 %l1.09, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}


; Scalar HIR:
;    DO i1 = 0, 99, 1   <DO_LOOP>
;      if (%n > 10)
;      {
;         @llvm.assume(%uni);
;      }
;    END LOOP
;
define void @foo_uniform_if(i64 %n, i1 %uni) {
; CHECK:        + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:   |   if (%n > 10)
; CHECK-NEXT:   |   {
; CHECK-NEXT:   |      @llvm.assume(%uni);
; CHECK-NEXT:   |   }
; CHECK-NEXT:   + END LOOP
;
entry:
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %l1.09 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %uniform.cond = icmp sgt i64 %n, 10
  br i1 %uniform.cond, label %if.then, label %for.inc

if.then:                                          ; preds = %for.body
  tail call void @llvm.assume(i1 %uni)
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %inc = add nuw nsw i64 %l1.09, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

declare void @llvm.assume(i1)
