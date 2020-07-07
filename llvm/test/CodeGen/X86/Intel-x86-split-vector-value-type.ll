; RUN: opt --mcpu=skylake-avx512 -S --x86-split-vector-value-type --verify < %s | FileCheck %s

; Tests for x86-split-vector-value-type pass.
; If an instruction has name, the split instructions will be name as [original inst name].(l|h)
; e.g. %x will be split into %x.l and %x.h.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CMPLRLLVM-19311: Avoid to split instructions chain if it contains supported vector value.
define <16 x i1> @overSplitTest(<16 x i32>* %x0_ptr, <16 x i32>* %y0_ptr, <16 x i64>* %x1_ptr, <16 x i64>* %y1_ptr) {
; CHECK-LABEL: overSplitTest
; CHECK:       %cmp0 = icmp sgt <16 x i32>
; CHECK-NEXT:  %cmp1 = icmp sgt <16 x i64>
entry:
  %x0 = load <16 x i32>, <16 x i32>* %x0_ptr
  %y0 = load <16 x i32>, <16 x i32>* %y0_ptr
  %x1 = load <16 x i64>, <16 x i64>* %x1_ptr
  %y1 = load <16 x i64>, <16 x i64>* %y1_ptr
  %cmp0 = icmp sgt <16 x i32> %x0, %y0
  %cmp1 = icmp sgt <16 x i64> %x1, %y1
  %res = and <16 x i1> %cmp0, %cmp1
  ret <16 x i1> %res
}

