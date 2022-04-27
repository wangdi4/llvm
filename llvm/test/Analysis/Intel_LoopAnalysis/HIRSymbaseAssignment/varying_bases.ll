; Check that symbase assignment does not incorrectly use aliasGEP logic in
; BasicAA to conclude that the two stores are independent. Instead, it should
; reason that they have unknown/varying base pointers and use loopCarriedAlias
; with "UnknownSize", which will not be able to claim "NoAlias". As a result,
; we should see the two stores assigned to the same symbase.
;
; This test comes from the following C sources:
;
; void foo(float **A, int n) {
;   for(int i=0; i<n; i++) {
;     float *p = A[i];
;     p[0] = 1.0;
;     p[1] = 2.0;
;   }
; }

; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -enable-new-pm=0 -hir-framework -hir-details | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -aa-pipeline=basic-aa -hir-details -disable-output 2>&1 | FileCheck %s

; CHECK-DAG: {{\(NON-LINEAR float\* %first.*\[i64 0\].*}} {sb:[[Base1:[0-9]+]]}
; CHECK-DAG: {{\(NON-LINEAR float\* %first.*\[i64 1\].*}} {sb:[[Base1]]}

define void @foo(float** %A, i32 %n) {
entry:
  %cmp8 = icmp sgt i32 %n, 0
  br i1 %cmp8, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count10 = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds float*, float** %A, i64 %indvars.iv
  %first = load float*, float** %arrayidx, align 8
  store float 1.000000e+00, float* %first, align 4
  %second = getelementptr inbounds float, float* %first, i64 1
  store float 2.000000e+00, float* %second, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count10
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}
