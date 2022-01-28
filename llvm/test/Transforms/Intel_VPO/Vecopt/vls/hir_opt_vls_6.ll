; Test to check that we generate wide load and appropriate shuffles for the
; two loads. The first value loaded is not from the lowest memory address
; and the memory reference for the load has a trailing struct offset. Test
; checks that we generate proper HIR for this case.
;
; HIR before vectorization:
;         + DO i1 = 0, 99, 1   <DO_LOOP>
;         |   %0 = (%sarr)[i1].1;
;         |   %1 = (%sarr)[i1].0;
;         |   (%arr)[i1] = %0 + %1;
;         + END LOOP
;
; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -vplan-force-vf=4 -print-after=hir-vplan-vec -disable-output < %s 2>&1  | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
; CHECK:        + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:   |   %gep.base = &((i64*)(%sarr)[i1].1);
; CHECK-NEXT:   |   %.vls.load = (<8 x i64>*)(%gep.base)[-1];
; CHECK-NEXT:   |   %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK-NEXT:   |   %vls.extract1 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK-NEXT:   |   (<4 x i32>*)(%arr)[i1] = %vls.extract + %vls.extract1;
; CHECK-NEXT:   + END LOOP
target triple = "x86_64-unknown-linux-gnu"
%struct.S1 = type { i64, i64 }

define dso_local void @foo(i32* noalias nocapture %arr, %struct.S1* noalias nocapture readonly %sarr) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.09 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %b = getelementptr inbounds %struct.S1, %struct.S1* %sarr, i64 %l1.09, i32 1
  %0 = load i64, i64* %b, align 8
  %a = getelementptr inbounds %struct.S1, %struct.S1* %sarr, i64 %l1.09, i32 0
  %1 = load i64, i64* %a, align 8
  %add = add nsw i64 %1, %0
  %conv = trunc i64 %add to i32
  %ptridx2 = getelementptr inbounds i32, i32* %arr, i64 %l1.09
  store i32 %conv, i32* %ptridx2, align 4
  %inc = add nuw nsw i64 %l1.09, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
