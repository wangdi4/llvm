; Verify that VPlan HIR vectorizer does not replace Fortran RNG functions
; with SIMD variants in scalar remainder loop.

; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec" -vplan-force-vf=4 -vector-library=SVML -print-after=hir-vplan-vec -disable-output  < %s 2>&1 | FileCheck %s

; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK:        |   %for_simd_random_number = @for_simd_random_number();
; CHECK:        |   %for_simd_random_number2 = @for_simd_random_number();
; CHECK:        |   %for_simd_random_number_single_mask = @for_simd_random_number_single_mask({{%.*}});
; CHECK:        + END LOOP

; CHECK:        + DO i1 = 1024, 1024, 1   <DO_LOOP> <vector-remainder> <novectorize>
; CHECK:        |   %res2 = @for_random_number();
; CHECK:        |   if (i1 == 42)
; CHECK:        |   {
; CHECK:        |      %res4 = @for_random_number_single();
; CHECK:        |   }
; CHECK:        + END LOOP
; CHECK:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @foo() local_unnamed_addr #1 {
loop.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop.body

loop.body:                                      ; preds = %loop.ph, %if.merge
  %iv = phi i64 [ 0, %loop.ph ], [ %iv.next, %if.merge ]
  %res2 = tail call reassoc ninf nsz arcp contract afn double @for_random_number() #4
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
declare double @for_random_number() local_unnamed_addr #3

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #1 = { nofree nounwind uwtable "intel-lang"="fortran" }
attributes #4 = { nounwind }