; CMPLRLLVM-18547: Test for ConstantExpr split fail bug.
@array0 = global [64 x i32] zeroinitializer
@array1 = global [64 x i32] zeroinitializer
define <16 x i32*> @constantExprSplitTest(i64 %val) {
; CHECK-LABEL: constantExprSplitTest
; CHECK:       %cmp0 = icmp eq <16 x i64>
; CHECK-NEXT:  %cmp1 = icmp sgt <16 x i64>
entry:
  %splatinsert = insertelement <16 x i64> undef, i64 %val, i32 0
  %splat = shufflevector <16 x i64> %splatinsert, <16 x i64> undef, <16 x i32> zeroinitializer
  %arrayIdx = getelementptr inbounds [64 x i32], <16 x [64 x i32]*> <[64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0, [64 x i32]* @array0>, <16 x i64> zeroinitializer, <16 x i64> %splat
  %cmp0 = icmp eq <16 x i64> %splat, <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
  %cmp1 = icmp sgt <16 x i64> %splat, zeroinitializer
  %cond = and <16 x i1> %cmp0, %cmp1
  %res = select <16 x i1> %cond, <16 x i32*> %arrayIdx, <16 x i32*> getelementptr ([64 x i32], <16 x [64 x i32]*> <[64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1, [64 x i32]* @array1>, <16 x i64> zeroinitializer, <16 x i64> <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>)
  ret <16 x i32*> %res
}

; CMPLRLLVM-19344: Test for split SelectInst when condition type is i1.
define <32 x i1> @selectSplitTest(<32 x i32>* %x0_ptr, <32 x i32>* %y0_ptr, <32 x i32>* %z0_ptr, i1 %cond) {
; CHECK-LABEL: selectSplitTest
; CHECK:       %sel.l = select i1 %cond, <16 x i32> %x0.l, <16 x i32> %y0.l
; CHECK-NEXT:  %sel.h = select i1 %cond, <16 x i32> %x0.h, <16 x i32> %y0.h
entry:
  %x0 = load <32 x i32>, <32 x i32>* %x0_ptr
  %y0 = load <32 x i32>, <32 x i32>* %y0_ptr
  %z0 = load <32 x i32>, <32 x i32>* %z0_ptr
  %sel = select i1 %cond, <32 x i32> %x0, <32 x i32> %y0
  %cmp0 = icmp sgt <32 x i32> %sel, %z0
  %cmp1 = icmp sgt <32 x i32> %x0, %y0
  %res = and <32 x i1> %cmp0, %cmp1
  ret <32 x i1> %res
}

; CMPLRLLVM-18912: The splitting of %x1.tr generates two shufflevector instructions and InstMap
; contains an element map %x1.tr to those shufflevector instructions. Then when updateInstChain
; method try to "update" %x1 to trunc instruction, a shufflvector instruction should be generated
; to "fuse" split instructions of %x1. This requires updateInstChain method should check if the
; instruction being updated is supported before checking InstMap. Otherwise "fuse" instruction
; won't be generated and this causes assertion fail in eraseInstSet method.
define <32 x i1> @unsupportedInstSplitTest(<32 x i64>* %x0_ptr) {
; CHECK-LABEL: unsupportedInstSplitTest
; CHECK:       %fused{{.*}} = shufflevector <16 x i64> %x1.l, <16 x i64> %x1.h, <32 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK-NEXT:  %x1.tr = trunc <32 x i64> %fused to <32 x i32>
; CHECK-NEXT:  %x1.tr.l = shufflevector <32 x i32> %x1.tr, <32 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:  %x1.tr.h = shufflevector <32 x i32> %x1.tr, <32 x i32> undef, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
entry:
  %x0 = load <32 x i64>, <32 x i64>* %x0_ptr
  %x1 = add <32 x i64> %x0, <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
  %x1.tr = trunc <32 x i64> %x1 to <32 x i32>
  %cmp0 = icmp sgt <32 x i32> %x1.tr, <i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0>
  %tmp0 = and <32 x i1> %cmp0, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %cmp1 = icmp sgt <32 x i64> %x1, <i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0, i64 0>
  %tmp1 = and <32 x i1> %cmp1, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %res = add <32 x i1> %tmp0, %cmp1
  ret <32 x i1> %res
}

; CMPLRLLVM-19385: Test if shufflevector to split value can be reused.
; %cmp0 %cmp4 %cmp5 can be split successfully. %cmp1 %cmp2 %cmp3 can't be split because %w0 is function parameter.
; The splitting of %cmp0 generate shufflevector instructions to split %x0 and %y0.
; It is expected those shufflevector instructions can be reused while split %cmp4 and %cmp5.
define i32 @CSETest(<32 x i32>* %x0_ptr, <32 x i32>* %y0_ptr, <32 x i32>* %z0_ptr, <32 x i32> %w0) {
; CHECK-LABEL: CSETest
; CHECK:       %x0 = load <32 x i32>, <32 x i32>* %x0_ptr
; CHECK-NEXT:  %x0.l = shufflevector <32 x i32> %x0, <32 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:  %x0.h = shufflevector <32 x i32> %x0, <32 x i32> undef, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK-NEXT:  %y0 = load <32 x i32>, <32 x i32>* %y0_ptr
; CHECK-NEXT:  %y0.l = shufflevector <32 x i32> %y0, <32 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:  %y0.h = shufflevector <32 x i32> %y0, <32 x i32> undef, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK-NEXT:  %z0 = load <32 x i32>, <32 x i32>* %z0_ptr
; CHECK-NEXT:  %z0.l = shufflevector <32 x i32> %z0, <32 x i32> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:  %z0.h = shufflevector <32 x i32> %z0, <32 x i32> undef, <16 x i32> <i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK-NEXT:  %cmp0.l = icmp sgt <16 x i32> %x0.l, %y0.l
entry:
  %x0 = load <32 x i32>, <32 x i32>* %x0_ptr
  %y0 = load <32 x i32>, <32 x i32>* %y0_ptr
  %z0 = load <32 x i32>, <32 x i32>* %z0_ptr
  %cmp0 = icmp sgt <32 x i32> %x0, %y0
  %cmp1 = icmp sgt <32 x i32> %x0, %w0
  %cmp2 = icmp sgt <32 x i32> %y0, %w0
  %cmp3 = icmp sgt <32 x i32> %z0, %w0
  %cmp4 = icmp sgt <32 x i32> %x0, %y0
  %cmp5 = icmp sgt <32 x i32> %x0, %z0
  %tmp0 = and <32 x i1> %cmp0, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %tmp1 = and <32 x i1> %cmp1, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %tmp2 = and <32 x i1> %cmp2, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %tmp3 = and <32 x i1> %cmp3, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %tmp4 = and <32 x i1> %cmp4, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %tmp5 = and <32 x i1> %cmp5, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  %tmp0.bc = bitcast <32 x i1> %tmp0 to i32
  %tmp1.bc = bitcast <32 x i1> %tmp1 to i32
  %tmp2.bc = bitcast <32 x i1> %tmp2 to i32
  %tmp3.bc = bitcast <32 x i1> %tmp3 to i32
  %tmp4.bc = bitcast <32 x i1> %tmp4 to i32
  %tmp5.bc = bitcast <32 x i1> %tmp5 to i32
  %sum0 = add i32 %tmp0.bc, %tmp1.bc
  %sum1 = add i32 %sum0, %tmp2.bc
  %sum2 = add i32 %sum1, %tmp3.bc
  %sum3 = add i32 %sum2, %tmp4.bc
  %sum4 = add i32 %sum3, %tmp5.bc
  ret i32 %sum4
}

define <32 x i1> @insertelementSplitTest(<32 x i32>* %data_ptr, i32 %val1, i32 %val2) {
; CHECK-LABEL:  insertelementSplitTest
; CHECK:        %x0.h = insertelement <16 x i32> %data.h, i32 %val1, i32 4
; CHECK-NEXT:   %y0.l = insertelement <16 x i32> undef, i32 %val2, i32 15
entry:
  %data = load <32 x i32>, <32 x i32>* %data_ptr
  %x0 = insertelement <32 x i32> %data, i32 %val1, i32 20
  %y0 = insertelement <32 x i32> undef, i32 %val2, i32 15
  %cmp0 = icmp sgt <32 x i32> %x0, %y0
  %cmp1 = icmp sgt <32 x i32> %x0, zeroinitializer
  %res = and <32 x i1> %cmp0, %cmp1
  ret <32 x i1> %res
}

define <32 x i1> @shufflevectorSplitTest(<32 x i32>* %x0_ptr, <32 x i32>* %y0_ptr) {
; CHECK-LABEL:  shufflevectorSplitTest
; CHECK:        %x1.h = shufflevector <16 x i32> %x0.h, <16 x i32> undef, <16 x i32> <i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4, i32 4>
; CHECK-NEXT:   %y1.l = shufflevector <16 x i32> %y0.l, <16 x i32> undef, <16 x i32> zeroinitializer
entry:
  %x0 = load <32 x i32>, <32 x i32>* %x0_ptr
  %y0 = load <32 x i32>, <32 x i32>* %y0_ptr
  %x1 = shufflevector <32 x i32> %x0, <32 x i32> undef, <32 x i32> <i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20, i32 20>
  %y1 = shufflevector <32 x i32> %y0, <32 x i32> undef, <32 x i32> zeroinitializer
  %cmp0 = icmp sgt <32 x i32> %x1, %y1
  %cmp1 = icmp sgt <32 x i32> %x1, zeroinitializer
  %res = and <32 x i1> %cmp0, %cmp1
  ret <32 x i1> %res
}

define void @phiSplitTest(<32 x i32>* %x0_ptr, <32 x i1>* %cond0_ptr) {
; CHECK-LABEL:  phiSplitTest
; CHECK:        bb1:
; CHECK-NEXT:   %x1.l = phi <16 x i32> [ %x0.l, %entry ], [ %x3.l, %bb3 ]
; CHECK-NEXT:   %x1.h = phi <16 x i32> [ %x0.h, %entry ], [ %x3.h, %bb3 ]
; CHECK:        bb2:
; CHECK-NEXT:   %x2.l = phi <16 x i32> [ %x1.l, %bb1 ]
; CHECK-NEXT:   %x2.h = phi <16 x i32> [ %x1.h, %bb1 ]
; CHECK:        bb3:
; CHECK-NEXT:   %x3.l = phi <16 x i32> [ %x2.l, %bb2 ]
; CHECK-NEXT:   %x3.h = phi <16 x i32> [ %x2.h, %bb2 ]
entry:
  %cond0 = load <32 x i1>, <32 x i1>* %cond0_ptr
  %x0 = load <32 x i32>, <32 x i32>* %x0_ptr
  br label %bb1

bb1:
  %x1 = phi <32 x i32> [ %x0, %entry ], [ %x3, %bb3 ]
  br label %bb2

bb2:
  %x2 = phi <32 x i32> [ %x1, %bb1 ]
  br label %bb3

bb3:
  %x3 = phi <32 x i32> [ %x2, %bb2 ]
  %cmp3 = icmp sgt <32 x i32> %x3, zeroinitializer
  %cond1 = and <32 x i1> %cmp3, %cond0
  %bc3 = bitcast <32 x i1> %cond1 to i32
  %zcmp3 = icmp eq i32 %bc3, 0
  br i1 %zcmp3, label %bb4, label %bb1

bb4:
  ret void
}

define i32 @foldFusedShufflevectorExtractElementTest(<32 x i32>* %x0_ptr, <32 x i32>* %y0_ptr) {
; CHECK-LABEL: foldFusedShufflevectorExtractElementTest
; CHECK:       %x1.h.extract.4{{.*}} = extractelement <16 x i32> %x1.h, i32 4
; CHECK-NEXT:  %y1.l.extract.15{{.*}} = extractelement <16 x i32> %y1.l, i32 15
entry:
  %x0 = load <32 x i32>, <32 x i32>* %x0_ptr
  %y0 = load <32 x i32>, <32 x i32>* %y0_ptr
  %x1 = insertelement <32 x i32> %x0, i32 1, i32 20
  %y1 = insertelement <32 x i32> %y0, i32 1, i32 15
  %x1.extract.20 = extractelement <32 x i32> %x1, i32 20
  %y1.extract.15 = extractelement <32 x i32> %y1, i32 15
  %cmp0 = icmp sgt <32 x i32> %x1, %y1
  %cmp1 = icmp sgt <32 x i32> %x1, zeroinitializer
  %cond0 = and <32 x i1> %cmp0, %cmp1
  %bc = bitcast <32 x i1> %cond0 to i32
  %sum0 = add i32 %x1.extract.20, %y1.extract.15
  %sum1 = add i32 %sum0, %bc
  ret i32 %sum1
}

define <32 x i1> @foldSplattedCmpShuffleVectorTest(<32 x i32>* %x_ptr) {
; CHECK-LABEL: foldSplattedCmpShuffleVectorTest
; CHECK:       %{{.+}} = shufflevector <16 x i32> %x.h, <16 x i32> undef, <16 x i32> <i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8, i32 8>
; CHECK-NEXT:  %z.h = icmp sgt <16 x i32> %{{.+}}, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
entry:
  %x = load <32 x i32>, <32 x i32>* %x_ptr
  %cmp = icmp sgt <32 x i32> %x, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %z = shufflevector <32 x i1> %cmp, <32 x i1> undef, <32 x i32> <i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24, i32 24>
  %res = and <32 x i1> %z, <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>
  ret <32 x i1> %res
}

; FIXME: This is a corner case where splitting of double promote ISel to
; generate mask instructions, but this causes too much register pressure.
define void @fooDouble(i32* noalias nocapture %result) {
; CHECK-LABEL: fooDouble
; CHECK:       phi <16 x i32>
; CHECK:       phi <8 x double>
; CHECK:       phi <16 x i32>
; CHECK:       phi <8 x i1>
; CHECK:       extractelement <16 x i32>{{.*}} i32 0
; CHECK:       fcmp fast olt <8 x double>
; CHECK:       and <8 x i1>
; CHECK:       insertelement <16 x i32>{{.*}} i32 0
; CHECK-NEXT:  shufflevector <16 x i32>{{.*}} <16 x i32> zeroinitializer
; CHECK-NEXT:  icmp sgt <16 x i32>
; CHECK:       select <16 x i1>{{.*}}<16 x i32>
; CHECK:       and <16 x i1>
; CHECK:       select <16 x i1>{{.*}}<16 x i32>
; CHECK:       bitcast <32 x i1>

omp.inner.for.body.lr.ph:
  br label %hir.L.1

hir.L.1:                                          ; preds = %then.38, %omp.inner.for.body.lr.ph
  %t3.0 = phi <32 x i32> [ undef, %omp.inner.for.body.lr.ph ], [ %22, %then.38 ]
  %t6.0 = phi <32 x double> [ <double 1.000000e+00, double 2.000000e+00, double 3.000000e+00, double 4.000000e+00, double 5.000000e+00, double 6.000000e+00, double 7.000000e+00, double 8.000000e+00, double 9.000000e+00, double 1.000000e+01, double 1.100000e+01, double 1.200000e+01, double 1.300000e+01, double 1.400000e+01, double 1.500000e+01, double 1.600000e+01, double 1.700000e+01, double 1.800000e+01, double 1.900000e+01, double 2.000000e+01, double 2.100000e+01, double 2.200000e+01, double 2.300000e+01, double 2.400000e+01, double 2.500000e+01, double 2.600000e+01, double 2.700000e+01, double 2.800000e+01, double 2.900000e+01, double 3.000000e+01, double 3.100000e+01, double 3.200000e+01>, %omp.inner.for.body.lr.ph ], [ %18, %then.38 ]
  %t5.0 = phi <32 x double> [ zeroinitializer, %omp.inner.for.body.lr.ph ], [ %25, %then.38 ]
  %t4.0 = phi <32 x i32> [ <i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512>, %omp.inner.for.body.lr.ph ], [ %broadcast.splat91, %then.38 ]
  %t7.0 = phi <32 x i1> [ <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %omp.inner.for.body.lr.ph ], [ %21, %then.38 ]
  %vec.phi31.extract.0.45 = extractelement <32 x i32> %t4.0, i32 0
  %0 = fadd fast <32 x double> %t5.0, %t6.0
  %hir.cmp.72 = fcmp fast olt <32 x double> %0, <double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01>
  %1 = and <32 x i1> %t7.0, %hir.cmp.72
  %2 = fmul fast <32 x double> %t6.0, %t6.0
  %3 = fadd fast <32 x double> %2, %t5.0
  %4 = fmul fast <32 x double> %t5.0, %t5.0
  %5 = fsub fast <32 x double> %4, %3
  %6 = add i32 %vec.phi31.extract.0.45, -1
  %broadcast.splatinsert53 = insertelement <32 x i32> undef, i32 %6, i32 0
  %broadcast.splat54 = shufflevector <32 x i32> %broadcast.splatinsert53, <32 x i32> undef, <32 x i32> zeroinitializer
  %hir.cmp.82 = icmp sgt <32 x i32> %t4.0, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %7 = select <32 x i1> %1, <32 x i32> %broadcast.splat54, <32 x i32> %t4.0
  %8 = and <32 x i1> %1, %hir.cmp.82
  %9 = select <32 x i1> %t7.0, <32 x i32> %7, <32 x i32> %t3.0
  %10 = bitcast <32 x i1> %8 to i32
  %hir.cmp.92 = icmp eq i32 %10, 0
  br i1 %hir.cmp.92, label %hir.L.68, label %ifmerge.101

hir.L.68:                                         ; preds = %ifmerge.101, %hir.L.1
  %t3.1 = phi <32 x i32> [ %9, %hir.L.1 ], [ %22, %ifmerge.101 ]
  %11 = icmp sgt <32 x i32> %t3.1, zeroinitializer
  %12 = sub nsw <32 x i32> <i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512>, %t3.1
  %13 = select <32 x i1> %11, <32 x i32> %12, <32 x i32> zeroinitializer
  %14 = bitcast i32* %result to <32 x i32>*
  %wide.load = load <32 x i32>, <32 x i32>* %14, align 4
  %15 = add nsw <32 x i32> %wide.load, %13
  store <32 x i32> %15, <32 x i32>* %14, align 4
  ret void

ifmerge.101:                                      ; preds = %hir.L.1
  %hir.cmp.8 = fcmp fast olt <32 x double> %4, <double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01, double 3.300000e+01>
  %16 = and <32 x i1> %8, %hir.cmp.8
  %17 = fmul fast <32 x double> %3, %3
  %18 = fadd fast <32 x double> %17, %5
  %19 = add i32 %vec.phi31.extract.0.45, -2
  %broadcast.splatinsert89 = insertelement <32 x i32> undef, i32 %19, i32 0
  %broadcast.splat91 = shufflevector <32 x i32> %broadcast.splatinsert89, <32 x i32> undef, <32 x i32> zeroinitializer
  %hir.cmp.19 = icmp sgt <32 x i32> %broadcast.splat54, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %20 = select <32 x i1> %16, <32 x i32> %broadcast.splat91, <32 x i32> %broadcast.splat54
  %21 = and <32 x i1> %16, %hir.cmp.19
  %22 = select <32 x i1> %8, <32 x i32> %20, <32 x i32> %9
  %23 = bitcast <32 x i1> %21 to i32
  %hir.cmp.29 = icmp eq i32 %23, 0
  br i1 %hir.cmp.29, label %hir.L.68, label %then.38

then.38:                                          ; preds = %ifmerge.101
  %24 = fmul fast <32 x double> %5, %5
  %25 = fsub fast <32 x double> %24, %18
  br label %hir.L.1
}

define void @fooFloat(i32* noalias nocapture %result) {
; CHECK-LABEL: fooFloat
; CHECK:       phi <16 x i32>
; CHECK:       phi <16 x float>
; CHECK:       phi <16 x i32>
; CHECK:       phi <16 x i1>
; CHECK:       extractelement <16 x i32>{{.*}} i32 0
; CHECK:       fcmp fast olt <16 x float>
; CHECK:       and <16 x i1>
; CHECK:       insertelement <16 x i32>{{.*}} i32 0
; CHECK-NEXT:  shufflevector <16 x i32>{{.*}} <16 x i32> zeroinitializer
; CHECK-NEXT:  icmp sgt <16 x i32>
; CHECK:       select <16 x i1>{{.*}}<16 x i32>
; CHECK:       and <16 x i1>
; CHECK:       select <16 x i1>{{.*}}<16 x i32>
; CHECK:       bitcast <32 x i1>

omp.inner.for.body.lr.ph:
  br label %hir.L.1

hir.L.1:                                          ; preds = %then.38, %omp.inner.for.body.lr.ph
  %t3.0 = phi <32 x i32> [ undef, %omp.inner.for.body.lr.ph ], [ %22, %then.38 ]
  %t6.0 = phi <32 x float> [ <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00, float 5.000000e+00, float 6.000000e+00, float 7.000000e+00, float 8.000000e+00, float 9.000000e+00, float 1.000000e+01, float 1.100000e+01, float 1.200000e+01, float 1.300000e+01, float 1.400000e+01, float 1.500000e+01, float 1.600000e+01, float 1.700000e+01, float 1.800000e+01, float 1.900000e+01, float 2.000000e+01, float 2.100000e+01, float 2.200000e+01, float 2.300000e+01, float 2.400000e+01, float 2.500000e+01, float 2.600000e+01, float 2.700000e+01, float 2.800000e+01, float 2.900000e+01, float 3.000000e+01, float 3.100000e+01, float 3.200000e+01>, %omp.inner.for.body.lr.ph ], [ %18, %then.38 ]
  %t5.0 = phi <32 x float> [ zeroinitializer, %omp.inner.for.body.lr.ph ], [ %25, %then.38 ]
  %t4.0 = phi <32 x i32> [ <i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512>, %omp.inner.for.body.lr.ph ], [ %broadcast.splat93, %then.38 ]
  %t7.0 = phi <32 x i1> [ <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %omp.inner.for.body.lr.ph ], [ %21, %then.38 ]
  %vec.phi33.extract.0.47 = extractelement <32 x i32> %t4.0, i32 0
  %0 = fadd fast <32 x float> %t5.0, %t6.0
  %hir.cmp.72 = fcmp fast olt <32 x float> %0, <float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01>
  %1 = and <32 x i1> %t7.0, %hir.cmp.72
  %2 = fmul fast <32 x float> %t6.0, %t6.0
  %3 = fadd fast <32 x float> %2, %t5.0
  %4 = fmul fast <32 x float> %t5.0, %t5.0
  %5 = fsub fast <32 x float> %4, %3
  %6 = add i32 %vec.phi33.extract.0.47, -1
  %broadcast.splatinsert55 = insertelement <32 x i32> undef, i32 %6, i32 0
  %broadcast.splat56 = shufflevector <32 x i32> %broadcast.splatinsert55, <32 x i32> undef, <32 x i32> zeroinitializer
  %hir.cmp.82 = icmp sgt <32 x i32> %t4.0, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %7 = select <32 x i1> %1, <32 x i32> %broadcast.splat56, <32 x i32> %t4.0
  %8 = and <32 x i1> %1, %hir.cmp.82
  %9 = select <32 x i1> %t7.0, <32 x i32> %7, <32 x i32> %t3.0
  %10 = bitcast <32 x i1> %8 to i32
  %hir.cmp.92 = icmp eq i32 %10, 0
  br i1 %hir.cmp.92, label %hir.L.68, label %ifmerge.101

hir.L.68:                                         ; preds = %ifmerge.101, %hir.L.1
  %t3.1 = phi <32 x i32> [ %9, %hir.L.1 ], [ %22, %ifmerge.101 ]
  %11 = icmp sgt <32 x i32> %t3.1, zeroinitializer
  %12 = sub nsw <32 x i32> <i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512, i32 512>, %t3.1
  %13 = select <32 x i1> %11, <32 x i32> %12, <32 x i32> zeroinitializer
  %14 = bitcast i32* %result to <32 x i32>*
  %wide.load = load <32 x i32>, <32 x i32>* %14, align 4
  %15 = add nsw <32 x i32> %wide.load, %13
  store <32 x i32> %15, <32 x i32>* %14, align 4
  ret void

ifmerge.101:                                      ; preds = %hir.L.1
  %hir.cmp.8 = fcmp fast olt <32 x float> %4, <float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01, float 3.300000e+01>
  %16 = and <32 x i1> %8, %hir.cmp.8
  %17 = fmul fast <32 x float> %3, %3
  %18 = fadd fast <32 x float> %17, %5
  %19 = add i32 %vec.phi33.extract.0.47, -2
  %broadcast.splatinsert91 = insertelement <32 x i32> undef, i32 %19, i32 0
  %broadcast.splat93 = shufflevector <32 x i32> %broadcast.splatinsert91, <32 x i32> undef, <32 x i32> zeroinitializer
  %hir.cmp.19 = icmp sgt <32 x i32> %broadcast.splat56, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %20 = select <32 x i1> %16, <32 x i32> %broadcast.splat93, <32 x i32> %broadcast.splat56
  %21 = and <32 x i1> %16, %hir.cmp.19
  %22 = select <32 x i1> %8, <32 x i32> %20, <32 x i32> %9
  %23 = bitcast <32 x i1> %21 to i32
  %hir.cmp.29 = icmp eq i32 %23, 0
  br i1 %hir.cmp.29, label %hir.L.68, label %then.38

then.38:                                          ; preds = %ifmerge.101
  %24 = fmul fast <32 x float> %5, %5
  %25 = fsub fast <32 x float> %24, %18
  br label %hir.L.1
}

; A test case similar to mandelbrot-dcg-omp, but this is "double" version.
; This case is used to test "over split" issue.
define <32 x i32> @fooDouble2(i32* %m_ptr, <32 x double>* %broadcast.splat77_ptr, <32 x double>* %elmt_ptr) {
; CHECK-LABEL: fooDouble
; CHECK:       phi <32 x i32>
; CHECK:       phi <16 x double>
; CHECK:       phi i32
; CHECK:       phi <16 x double>
; CHECK:       phi <32 x i1>
; CHECK:       insertelement <16 x i32>{{.*}} i32 0
; CHECK:       add i32{{.*}} -1
; CHECK-NEXT:  insertelement <32 x i32>{{.*}} i32 0
; CHECK-NEXT:  shufflevector <32 x i32>{{.*}} <32 x i32> zeroinitializer
; CHECK-NEXT:  shufflevector <16 x i32>{{.*}} <16 x i32> zeroinitializer
; CHECK-NEXT:  icmp sgt <16 x i32>
; CHECK:       fcmp fast olt <16 x double>
; CHECK:       and <16 x i1>
; CHECK:       and <32 x i1>
; CHECK:       select <32 x i1>
; CHECK:       bitcast <32 x i1>

entry:
  %m = load i32, i32* %m_ptr
  %broadcast.splat77 = load <32 x double>, <32 x double>* %broadcast.splat77_ptr
  %elmt = load <32 x double>, <32 x double> *%elmt_ptr
  br label %VPlannedBB68

VPlannedBB68:
  %vec.phi69 = phi <32 x i32> [ %tmp14, %VPlannedBB68 ], [ undef, %entry ]
  %vec.phi70 = phi <32 x double> [ %tmp5, %VPlannedBB68 ], [ zeroinitializer, %entry ]
  %vec.phi71 = phi <32 x double> [ %tmp6, %VPlannedBB68 ], [ zeroinitializer, %entry ]
  %vec.phi72 = phi <32 x double> [ %tmp2, %VPlannedBB68 ], [ zeroinitializer, %entry ]
  %uni.phi73 = phi i32 [ %tmp7, %VPlannedBB68 ], [ %m, %entry ]
  %vec.phi74 = phi <32 x double> [ %tmp4, %VPlannedBB68 ], [ zeroinitializer, %entry ]
  %vec.phi75 = phi <32 x i1> [ %tmp13, %VPlannedBB68 ], [ <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %entry ]
  %broadcast.splatinsert78 = insertelement <32 x i32> undef, i32 %uni.phi73, i32 0
  %tmp0 = fmul fast <32 x double> %vec.phi74, <double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00, double 2.000000e+00>
  %tmp1 = fmul fast <32 x double> %tmp0, %vec.phi72
  %tmp2 = fadd fast <32 x double> %broadcast.splat77, %tmp1
  %tmp3 = fsub fast <32 x double> %vec.phi71, %vec.phi70
  %tmp4 = fadd fast <32 x double> %elmt, %tmp3
  %tmp5 = fmul fast <32 x double> %tmp2, %tmp2
  %tmp6 = fmul fast <32 x double> %tmp4, %tmp4
  %tmp7 = add i32 %uni.phi73, -1
  %broadcast.splatinsert80 = insertelement <32 x i32> undef, i32 %tmp7, i32 0
  %broadcast.splat81 = shufflevector <32 x i32> %broadcast.splatinsert80, <32 x i32> undef, <32 x i32> zeroinitializer
  %tmp8 = icmp sgt <32 x i32> %broadcast.splatinsert78, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %tmp9 = shufflevector <32 x i1> %tmp8, <32 x i1> undef, <32 x i32> zeroinitializer
  %tmp10 = fadd fast <32 x double> %tmp5, %tmp6
  %tmp11 = fcmp fast olt <32 x double> %tmp10, <double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00, double 4.000000e+00>
  %tmp12 = and <32 x i1> %tmp9, %tmp11
  %tmp13 = and <32 x i1> %tmp12, %vec.phi75
  %tmp14 = select <32 x i1> %vec.phi75, <32 x i32> %broadcast.splat81, <32 x i32> %vec.phi69
  %tmp15 = bitcast <32 x i1> %tmp13 to i32
  %tmp16 = icmp eq i32 %tmp15, 0
  br i1 %tmp16, label %end, label %VPlannedBB68

end:
  ret <32 x i32> %tmp14
}

; A test case similar to mandelbrot-dcg-omp, but this is 64-way version (quad-pumping).
define <64 x i32> @fooFloat2(i32* %m_ptr, <64 x float>* %broadcast.splat77_ptr, <64 x float>* %elmt_ptr) {
; CHECK-LABEL: fooFloat2
; CHECK:       phi <64 x i32>
; CHECK:       phi <16 x float>
; CHECK:       phi i32
; CHECK:       phi <16 x float>
; CHECK:       phi <64 x i1>
; CHECK:       insertelement <16 x i32>{{.*}} i32 0
; CHECK:       add i32{{.*}} -1
; CHECK-NEXT:  insertelement <64 x i32>{{.*}} i32 0
; CHECK-NEXT:  shufflevector <64 x i32>{{.*}} <64 x i32> zeroinitializer
; CHECK-NEXT:  shufflevector <16 x i32>{{.*}} <16 x i32> zeroinitializer
; CHECK-NEXT:  icmp sgt <16 x i32>
; CHECK:       fcmp fast olt <16 x float>
; CHECK:       and <16 x i1>
; CHECK:       and <64 x i1>
; CHECK:       select <64 x i1>
; CHECK:       bitcast <64 x i1>

entry:
  %m = load i32, i32* %m_ptr
  %broadcast.splat77 = load <64 x float>, <64 x float>* %broadcast.splat77_ptr
  %elmt = load <64 x float>, <64 x float> *%elmt_ptr
  br label %VPlannedBB68

VPlannedBB68:
  %vec.phi69 = phi <64 x i32> [ %tmp14, %VPlannedBB68 ], [ undef, %entry ]
  %vec.phi70 = phi <64 x float> [ %tmp5, %VPlannedBB68 ], [ zeroinitializer, %entry ]
  %vec.phi71 = phi <64 x float> [ %tmp6, %VPlannedBB68 ], [ zeroinitializer, %entry ]
  %vec.phi72 = phi <64 x float> [ %tmp2, %VPlannedBB68 ], [ zeroinitializer, %entry ]
  %uni.phi73 = phi i32 [ %tmp7, %VPlannedBB68 ], [ %m, %entry ]
  %vec.phi74 = phi <64 x float> [ %tmp4, %VPlannedBB68 ], [ zeroinitializer, %entry ]
  %vec.phi75 = phi <64 x i1> [ %tmp13, %VPlannedBB68 ], [ <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, %entry ]
  %broadcast.splatinsert78 = insertelement <64 x i32> undef, i32 %uni.phi73, i32 0
  %tmp0 = fmul fast <64 x float> %vec.phi74, <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
  %tmp1 = fmul fast <64 x float> %tmp0, %vec.phi72
  %tmp2 = fadd fast <64 x float> %broadcast.splat77, %tmp1
  %tmp3 = fsub fast <64 x float> %vec.phi71, %vec.phi70
  %tmp4 = fadd fast <64 x float> %elmt, %tmp3
  %tmp5 = fmul fast <64 x float> %tmp2, %tmp2
  %tmp6 = fmul fast <64 x float> %tmp4, %tmp4
  %tmp7 = add i32 %uni.phi73, -1
  %broadcast.splatinsert80 = insertelement <64 x i32> undef, i32 %tmp7, i32 0
  %broadcast.splat81 = shufflevector <64 x i32> %broadcast.splatinsert80, <64 x i32> undef, <64 x i32> zeroinitializer
  %tmp8 = icmp sgt <64 x i32> %broadcast.splatinsert78, <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %tmp9 = shufflevector <64 x i1> %tmp8, <64 x i1> undef, <64 x i32> zeroinitializer
  %tmp10 = fadd fast <64 x float> %tmp5, %tmp6
  %tmp11 = fcmp fast olt <64 x float> %tmp10, <float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00, float 4.000000e+00>
  %tmp12 = and <64 x i1> %tmp9, %tmp11
  %tmp13 = and <64 x i1> %tmp12, %vec.phi75
  %tmp14 = select <64 x i1> %vec.phi75, <64 x i32> %broadcast.splat81, <64 x i32> %vec.phi69
  %tmp15 = bitcast <64 x i1> %tmp13 to i64
  %tmp16 = icmp eq i64 %tmp15, 0
  br i1 %tmp16, label %end, label %VPlannedBB68

end:
  ret <64 x i32> %tmp14
}
