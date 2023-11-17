;
; RUN: opt -disable-output -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-force-vf=4 -hir-details-dims < %s 2>&1 | FileCheck %s
;
; LIT test to ensure that VLS code generation is done correctly when we have
; a mismatch between the type used to compute access address and the type
; being loaded stored. For the test below, the incoming HIR looks like the
; following:
;
;       + DO i1 = 0, 1023, 1   <DO_LOOP>
;       |   %0 = (i64*)(%s1p)[0:16 * i1 + 8:1(i8:0)];
;       |   %1 = (%s1p)[0:i1:16(%struct.S1:0)].0;
;       + END LOOP
;
; The two loads are 8-bytes apart and have stride of 16. VLS is able
; to form a load group for the same. Note that the type of the first
; load(i64) and the type used in the address computation(i8) do not
; match. As a result, trying to offset the address with -1 for doing
; the wide load would be incorrect.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.S1 = type { i64, i64 }

;
; Check that we do not try to optimize the address for the wide load
; by trying to offset the index with -1.
;
; CHECK:         + DO i1 = 0, 1023, 4   <DO_LOOP> <auto-vectorized>
; CHECK-NEXT:    |   %gep.base = &((i8*)(%s1p)[0:16 * i1 + 8:1(i8:0)]);
; CHECK-NEXT:    |   %.vls.load = (<8 x i64>*)(%gep.base)[0:-1:8(i64:0)];
; CHECK-NEXT:    |   %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; CHECK-NEXT:    |   %vls.extract1 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; CHECK-NEXT:    + END LOOP
;
define void @foo(ptr noalias %s1p) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %l1.010 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %s1p.addr.09 = phi ptr [ %s1p, %entry ], [ %incdec.ptr, %for.body ]
  %add.ptr = getelementptr inbounds i8, ptr %s1p.addr.09, i64 8
  %0 = load i64, ptr %add.ptr, align 8
  %1 = load i64, ptr %s1p.addr.09, align 8
  %incdec.ptr = getelementptr inbounds %struct.S1, ptr %s1p.addr.09, i64 1
  %inc = add nuw nsw i64 %l1.010, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret void
}
