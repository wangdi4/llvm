; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -disable-output -print-after=hir-vplan-vec  -vplan-force-vf=4 -vplan-enable-peeling < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -disable-output -vplan-force-vf=4 < %s 2>&1 -vplan-enable-peeling | FileCheck %s
;
; LIT test to check that we do not crash when peeling memory references with trailing
; offsets for alignment. Issue was seen during stress testing enabling dynamic peeling
; by default. The initial implementation of alignment analysis bails out for memory
; references with trailing struct offsets.
;
; Scalar HIR:
;               + DO i1 = 0, 1023, 1   <DO_LOOP>
;               |   (%sp)[i1].0 = i1;
;               + END LOOP
;
; CHECK:           + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; CHECK-NEXT:      |   (<4 x i64>*)(%sp)[i1].0 = i1 + <i64 0, i64 1, i64 2, i64 3>;
; CHECK-NEXT:      + END LOOP

%struct.S1 = type { i64 }

define void @baz(%struct.S1* %sp) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.05 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %a1 = getelementptr inbounds %struct.S1, %struct.S1* %sp, i64 %l1.05, i32 0
  store i64 %l1.05, i64* %a1, align 8
  %inc = add nuw nsw i64 %l1.05, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
