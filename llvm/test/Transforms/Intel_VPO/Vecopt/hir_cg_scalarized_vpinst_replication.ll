; Check HIR vector CG's handling of scalarized vector blobs. These blobs
; need to be replicated if they are used in revectorized context.

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -disable-output -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Input HIR:
;   + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
;   |   %0 = (%lp)[0];
;   |   (<2 x i32>*)(%darr)[i1] = %0;
;   + END LOOP
define void @scalarized_load(<2 x i32>* nocapture readonly %lp, i64* nocapture %darr) {
; CHECK-LABEL: Function: scalarized_load
; CHECK:         + DO i1 = 0, 99, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:    |   %.unifload = (%lp)[0];
; CHECK-NEXT:    |   %.replicated = shufflevector %.unifload,  undef,  <i32 0, i32 1, i32 0, i32 1, i32 0, i32 1, i32 0, i32 1>;
; CHECK-NEXT:    |   (<8 x i32>*)(%darr)[i1] = %.replicated;
; CHECK-NEXT:    + END LOOP
;
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %l1.09 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %0 = load <2 x i32>, <2 x i32>* %lp, align 8
  %arrayidx2 = getelementptr inbounds i64, i64* %darr, i64 %l1.09
  %bc = bitcast i64* %arrayidx2 to <2 x i32>*
  store <2 x i32> %0, <2 x i32>* %bc, align 8
  %inc = add nuw nsw i64 %l1.09, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Input HIR:
;   + DO i1 = 0, 127, 1   <DO_LOOP> <simd>
;   |   %ld = (%a)[i1];
;   |   %vec = bitcast.i64.<2 x i32>(%ld);
;   |   %shuffle = shufflevector %vec,  %vec,  <i32 3, i32 0>;
;   |   %ld.uni = (%b)[0];
;   |   %vec.uni = bitcast.i64.<2 x i32>(%ld.uni);
;   |   %shuffle.uni = shufflevector %vec.uni,  %vec.uni,  <i32 3, i32 0>;
;   |   (<2 x i32>*)(%p)[i1] = %shuffle.uni;
;   + END LOOP
define void @scalarized_shuffle(i64 *%a, i64 *%b, i64 *%p) {
; CHECK-LABEL: Function: scalarized_shuffle
; CHECK:         + DO i1 = 0, 127, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:    |   %.vec = (<4 x i64>*)(%a)[i1];
; CHECK-NEXT:    |   %.vec2 = bitcast.<4 x i64>.<8 x i32>(%.vec);
; CHECK-NEXT:    |   %shuffle3 = shufflevector %.vec2,  %.vec2,  <i32 9, i32 0, i32 11, i32 2, i32 13, i32 4, i32 15, i32 6>;
; CHECK-NEXT:    |   %.unifload = (%b)[0];
; CHECK-NEXT:    |   %.vec4 = bitcast.<4 x i64>.<8 x i32>(%.unifload);
; CHECK-NEXT:    |   %extractsubvec. = shufflevector %.vec4,  undef,  <i32 0, i32 1>;
; CHECK-NEXT:    |   %extractsubvec.5 = shufflevector %.vec4,  undef,  <i32 0, i32 1>;
; CHECK-NEXT:    |   %shuffle6 = shufflevector %extractsubvec.,  %extractsubvec.5,  <i32 3, i32 0>;
; CHECK-NEXT:    |   %.replicated = shufflevector %shuffle6,  undef,  <i32 0, i32 1, i32 0, i32 1, i32 0, i32 1, i32 0, i32 1>;
; CHECK-NEXT:    |   (<8 x i32>*)(%p)[i1] = %.replicated;
; CHECK-NEXT:    + END LOOP
;
 entry:
   %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
   br label %header

 header:
   %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]

   %ptr = getelementptr i64, i64 *%a, i64 %iv
   %ld = load i64, i64 *%ptr
   %vec = bitcast i64 %ld to <2 x i32>
   %shuffle = shufflevector <2 x i32> %vec, <2 x i32> %vec, <2 x i32><i32 3, i32 0>

   %ld.uni = load i64, i64 *%b
   %vec.uni = bitcast i64 %ld.uni to <2 x i32>
   %shuffle.uni = shufflevector <2 x i32> %vec.uni, <2 x i32> %vec.uni, <2 x i32><i32 3, i32 0>

  %gep = getelementptr i64, i64 *%p, i64 %iv
  %gep.cast = bitcast i64 *%gep to <2 x i32> *
  store <2 x i32> %shuffle.uni, <2 x i32> *%gep.cast

   %iv.next = add nuw nsw i64 %iv, 1
   %exitcond = icmp eq i64 %iv.next, 128
   br i1 %exitcond, label %exit, label %header

 exit:
   call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
   ret void
 }

 declare token @llvm.directive.region.entry()
 declare void @llvm.directive.region.exit(token)
