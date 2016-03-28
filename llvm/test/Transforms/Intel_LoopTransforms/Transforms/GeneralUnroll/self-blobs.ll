; Check that self-blob symbases are set after general unroll
; RUN: opt -hir-ssa-deconstruction -hir-general-unroll -print-before=hir-general-unroll -print-after=hir-general-unroll -hir-details < %s 2>&1 | FileCheck %s

; No source available;
;
; HIR Before:
;
;           BEGIN REGION { }
; <11>         + DO i1 = 0, 8 * %0 + -1, 1   <DO_LOOP>
; <2>          |   %hashVal.012.out = %hashVal.012;
; <4>          |   %hashVal.012 = %3  ^  2 * %hashVal.012.out;
; <11>         + END LOOP
;           END REGION
;
; HIR After:
;
;           BEGIN REGION { modified }
; <12>         %tgu = %0; <<< Here %0 symbase shoule be updated as it became as self-blob
; <13>         + DO i1 = 0, %tgu + -1, 1   <DO_LOOP>
; <15>         |   %hashVal.012.out = %hashVal.012;
; <16>         |   %hashVal.012 = %3  ^  2 * %hashVal.012.out;
; <17>         |   %hashVal.012.out = %hashVal.012;
; <18>         |   %hashVal.012 = %3  ^  2 * %hashVal.012.out;
; <19>         |   %hashVal.012.out = %hashVal.012;
; <20>         |   %hashVal.012 = %3  ^  2 * %hashVal.012.out;
; <21>         |   %hashVal.012.out = %hashVal.012;
; <22>         |   %hashVal.012 = %3  ^  2 * %hashVal.012.out;
; <23>         |   %hashVal.012.out = %hashVal.012;
; <24>         |   %hashVal.012 = %3  ^  2 * %hashVal.012.out;
; <25>         |   %hashVal.012.out = %hashVal.012;
; <26>         |   %hashVal.012 = %3  ^  2 * %hashVal.012.out;
; <27>         |   %hashVal.012.out = %hashVal.012;
; <28>         |   %hashVal.012 = %3  ^  2 * %hashVal.012.out;
; <29>         |   %hashVal.012.out = %hashVal.012;
; <30>         |   %hashVal.012 = %3  ^  2 * %hashVal.012.out;
; <13>         + END LOOP
; <11>         + DO i1 = 8 * %tgu, 8 * %0 + -1, 1   <DO_LOOP>
; <2>          |   %hashVal.012.out = %hashVal.012;
; <4>          |   %hashVal.012 = %3  ^  2 * %hashVal.012.out;
; <11>         + END LOOP
;           END REGION
;
; CHECK: Before
; CHECK: BEGIN REGION { }
; CHECK: DO i32 i1 = 0, 8 * %0 + -1, 1
; CHECK-NEXT: <RVAL-REG> LINEAR i32 8 * %0 + -1 {sb:2}
; CHECK-NEXT: <BLOB> LINEAR i32 %0 {sb:[[BLOBSB:[0-9]+]]}

; CHECK: After
; CHECK: BEGIN REGION { modified }
; CHECK: %tgu = %0

; CHECK-NEXT: <LVAL-REG>
; CHECK-NEXT: <RVAL-REG> LINEAR i32 %0 {sb:[[BLOBSB]]}

; ModuleID = 'self-blob.ll'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.xercesc_2_5::BitSet" = type<{%"class.xercesc_2_5::MemoryManager" *, i64*, i32, [4 x i8]}>
%"class.xercesc_2_5::MemoryManager" = type { i32(...) * * }

define i32 @_ZNK11xercesc_2_56BitSet4hashEj(%"class.xercesc_2_5::BitSet" * nocapture readonly %this, i32 %hashModulus) align 2 {
entry:
  %fUnitLen = getelementptr inbounds %"class.xercesc_2_5::BitSet", %"class.xercesc_2_5::BitSet"* %this, i64 0, i32 2
  %0 = load i32, i32* %fUnitLen, align 8
  %mul.mask = and i32 %0, 536870911
  %cmp11 = icmp eq i32 %mul.mask, 0
  br i1 %cmp11, label %for.cond.cleanup, label %for.body.lr.ph

for.body.lr.ph:                                   ; preds = %entry
  %fBits = getelementptr inbounds %"class.xercesc_2_5::BitSet", %"class.xercesc_2_5::BitSet"* %this, i64 0, i32 1
  %1 = bitcast i64** %fBits to i8**
  %2 = load i8*, i8** %1, align 8
  %3 = load i8, i8* %2, align 1
  %conv3 = zext i8 %3 to i32
  %4 = shl i32 %0, 3
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %hashVal.0.lcssa = phi i32 [ 0, %entry ], [ %xor, %for.cond.cleanup.loopexit ]
  %rem = urem i32 %hashVal.0.lcssa, %hashModulus
  ret i32 %rem

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %index.013 = phi i32 [ 0, %for.body.lr.ph ], [ %inc, %for.body ]
  %hashVal.012 = phi i32 [ 0, %for.body.lr.ph ], [ %xor, %for.body ]
  %shl = shl i32 %hashVal.012, 1
  %xor = xor i32 %conv3, %shl
  %inc = add nuw i32 %index.013, 1
  %exitcond = icmp eq i32 %inc, %4
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}
