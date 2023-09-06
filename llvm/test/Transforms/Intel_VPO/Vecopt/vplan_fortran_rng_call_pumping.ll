; Verify that VPlan vectorizers correctly performs call pumping
; for masked Fortran RNG functions.

; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec" -vplan-force-vf=16 -vector-library=SVML -print-after=hir-vplan-vec -disable-output  < %s 2>&1 | FileCheck %s --check-prefix=HIR
; RUN: opt -passes="vplan-vec" -vplan-force-vf=16 -vector-library=SVML -print-after=vplan-vec -disable-output -S < %s 2>&1 | FileCheck %s --check-prefix=IR

; HIR:  BEGIN REGION { modified }
; HIR:        + DO i1 = 0, 1023, 16   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR:        |   %.vec = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> + 1 == 42;
; HIR:        |   %.extracted.subvec = shufflevector %.vec,  undef,  <i32 0, i32 1, i32 2, i32 3>;
; HIR:        |   %sext = sext.<4 x i1>.<4 x i32>(%.extracted.subvec);
; HIR:        |   %for_simd_random_number_single_mask = @for_simd_random_number_single_mask(%sext);
; HIR:        |   %.extracted.subvec2 = shufflevector %.vec,  undef,  <i32 4, i32 5, i32 6, i32 7>;
; HIR:        |   %sext3 = sext.<4 x i1>.<4 x i32>(%.extracted.subvec2);
; HIR:        |   %for_simd_random_number_single_mask4 = @for_simd_random_number_single_mask(%sext3);
; HIR:        |   %.extracted.subvec5 = shufflevector %.vec,  undef,  <i32 8, i32 9, i32 10, i32 11>;
; HIR:        |   %sext6 = sext.<4 x i1>.<4 x i32>(%.extracted.subvec5);
; HIR:        |   %for_simd_random_number_single_mask7 = @for_simd_random_number_single_mask(%sext6);
; HIR:        |   %.extracted.subvec8 = shufflevector %.vec,  undef,  <i32 12, i32 13, i32 14, i32 15>;
; HIR:        |   %sext9 = sext.<4 x i1>.<4 x i32>(%.extracted.subvec8);
; HIR:        |   %for_simd_random_number_single_mask10 = @for_simd_random_number_single_mask(%sext9);
; HIR:        |   %comb.shuf = shufflevector %for_simd_random_number_single_mask,  %for_simd_random_number_single_mask4,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>;
; HIR:        |   %comb.shuf11 = shufflevector %for_simd_random_number_single_mask7,  %for_simd_random_number_single_mask10,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>;
; HIR:        |   %comb.shuf12 = shufflevector %comb.shuf,  %comb.shuf11,  <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>;
; HIR:        + END LOOP
; HIR:        ret ;
; HIR:  END REGION

; IR-LABEL: define void @foo
; IR:       VPlannedBB4:
; IR-NEXT:    [[DOTPART_0_OF_4_:%.*]] = shufflevector <16 x i1> [[TMP0:%.*]], <16 x i1> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; IR-NEXT:    [[TMP1:%.*]] = sext <4 x i1> [[DOTPART_0_OF_4_]] to <4 x i32>
; IR-NEXT:    [[TMP2:%.*]] = call reassoc ninf nsz arcp contract afn <4 x float> @for_simd_random_number_single_mask(<4 x i32> [[TMP1]]) #[[ATTR1:[0-9]+]]
; IR-NEXT:    [[DOTPART_1_OF_4_:%.*]] = shufflevector <16 x i1> [[TMP0]], <16 x i1> undef, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
; IR-NEXT:    [[TMP3:%.*]] = sext <4 x i1> [[DOTPART_1_OF_4_]] to <4 x i32>
; IR-NEXT:    [[TMP4:%.*]] = call reassoc ninf nsz arcp contract afn <4 x float> @for_simd_random_number_single_mask(<4 x i32> [[TMP3]]) #[[ATTR1]]
; IR-NEXT:    [[DOTPART_2_OF_4_:%.*]] = shufflevector <16 x i1> [[TMP0]], <16 x i1> undef, <4 x i32> <i32 8, i32 9, i32 10, i32 11>
; IR-NEXT:    [[TMP5:%.*]] = sext <4 x i1> [[DOTPART_2_OF_4_]] to <4 x i32>
; IR-NEXT:    [[TMP6:%.*]] = call reassoc ninf nsz arcp contract afn <4 x float> @for_simd_random_number_single_mask(<4 x i32> [[TMP5]]) #[[ATTR1]]
; IR-NEXT:    [[DOTPART_3_OF_4_:%.*]] = shufflevector <16 x i1> [[TMP0]], <16 x i1> undef, <4 x i32> <i32 12, i32 13, i32 14, i32 15>
; IR-NEXT:    [[TMP7:%.*]] = sext <4 x i1> [[DOTPART_3_OF_4_]] to <4 x i32>
; IR-NEXT:    [[TMP8:%.*]] = call reassoc ninf nsz arcp contract afn <4 x float> @for_simd_random_number_single_mask(<4 x i32> [[TMP7]]) #[[ATTR1]]
; IR-NEXT:    [[TMP9:%.*]] = shufflevector <4 x float> [[TMP2]], <4 x float> [[TMP4]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; IR-NEXT:    [[TMP10:%.*]] = shufflevector <4 x float> [[TMP6]], <4 x float> [[TMP8]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; IR-NEXT:    [[COMBINED:%.*]] = shufflevector <8 x float> [[TMP9]], <8 x float> [[TMP10]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>


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
