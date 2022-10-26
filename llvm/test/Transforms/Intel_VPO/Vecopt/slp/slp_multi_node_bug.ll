; RUN: opt < %s -mtriple=i386 -slp-vectorizer -enable-intel-advanced-opts -slp-multinode -S -mcpu=core-avx2 -slp-threshold=-10 | FileCheck %s

; Test case exposing build of a broken MultiNode (MN).
; Vectorization is forced with lowering threshold in order to expose
; changes made to LLVM IR as MN operands reordering.

define void @test(i64* %ptr, i64 %add216567, i64 %add242566, i32 %i31) {
; CHECK-LABEL: @test(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[W:%.*]] = alloca [80 x i64], align 8
; CHECK-NEXT:    br i1 undef, label [[BB1:%.*]], label [[BB2:%.*]]
; CHECK:       bb1:
; CHECK-NEXT:    br i1 undef, label [[BB3:%.*]], label [[BB2]]
; CHECK:       bb2:
; CHECK-NEXT:    [[ADD190568:%.*]] = phi i64 [ 0, [[ENTRY:%.*]] ], [ undef, [[BB1]] ]
;
; CHECK:         [[ARRAYIDX75:%.*]] = getelementptr inbounds [80 x i64], [80 x i64]* [[W]], i32 0, i32 [[I31:%.*]]
; CHECK-NEXT:    [[I368:%.*]] = load i64, i64* [[ARRAYIDX75]], align 8
;
; CHECK-NOT:     %add76 = add i64 %add73, [[ADD190568]]
;
; Check that vectorization happens as the way to expose the bug.
; CHECK:  store <2 x i64>

entry:
  %W = alloca [80 x i64], align 8
  br i1 undef, label %bb1, label %bb2

bb1:                                              ; preds = %entry
  br i1 undef, label %bb3, label %bb2

bb2:                                              ; preds = %bb1, %entry
  %add190568 = phi i64 [ 0, %entry ], [ undef, %bb1 ]
  %add60 = add i64 undef, undef
  %xor.i508 = xor i64 undef, undef
  %shr.i6.i509 = lshr i64 undef, 41
  %shl.i7.i510 = shl i64 undef, 23
  %or.i8.i511 = or i64 %shl.i7.i510, %shr.i6.i509
  %xor3.i512 = xor i64 %xor.i508, %or.i8.i511
  %arrayidx75 = getelementptr inbounds [80 x i64], [80 x i64]* %W, i32 0, i32 %i31
  %i368 = load i64, i64* %arrayidx75, align 8
  %add73 = add i64 undef, %i368
  %shr.i.i488 = lshr i64 %add60, 28
  %shl.i.i489 = shl i64 %add60, 36
  %or.i.i490 = or i64 %shl.i.i489, %shr.i.i488
  %shr.i9.i491 = lshr i64 %add60, 34
  %shl.i10.i492 = shl i64 %add60, 30
  %or.i11.i493 = or i64 %shl.i10.i492, %shr.i9.i491
  %xor.i494 = xor i64 %or.i.i490, %or.i11.i493
  %shr.i6.i495 = lshr i64 %add60, 39
  %shl.i7.i496 = shl i64 %add60, 25
  %or.i8.i497 = or i64 %shl.i7.i496, %shr.i6.i495
  %xor3.i498 = xor i64 %xor.i494, %or.i8.i497
  %or.i484 = or i64 %add60, %add242566
  %and.i485 = and i64 %or.i484, %add216567
  %and1.i486 = and i64 %add60, %add242566
  %or2.i487 = or i64 %and.i485, %and1.i486

; Problem here is that MN built so that
; (1) %add76 was added as lane 0 MN operand and
; (2) both %add73 and %xor3.i512 was added as lane 1 MN operands.
; Thus instruction add76 turned out both operand and frontier in the same MN.
; Such MN configuration is illegal.
; Conseqences of that is MN reordering then swapped lane 1 operands: %xor3.i512, %add190568
; finally producing code with changed original semantics for other users of add76:
;  %add76 = add i64 %add73, %add190568
;  %add85 = add i64 %add76, %xor3.i512

  %add76 = add i64 %add73, %xor3.i512
  %add85 = add i64 %add76, %add190568
  %add83 = add i64 %xor3.i498, %or2.i487
  %add86 = add i64 %add83, %add76
  %gep0 = getelementptr i64, i64* %ptr, i32 0
  %gep1 = getelementptr i64, i64* %ptr, i32 1
  store i64 %add86, i64* %gep0, align 4
  store i64 %add85, i64* %gep1, align 4
  br label %bb3

bb3:                                              ; preds = %bb2, %bb1
  ret void
}
