; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced, asserts
;
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>,hir-vplan-vec' -mattr=avx512f -mtriple=x86_64-unknown-unknown -enable-intel-advanced-opts -debug-only=ovls < %s 2>&1 | FileCheck %s --check-prefix=NOVLS
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vec-dir-insert,print<hir>,hir-vplan-vec' -mattr=avx2 -mtriple=x86_64-unknown-unknown -enable-intel-advanced-opts -debug-only=ovls < %s 2>&1 | FileCheck %s --check-prefix=VLS
;
; LIT test to check that OVLS groups are not formed for non-stride-2 masked accesses
; for core-avx512 compilation.

; Check that we do not see any vector of memrefs for core-avx512.
; NOVLS-LABEL:       Function: foo
; NOVLS-NOT:   Received a vector of memrefs ({{[1-9].*}})
; NOVLS-LABEL:       Function: foo2
; NOVLS-NOT:   Received a vector of memrefs ({{[1-9].*}})

; Check that we see vector of memrefs for core-avx2.
; VLS-LABEL:         Function: foo
; VLS:         Received a vector of memrefs ({{[1-9].*}})
; VLS-LABEL:         Function: foo2
; VLS:         Received a vector of memrefs ({{[1-9].*}})
;
define void @foo(ptr noalias %lp2) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %l1.017 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %rem = and i64 %l1.017, 1
  %tobool.not = icmp eq i64 %rem, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %mul1 = mul nuw nsw i64 %l1.017, 3
  %arrayidx2 = getelementptr inbounds i64, ptr %lp2, i64 %mul1
  store i64 %l1.017, ptr %arrayidx2, align 8
  %add5 = add nuw nsw i64 %mul1, 1
  %arrayidx6 = getelementptr inbounds i64, ptr %lp2, i64 %add5
  store i64 %mul1, ptr %arrayidx6, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %inc = add nuw nsw i64 %l1.017, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

; Check that a VLS memref is not created for a load of i16 type whose only use
; is in a uitofp instruction.
define void @foo2(ptr noalias %lp2) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %l1.017 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul1 = mul nuw nsw i64 %l1.017, 3
  %arrayidx2 = getelementptr inbounds i16, ptr %lp2, i64 %mul1
  %val = load i16, ptr %arrayidx2, align 2
  %valfp = uitofp i16 %val to float
  %inc = add nuw nsw i64 %l1.017, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
