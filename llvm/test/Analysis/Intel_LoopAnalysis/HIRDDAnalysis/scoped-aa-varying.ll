; RUN: opt -hir-ssa-deconstruction %s | opt -scoped-noalias-aa -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region | FileCheck %s
; RUN: opt -hir-ssa-deconstruction %s | opt -scoped-noalias-aa -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=L2,L1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction %s | opt -scoped-noalias-aa -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=L1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction %s | opt -scoped-noalias-aa -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=L2 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction" | opt -passes="require<scoped-noalias-aa>,print<hir-dd-analysis>"  -hir-dd-analysis-verify=Region -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction" | opt -passes="require<scoped-noalias-aa>,print<hir-dd-analysis>"  -hir-dd-analysis-verify=L2,L1 -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction" | opt -passes="require<scoped-noalias-aa>,print<hir-dd-analysis>"  -hir-dd-analysis-verify=L1 -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction" | opt -passes="require<scoped-noalias-aa>,print<hir-dd-analysis>"  -hir-dd-analysis-verify=L2 -disable-output 2>&1 | FileCheck %s
; We'll check that we don't completely break the dependence between the
; references in the inner loop. A and B are marked 'restrict,' but this only
; tells us that they are distinct within each outer loop iteration -- not
; between all outer loop iterations.
;
; The many RUN lines above test all possible orderings for analyzing either or
; both loop levels. In all cases, the dependence in question should remain.
;
; C source:
; void foo(float ** restrict Ap, float** restrict Bp) {
;   for(int i=0; i<1024; i++) {
;       float* restrict A = Ap[i];
;       float* restrict B = Bp[i];
;       for(int j=0; j<1024; j++){
;           A[j] = B[j-1] + 4;
;       }
;     }
; }
;
; HIR:
; BEGIN REGION { }
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
;       |   %A = (%Ap)[i1];
;       |   %B = (%Bp)[i1];
;       |
;       |   + DO i2 = 0, 1023, 1   <DO_LOOP>
;       |   |   %add = (%B)[i2 + -1]  +  4.000000e+00;
;       |   |   (%A)[i2] = %add;
;       |   + END LOOP
;       + END LOOP
; END REGION

; CHECK-LABEL: DD graph for function foo:
; CHECK-DAG: (%B)[i2 + -1] --> (%A)[i2] ANTI (* 0)
; CHECK-DAG: (%A)[i2] --> (%B)[i2 + -1] FLOW (* 0)

define void @foo(float** noalias nocapture readonly %Ap, float** noalias nocapture readonly %Bp) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup5
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup5
  %indvars.iv27 = phi i64 [ 0, %entry ], [ %indvars.iv.next28, %for.cond.cleanup5 ]
  %ptridx = getelementptr inbounds float*, float** %Ap, i64 %indvars.iv27
  %A = load float*, float** %ptridx, align 8
  %ptridx2 = getelementptr inbounds float*, float** %Bp, i64 %indvars.iv27
  %B = load float*, float** %ptridx2, align 8
  br label %for.body6

for.cond.cleanup5:                                ; preds = %for.body6
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond29 = icmp eq i64 %indvars.iv.next28, 1024
  br i1 %exitcond29, label %for.cond.cleanup, label %for.body

for.body6:                                        ; preds = %for.body, %for.body6
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body6 ]
  %dec = add nsw i64 %indvars.iv, -1
  %ptridx8 = getelementptr inbounds float, float* %B, i64 %dec
  %load = load float, float* %ptridx8, align 4, !alias.scope !1, !noalias !4
  %add = fadd float %load, 4.000000e+00
  %ptridx10 = getelementptr inbounds float, float* %A, i64 %indvars.iv
  store float %add, float* %ptridx10, align 4, !alias.scope !4, !noalias !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.cond.cleanup5, label %for.body6
}

; This is a very similar case, except the references have unique indexing in
; every iteration. Because of this, the only dependence through memory is that
; between the two references.
;
; C source:
; void foo(float ** restrict Ap, float** restrict Bp) {
;   for(int i=0; i<1024; i++) {
;       float* restrict A = Ap[i];
;       float* restrict B = Bp[i];
;       for(int j=0; j<1024; j++){
;           A[i*1024 + j] = B[i*1024 + j-1] + 4;
;       }
;     }
; }
;
; HIR:
; BEGIN REGION { }
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
;       |   %A = (%Ap)[i1];
;       |   %B = (%Bp)[i1];
;       |
;       |   + DO i2 = 0, 1023, 1   <DO_LOOP>
;       |   |   %add = (%B)[1024 * i1 + i2 + -1]  +  4.000000e+00;
;       |   |   (%A)[1024 * i1 + i2] = %add;
;       |   + END LOOP
;       + END LOOP
; END REGION


; CHECK-LABEL: DD graph for function bar:
; CHECK-DAG: (%B)[1024 * i1 + i2 + -1] --> (%A)[1024 * i1 + i2] ANTI (* 0)
; CHECK-DAG: (%A)[1024 * i1 + i2] --> (%B)[1024 * i1 + i2 + -1] FLOW (* 0)

define void @bar(float** noalias nocapture readonly %Ap, float** noalias nocapture readonly %Bp) {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup5
  ret void

for.body:                                         ; preds = %entry, %for.cond.cleanup5
  %indvars.iv27 = phi i64 [ 0, %entry ], [ %indvars.iv.next28, %for.cond.cleanup5 ]
  %ptridx = getelementptr inbounds float*, float** %Ap, i64 %indvars.iv27
  %A = load float*, float** %ptridx, align 8
  %ptridx2 = getelementptr inbounds float*, float** %Bp, i64 %indvars.iv27
  %B = load float*, float** %ptridx2, align 8
  %offset = mul nuw nsw i64 %indvars.iv27, 1024
  br label %for.body6

for.cond.cleanup5:                                ; preds = %for.body6
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond29 = icmp eq i64 %indvars.iv.next28, 1024
  br i1 %exitcond29, label %for.cond.cleanup, label %for.body

for.body6:                                        ; preds = %for.body, %for.body6
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body6 ]
  %base = add nuw nsw i64 %indvars.iv, %offset
  %dec = add nsw i64 %base, -1
  %ptridx8 = getelementptr inbounds float, float* %B, i64 %dec
  %load = load float, float* %ptridx8, align 4, !alias.scope !1, !noalias !4
  %add = fadd float %load, 4.000000e+00
  %ptridx10 = getelementptr inbounds float, float* %A, i64 %base
  store float %add, float* %ptridx10, align 4, !alias.scope !4, !noalias !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %for.cond.cleanup5, label %for.body6
}

!1 = !{!2}
!2 = distinct !{!2, !3, !"foo: B"}
!3 = distinct !{!3, !"foo"}
!4 = !{!5}
!5 = distinct !{!5, !3, !"foo: A"}
