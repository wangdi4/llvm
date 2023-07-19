; Verify that VPlan vectorizes Fortran specific library calls using
; their SIMD variants.

; RUN: opt < %s -passes="vplan-vec" -vplan-force-vf=4 -vector-library=SVML -S | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec" -vplan-force-vf=4 -vector-library=SVML -print-after=hir-vplan-vec -disable-output  < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @foo() local_unnamed_addr #1 {
loop.ph:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %loop.body

loop.body:                                      ; preds = %loop.ph, %if.merge
  %iv = phi i64 [ 1, %loop.ph ], [ %iv.next, %if.merge ]
  %res1 = tail call reassoc ninf nsz arcp contract afn double @for_random_number() #4
  ; CHECK-2: @for_simd_random_number()

  %res2 = tail call reassoc ninf nsz arcp contract afn float @for_random_number_single() #4
  ; CHECK: @for_simd_random_number_single()

  %cmp = icmp eq i64 %iv, 42
  br i1 %cmp, label %if.then, label %if.merge

if.then:                                        ; preds = %loop.body
  %res3 = tail call reassoc ninf nsz arcp contract afn double @for_random_number() #4
  ; CHECK-2: @for_simd_random_number_mask({{.*}})

  %res4 = tail call reassoc ninf nsz arcp contract afn float @for_random_number_single() #4
  ; CHECK: @for_simd_random_number_single_mask({{.*}})

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
declare double @for_random_number() local_unnamed_addr #3

; Function Attrs: nofree
declare float @for_random_number_single() local_unnamed_addr #3

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #1 = { nofree nounwind uwtable "intel-lang"="fortran" }
attributes #4 = { nounwind }
