; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation -print-after=hir-last-value-computation < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,print<hir>" -aa-pipeline="basic-aa" 2>&1 < %s | FileCheck %s

; Verify that we do not move '%LargestWidenIdx.084 = i1' to postexit.
; Domination utility incorrrectly evaluated it as dominating the last loop child 'for.inc24:'.

; CHECK-NOT: modified

; CHECK: + DO i1 = 0, %umax + -1, 1   <DO_LOOP>
; CHECK: |   switch(%t5)
; CHECK: |   {
; CHECK: |   case 3:
; CHECK: |      goto sw.bb;
; CHECK: |   case 1:
; CHECK: |      goto sw.bb;
; CHECK: |   case 2:
; CHECK: |      break;
; CHECK: |   case 4:
; CHECK: |      break;
; CHECK: |   case 9:
; CHECK: |      goto for.inc24;
; CHECK: |   default:
; CHECK: |      %SmallestLegalizableToSameSizeIdx.085 = (%SmallestLegalizableToSameSizeIdx.085 == -1) ? i1 : %SmallestLegalizableToSameSizeIdx.085;
; CHECK: |      %LargestLegalizableToSameSizeIdx.086 = i1;
; CHECK: |      goto for.inc24;
; CHECK: |   }
; CHECK: |   %LargestWidenIdx.084 = i1;
; CHECK: |   goto for.inc24;
; CHECK: |   sw.bb:
; CHECK: |   %SmallestNarrowIdx.083 = (%SmallestNarrowIdx.083 == -1) ? i1 : %SmallestNarrowIdx.083;
; CHECK: |   for.inc24:
; CHECK: + END LOOP

define void @foo(i8 %t5, i64 %umax) {
entry:
br label %for.body12

for.body12:                                       ; preds = %for.inc24, %entry
  %i.087 = phi i64 [ 0, %entry ], [ %inc, %for.inc24 ]
  %LargestLegalizableToSameSizeIdx.086 = phi i32 [ -1, %entry ], [ %LargestLegalizableToSameSizeIdx.1, %for.inc24 ]
  %SmallestLegalizableToSameSizeIdx.085 = phi i32 [ -1, %entry ], [ %SmallestLegalizableToSameSizeIdx.2, %for.inc24 ]
  %LargestWidenIdx.084 = phi i32 [ -1, %entry ], [ %LargestWidenIdx.1, %for.inc24 ]
  %SmallestNarrowIdx.083 = phi i32 [ -1, %entry ], [ %SmallestNarrowIdx.1, %for.inc24 ]
  switch i8 %t5, label %sw.default [
    i8 3, label %sw.bb
    i8 1, label %sw.bb
    i8 2, label %sw.bb17
    i8 4, label %sw.bb17
    i8 9, label %for.inc24
  ]

sw.bb:                                            ; preds = %for.body12, %for.body12
  %cmp15 = icmp eq i32 %SmallestNarrowIdx.083, -1
  %conv16 = trunc i64 %i.087 to i32
  %spec.select = select i1 %cmp15, i32 %conv16, i32 %SmallestNarrowIdx.083
  br label %for.inc24

sw.bb17:                                          ; preds = %for.body12, %for.body12
  %conv18 = trunc i64 %i.087 to i32
  br label %for.inc24

sw.default:                                       ; preds = %for.body12
  %cmp19 = icmp eq i32 %SmallestLegalizableToSameSizeIdx.085, -1
  %conv21 = trunc i64 %i.087 to i32
  %spec.select72 = select i1 %cmp19, i32 %conv21, i32 %SmallestLegalizableToSameSizeIdx.085
  br label %for.inc24

for.inc24:                                        ; preds = %sw.bb, %sw.bb17, %for.body12, %sw.default
  %SmallestNarrowIdx.1 = phi i32 [ %SmallestNarrowIdx.083, %sw.default ], [ %SmallestNarrowIdx.083, %for.body12 ], [ %SmallestNarrowIdx.083, %sw.bb17 ], [ %spec.select, %sw.bb ]
  %LargestWidenIdx.1 = phi i32 [ %LargestWidenIdx.084, %sw.default ], [ %LargestWidenIdx.084, %for.body12 ], [ %conv18, %sw.bb17 ], [ %LargestWidenIdx.084, %sw.bb ]
  %SmallestLegalizableToSameSizeIdx.2 = phi i32 [ %spec.select72, %sw.default ], [ %SmallestLegalizableToSameSizeIdx.085, %for.body12 ], [ %SmallestLegalizableToSameSizeIdx.085, %sw.bb17 ], [ %SmallestLegalizableToSameSizeIdx.085, %sw.bb ]
  %LargestLegalizableToSameSizeIdx.1 = phi i32 [ %conv21, %sw.default ], [ %LargestLegalizableToSameSizeIdx.086, %for.body12 ], [ %LargestLegalizableToSameSizeIdx.086, %sw.bb17 ], [ %LargestLegalizableToSameSizeIdx.086, %sw.bb ]
  %inc = add nuw i64 %i.087, 1
  %exitcond = icmp eq i64 %inc, %umax
  br i1 %exitcond, label %for.cond.cleanup11, label %for.body12

for.cond.cleanup11:
  %SmallestNarrowIdx.1.lcssa = phi i32 [ %SmallestNarrowIdx.1, %for.inc24 ]
  %LargestWidenIdx.1.lcssa = phi i32 [ %LargestWidenIdx.1, %for.inc24 ]
  %SmallestLegalizableToSameSizeIdx.2.lcssa = phi i32 [ %SmallestLegalizableToSameSizeIdx.2, %for.inc24 ]
  %LargestLegalizableToSameSizeIdx.1.lcssa = phi i32 [ %LargestLegalizableToSameSizeIdx.1, %for.inc24 ]
  ret void
}
