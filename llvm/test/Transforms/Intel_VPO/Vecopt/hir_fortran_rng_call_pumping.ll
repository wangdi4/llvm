; Verify that HIR VPlan vectorizer correctly performs call pumping
; for masked Fortran RNG functions.

; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec" -vplan-force-vf=16 -vector-library=SVML -print-after=hir-vplan-vec -disable-output  < %s 2>&1 | FileCheck %s

; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 1023, 16   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK:        |   %.vec = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> + 1 == 42;
; CHECK:        |   %.extracted.subvec = shufflevector %.vec,  undef,  <i32 0, i32 1, i32 2, i32 3>;
; CHECK:        |   %for_simd_random_number_single_mask = @for_simd_random_number_single_mask(%.extracted.subvec);
; CHECK:        |   %.extracted.subvec2 = shufflevector %.vec,  undef,  <i32 4, i32 5, i32 6, i32 7>;
; CHECK:        |   %for_simd_random_number_single_mask3 = @for_simd_random_number_single_mask(%.extracted.subvec2);
; CHECK:        |   %.extracted.subvec4 = shufflevector %.vec,  undef,  <i32 8, i32 9, i32 10, i32 11>;
; CHECK:        |   %for_simd_random_number_single_mask5 = @for_simd_random_number_single_mask(%.extracted.subvec4);
; CHECK:        |   %.extracted.subvec6 = shufflevector %.vec,  undef,  <i32 12, i32 13, i32 14, i32 15>;
; CHECK:        |   %for_simd_random_number_single_mask7 = @for_simd_random_number_single_mask(%.extracted.subvec6);
; CHECK:        |   %comb.shuf = shufflevector %for_simd_random_number_single_mask,  %for_simd_random_number_single_mask3,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>;
; CHECK:        |   %comb.shuf8 = shufflevector %for_simd_random_number_single_mask5,  %for_simd_random_number_single_mask7,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>;
; CHECK:        |   %comb.shuf9 = shufflevector %comb.shuf,  %comb.shuf8,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>;
; CHECK:        + END LOOP
; CHECK:        ret ;
; CHECK:  END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @foo() local_unnamed_addr #1 {
loop.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop.body

loop.body:                                      ; preds = %loop.ph, %if.merge
  %iv = phi i64 [ 1, %loop.ph ], [ %iv.next, %if.merge ]
  %cmp = icmp eq i64 %iv, 42
  br i1 %cmp, label %if.then, label %if.merge

if.then:                                        ; preds = %loop.body
  %res4 = tail call reassoc ninf nsz arcp contract afn float @for_random_number_single() #4
  br label %if.merge

if.merge:                                       ; preds = %if.then, %loop.body
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond.not = icmp eq i64 %iv.next, 1025
  br i1 %exitcond.not, label %loop.exit, label %loop.body

loop.exit:                                      ; preds = %if.merge
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nofree
declare float @for_random_number_single() local_unnamed_addr #3

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #1 = { nofree nounwind uwtable "intel-lang"="fortran" }
attributes #4 = { nounwind }
