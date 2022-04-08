; REQUIRES: asserts
; RUN: opt < %s -loop-reduce -debug-only=loop-reduce -S 2>&1 | FileCheck %s
;
; Test to check LSR is not applied when LSR Cost is exceeding X86TTI registers

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: LSR: Solution's LSR cost is exceeding Target supports, abandoned the solution.

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @foo(i64 %N, i64 %instride, i64 %outstride, float* nocapture readonly %in, float* nocapture writeonly %out) local_unnamed_addr #0 {
entry:
  %mul = shl nsw i64 %instride, 1
  %add = mul nsw i64 %instride, 3
  %mul1 = shl nsw i64 %instride, 2
  %mul2 = shl nsw i64 %outstride, 3
  %mul3 = shl nsw i64 %outstride, 4
  %add4 = mul nsw i64 %outstride, 24
  %cmp601 = icmp sgt i64 %N, 0
  br i1 %cmp601, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %entry, %for.body
  %in.addr.0604 = phi float* [ %add.ptr161, %for.body ], [ %in, %entry ]
  %out.addr.0603 = phi float* [ %add.ptr162, %for.body ], [ %out, %entry ]
  %i.0602 = phi i64 [ %add163, %for.body ], [ 0, %entry ]
  %__v.i = bitcast float* %in.addr.0604 to <16 x float>*
  %0 = load <16 x float>, <16 x float>* %__v.i, align 1, !tbaa !3
  %add.ptr = getelementptr inbounds float, float* %in.addr.0604, i64 %instride, !intel-tbaa !6
  %__v.i600 = bitcast float* %add.ptr to <16 x float>*
  %1 = load <16 x float>, <16 x float>* %__v.i600, align 1, !tbaa !3
  %add.ptr6 = getelementptr inbounds float, float* %in.addr.0604, i64 %mul, !intel-tbaa !6
  %__v.i599 = bitcast float* %add.ptr6 to <16 x float>*
  %2 = load <16 x float>, <16 x float>* %__v.i599, align 1, !tbaa !3
  %add.ptr8 = getelementptr inbounds float, float* %in.addr.0604, i64 %add, !intel-tbaa !6
  %__v.i598 = bitcast float* %add.ptr8 to <16 x float>*
  %3 = load <16 x float>, <16 x float>* %__v.i598, align 1, !tbaa !3
  %add.ptr10 = getelementptr inbounds float, float* %in.addr.0604, i64 %mul1, !intel-tbaa !6
  %__v.i597 = bitcast float* %add.ptr10 to <16 x float>*
  %4 = load <16 x float>, <16 x float>* %__v.i597, align 1, !tbaa !3
  %add.ptr12 = getelementptr inbounds float, float* %add.ptr10, i64 %instride, !intel-tbaa !6
  %__v.i596 = bitcast float* %add.ptr12 to <16 x float>*
  %5 = load <16 x float>, <16 x float>* %__v.i596, align 1, !tbaa !3
  %add.ptr14 = getelementptr inbounds float, float* %add.ptr10, i64 %mul, !intel-tbaa !6
  %__v.i595 = bitcast float* %add.ptr14 to <16 x float>*
  %6 = load <16 x float>, <16 x float>* %__v.i595, align 1, !tbaa !3
  %add.ptr16 = getelementptr inbounds float, float* %add.ptr10, i64 %add, !intel-tbaa !6
  %__v.i594 = bitcast float* %add.ptr16 to <16 x float>*
  %7 = load <16 x float>, <16 x float>* %__v.i594, align 1, !tbaa !3
  %add.ptr18 = getelementptr inbounds float, float* %add.ptr10, i64 %mul1, !intel-tbaa !6
  %__v.i593 = bitcast float* %add.ptr18 to <16 x float>*
  %8 = load <16 x float>, <16 x float>* %__v.i593, align 1, !tbaa !3
  %add.ptr20 = getelementptr inbounds float, float* %add.ptr18, i64 %instride, !intel-tbaa !6
  %__v.i592 = bitcast float* %add.ptr20 to <16 x float>*
  %9 = load <16 x float>, <16 x float>* %__v.i592, align 1, !tbaa !3
  %add.ptr22 = getelementptr inbounds float, float* %add.ptr18, i64 %mul, !intel-tbaa !6
  %__v.i591 = bitcast float* %add.ptr22 to <16 x float>*
  %10 = load <16 x float>, <16 x float>* %__v.i591, align 1, !tbaa !3
  %add.ptr24 = getelementptr inbounds float, float* %add.ptr18, i64 %add, !intel-tbaa !6
  %__v.i590 = bitcast float* %add.ptr24 to <16 x float>*
  %11 = load <16 x float>, <16 x float>* %__v.i590, align 1, !tbaa !3
  %add.ptr26 = getelementptr inbounds float, float* %add.ptr18, i64 %mul1, !intel-tbaa !6
  %__v.i589 = bitcast float* %add.ptr26 to <16 x float>*
  %12 = load <16 x float>, <16 x float>* %__v.i589, align 1, !tbaa !3
  %add.ptr28 = getelementptr inbounds float, float* %add.ptr26, i64 %instride, !intel-tbaa !6
  %__v.i588 = bitcast float* %add.ptr28 to <16 x float>*
  %13 = load <16 x float>, <16 x float>* %__v.i588, align 1, !tbaa !3
  %add.ptr30 = getelementptr inbounds float, float* %add.ptr26, i64 %mul, !intel-tbaa !6
  %__v.i587 = bitcast float* %add.ptr30 to <16 x float>*
  %14 = load <16 x float>, <16 x float>* %__v.i587, align 1, !tbaa !3
  %add.ptr32 = getelementptr inbounds float, float* %add.ptr26, i64 %add, !intel-tbaa !6
  %__v.i586 = bitcast float* %add.ptr32 to <16 x float>*
  %15 = load <16 x float>, <16 x float>* %__v.i586, align 1, !tbaa !3
  %add.ptr34 = getelementptr inbounds float, float* %add.ptr26, i64 %mul1, !intel-tbaa !6
  %__v.i585 = bitcast float* %add.ptr34 to <16 x float>*
  %16 = load <16 x float>, <16 x float>* %__v.i585, align 1, !tbaa !3
  %add.ptr36 = getelementptr inbounds float, float* %add.ptr34, i64 %instride, !intel-tbaa !6
  %__v.i584 = bitcast float* %add.ptr36 to <16 x float>*
  %17 = load <16 x float>, <16 x float>* %__v.i584, align 1, !tbaa !3
  %add.ptr38 = getelementptr inbounds float, float* %add.ptr34, i64 %mul, !intel-tbaa !6
  %__v.i583 = bitcast float* %add.ptr38 to <16 x float>*
  %18 = load <16 x float>, <16 x float>* %__v.i583, align 1, !tbaa !3
  %add.ptr40 = getelementptr inbounds float, float* %add.ptr34, i64 %add, !intel-tbaa !6
  %__v.i582 = bitcast float* %add.ptr40 to <16 x float>*
  %19 = load <16 x float>, <16 x float>* %__v.i582, align 1, !tbaa !3
  %add.ptr42 = getelementptr inbounds float, float* %add.ptr34, i64 %mul1, !intel-tbaa !6
  %__v.i581 = bitcast float* %add.ptr42 to <16 x float>*
  %20 = load <16 x float>, <16 x float>* %__v.i581, align 1, !tbaa !3
  %add.ptr44 = getelementptr inbounds float, float* %add.ptr42, i64 %instride, !intel-tbaa !6
  %__v.i580 = bitcast float* %add.ptr44 to <16 x float>*
  %21 = load <16 x float>, <16 x float>* %__v.i580, align 1, !tbaa !3
  %add.ptr46 = getelementptr inbounds float, float* %add.ptr42, i64 %mul, !intel-tbaa !6
  %__v.i579 = bitcast float* %add.ptr46 to <16 x float>*
  %22 = load <16 x float>, <16 x float>* %__v.i579, align 1, !tbaa !3
  %add.ptr48 = getelementptr inbounds float, float* %add.ptr42, i64 %add, !intel-tbaa !6
  %__v.i578 = bitcast float* %add.ptr48 to <16 x float>*
  %23 = load <16 x float>, <16 x float>* %__v.i578, align 1, !tbaa !3
  %add.ptr50 = getelementptr inbounds float, float* %add.ptr42, i64 %mul1, !intel-tbaa !6
  %__v.i577 = bitcast float* %add.ptr50 to <16 x float>*
  %24 = load <16 x float>, <16 x float>* %__v.i577, align 1, !tbaa !3
  %add.ptr52 = getelementptr inbounds float, float* %add.ptr50, i64 %instride, !intel-tbaa !6
  %__v.i576 = bitcast float* %add.ptr52 to <16 x float>*
  %25 = load <16 x float>, <16 x float>* %__v.i576, align 1, !tbaa !3
  %add.ptr54 = getelementptr inbounds float, float* %add.ptr50, i64 %mul, !intel-tbaa !6
  %__v.i575 = bitcast float* %add.ptr54 to <16 x float>*
  %26 = load <16 x float>, <16 x float>* %__v.i575, align 1, !tbaa !3
  %add.ptr56 = getelementptr inbounds float, float* %add.ptr50, i64 %add, !intel-tbaa !6
  %__v.i574 = bitcast float* %add.ptr56 to <16 x float>*
  %27 = load <16 x float>, <16 x float>* %__v.i574, align 1, !tbaa !3
  %add.ptr58 = getelementptr inbounds float, float* %add.ptr50, i64 %mul1, !intel-tbaa !6
  %__v.i573 = bitcast float* %add.ptr58 to <16 x float>*
  %28 = load <16 x float>, <16 x float>* %__v.i573, align 1, !tbaa !3
  %add.ptr60 = getelementptr inbounds float, float* %add.ptr58, i64 %instride, !intel-tbaa !6
  %__v.i572 = bitcast float* %add.ptr60 to <16 x float>*
  %29 = load <16 x float>, <16 x float>* %__v.i572, align 1, !tbaa !3
  %add.ptr62 = getelementptr inbounds float, float* %add.ptr58, i64 %mul, !intel-tbaa !6
  %__v.i571 = bitcast float* %add.ptr62 to <16 x float>*
  %30 = load <16 x float>, <16 x float>* %__v.i571, align 1, !tbaa !3
  %add.ptr64 = getelementptr inbounds float, float* %add.ptr58, i64 %add, !intel-tbaa !6
  %__v.i570 = bitcast float* %add.ptr64 to <16 x float>*
  %31 = load <16 x float>, <16 x float>* %__v.i570, align 1, !tbaa !3
  %add.i569 = fadd <16 x float> %0, %4
  %add.i568 = fadd <16 x float> %1, %5
  %add.i567 = fadd <16 x float> %2, %6
  %add.i566 = fadd <16 x float> %3, %7
  %add.i565 = fadd <16 x float> %add.i569, %8
  %add.i564 = fadd <16 x float> %add.i568, %9
  %add.i563 = fadd <16 x float> %add.i567, %10
  %add.i562 = fadd <16 x float> %add.i566, %11
  %add.i561 = fadd <16 x float> %add.i565, %12
  %add.i560 = fadd <16 x float> %add.i564, %13
  %add.i559 = fadd <16 x float> %add.i563, %14
  %add.i558 = fadd <16 x float> %add.i562, %15
  %add.i557 = fadd <16 x float> %add.i561, %16
  %add.i556 = fadd <16 x float> %add.i560, %17
  %add.i555 = fadd <16 x float> %add.i559, %18
  %add.i554 = fadd <16 x float> %add.i558, %19
  %add.i553 = fadd <16 x float> %add.i557, %20
  %add.i552 = fadd <16 x float> %add.i556, %21
  %add.i551 = fadd <16 x float> %add.i555, %22
  %add.i550 = fadd <16 x float> %add.i554, %23
  %add.i549 = fadd <16 x float> %add.i553, %24
  %add.i548 = fadd <16 x float> %add.i552, %25
  %add.i547 = fadd <16 x float> %add.i551, %26
  %add.i546 = fadd <16 x float> %add.i550, %27
  %add.i545 = fadd <16 x float> %add.i549, %28
  %add.i544 = fadd <16 x float> %add.i548, %29
  %add.i543 = fadd <16 x float> %add.i547, %30
  %add.i542 = fadd <16 x float> %add.i546, %31
  %add.i541 = fadd <16 x float> %0, %add.i545
  %add.i540 = fadd <16 x float> %1, %add.i544
  %add.i539 = fadd <16 x float> %2, %add.i543
  %add.i538 = fadd <16 x float> %3, %add.i542
  %add.i537 = fadd <16 x float> %add.i557, %add.i541
  %add.i536 = fadd <16 x float> %add.i565, %add.i549
  %add.i535 = fadd <16 x float> %add.i557, %add.i537
  %add.i534 = fadd <16 x float> %add.i549, %add.i536
  %__v.i533 = bitcast float* %out.addr.0603 to <16 x float>*
  store <16 x float> %add.i537, <16 x float>* %__v.i533, align 1, !tbaa !3
  %add.ptr102 = getelementptr inbounds float, float* %out.addr.0603, i64 %mul2, !intel-tbaa !6
  %__v.i532 = bitcast float* %add.ptr102 to <16 x float>*
  store <16 x float> %add.i536, <16 x float>* %__v.i532, align 1, !tbaa !3
  %add.ptr103 = getelementptr inbounds float, float* %out.addr.0603, i64 %mul3, !intel-tbaa !6
  %__v.i531 = bitcast float* %add.ptr103 to <16 x float>*
  store <16 x float> %add.i535, <16 x float>* %__v.i531, align 1, !tbaa !3
  %add.ptr104 = getelementptr inbounds float, float* %out.addr.0603, i64 %add4, !intel-tbaa !6
  %__v.i530 = bitcast float* %add.ptr104 to <16 x float>*
  store <16 x float> %add.i534, <16 x float>* %__v.i530, align 1, !tbaa !3
  %add.i529 = fadd <16 x float> %add.i556, %add.i540
  %add.i528 = fadd <16 x float> %add.i564, %add.i548
  %add.i527 = fadd <16 x float> %add.i556, %add.i529
  %add.i526 = fadd <16 x float> %add.i548, %add.i528
  %add.ptr109 = getelementptr inbounds float, float* %out.addr.0603, i64 %outstride, !intel-tbaa !6
  %__v.i525 = bitcast float* %add.ptr109 to <16 x float>*
  store <16 x float> %add.i529, <16 x float>* %__v.i525, align 1, !tbaa !3
  %add.ptr110 = getelementptr inbounds float, float* %add.ptr109, i64 %mul2, !intel-tbaa !6
  %__v.i524 = bitcast float* %add.ptr110 to <16 x float>*
  store <16 x float> %add.i528, <16 x float>* %__v.i524, align 1, !tbaa !3
  %add.ptr111 = getelementptr inbounds float, float* %add.ptr109, i64 %mul3, !intel-tbaa !6
  %__v.i523 = bitcast float* %add.ptr111 to <16 x float>*
  store <16 x float> %add.i527, <16 x float>* %__v.i523, align 1, !tbaa !3
  %add.ptr112 = getelementptr inbounds float, float* %add.ptr109, i64 %add4, !intel-tbaa !6
  %__v.i522 = bitcast float* %add.ptr112 to <16 x float>*
  store <16 x float> %add.i526, <16 x float>* %__v.i522, align 1, !tbaa !3
  %add.i521 = fadd <16 x float> %add.i555, %add.i539
  %add.i520 = fadd <16 x float> %add.i563, %add.i547
  %add.i519 = fadd <16 x float> %add.i555, %add.i521
  %add.i518 = fadd <16 x float> %add.i547, %add.i520
  %add.ptr117 = getelementptr inbounds float, float* %add.ptr109, i64 %outstride, !intel-tbaa !6
  %__v.i517 = bitcast float* %add.ptr117 to <16 x float>*
  store <16 x float> %add.i521, <16 x float>* %__v.i517, align 1, !tbaa !3
  %add.ptr118 = getelementptr inbounds float, float* %add.ptr117, i64 %mul2, !intel-tbaa !6
  %__v.i516 = bitcast float* %add.ptr118 to <16 x float>*
  store <16 x float> %add.i520, <16 x float>* %__v.i516, align 1, !tbaa !3
  %add.ptr119 = getelementptr inbounds float, float* %add.ptr117, i64 %mul3, !intel-tbaa !6
  %__v.i515 = bitcast float* %add.ptr119 to <16 x float>*
  store <16 x float> %add.i519, <16 x float>* %__v.i515, align 1, !tbaa !3
  %add.ptr120 = getelementptr inbounds float, float* %add.ptr117, i64 %add4, !intel-tbaa !6
  %__v.i514 = bitcast float* %add.ptr120 to <16 x float>*
  store <16 x float> %add.i518, <16 x float>* %__v.i514, align 1, !tbaa !3
  %add.i513 = fadd <16 x float> %add.i554, %add.i538
  %add.i512 = fadd <16 x float> %add.i562, %add.i546
  %add.i511 = fadd <16 x float> %add.i554, %add.i513
  %add.i510 = fadd <16 x float> %add.i546, %add.i512
  %add.ptr125 = getelementptr inbounds float, float* %add.ptr117, i64 %outstride, !intel-tbaa !6
  %__v.i509 = bitcast float* %add.ptr125 to <16 x float>*
  store <16 x float> %add.i513, <16 x float>* %__v.i509, align 1, !tbaa !3
  %add.ptr126 = getelementptr inbounds float, float* %add.ptr125, i64 %mul2, !intel-tbaa !6
  %__v.i508 = bitcast float* %add.ptr126 to <16 x float>*
  store <16 x float> %add.i512, <16 x float>* %__v.i508, align 1, !tbaa !3
  %add.ptr127 = getelementptr inbounds float, float* %add.ptr125, i64 %mul3, !intel-tbaa !6
  %__v.i507 = bitcast float* %add.ptr127 to <16 x float>*
  store <16 x float> %add.i511, <16 x float>* %__v.i507, align 1, !tbaa !3
  %add.ptr128 = getelementptr inbounds float, float* %add.ptr125, i64 %add4, !intel-tbaa !6
  %__v.i506 = bitcast float* %add.ptr128 to <16 x float>*
  store <16 x float> %add.i510, <16 x float>* %__v.i506, align 1, !tbaa !3
  %add.i505 = fadd <16 x float> %add.i569, %add.i553
  %add.i504 = fadd <16 x float> %add.i561, %add.i545
  %add.i503 = fadd <16 x float> %add.i553, %add.i505
  %add.i502 = fadd <16 x float> %add.i545, %add.i504
  %add.ptr133 = getelementptr inbounds float, float* %add.ptr125, i64 %outstride, !intel-tbaa !6
  %__v.i501 = bitcast float* %add.ptr133 to <16 x float>*
  store <16 x float> %add.i505, <16 x float>* %__v.i501, align 1, !tbaa !3
  %add.ptr134 = getelementptr inbounds float, float* %add.ptr133, i64 %mul2, !intel-tbaa !6
  %__v.i500 = bitcast float* %add.ptr134 to <16 x float>*
  store <16 x float> %add.i504, <16 x float>* %__v.i500, align 1, !tbaa !3
  %add.ptr135 = getelementptr inbounds float, float* %add.ptr133, i64 %mul3, !intel-tbaa !6
  %__v.i499 = bitcast float* %add.ptr135 to <16 x float>*
  store <16 x float> %add.i503, <16 x float>* %__v.i499, align 1, !tbaa !3
  %add.ptr136 = getelementptr inbounds float, float* %add.ptr133, i64 %add4, !intel-tbaa !6
  %__v.i498 = bitcast float* %add.ptr136 to <16 x float>*
  store <16 x float> %add.i502, <16 x float>* %__v.i498, align 1, !tbaa !3
  %add.i497 = fadd <16 x float> %add.i568, %add.i552
  %add.i496 = fadd <16 x float> %add.i560, %add.i544
  %add.i495 = fadd <16 x float> %add.i552, %add.i497
  %add.i494 = fadd <16 x float> %add.i544, %add.i496
  %add.ptr141 = getelementptr inbounds float, float* %add.ptr133, i64 %outstride, !intel-tbaa !6
  %__v.i493 = bitcast float* %add.ptr141 to <16 x float>*
  store <16 x float> %add.i497, <16 x float>* %__v.i493, align 1, !tbaa !3
  %add.ptr142 = getelementptr inbounds float, float* %add.ptr141, i64 %mul2, !intel-tbaa !6
  %__v.i492 = bitcast float* %add.ptr142 to <16 x float>*
  store <16 x float> %add.i496, <16 x float>* %__v.i492, align 1, !tbaa !3
  %add.ptr143 = getelementptr inbounds float, float* %add.ptr141, i64 %mul3, !intel-tbaa !6
  %__v.i491 = bitcast float* %add.ptr143 to <16 x float>*
  store <16 x float> %add.i495, <16 x float>* %__v.i491, align 1, !tbaa !3
  %add.ptr144 = getelementptr inbounds float, float* %add.ptr141, i64 %add4, !intel-tbaa !6
  %__v.i490 = bitcast float* %add.ptr144 to <16 x float>*
  store <16 x float> %add.i494, <16 x float>* %__v.i490, align 1, !tbaa !3
  %add.i489 = fadd <16 x float> %add.i567, %add.i551
  %add.i488 = fadd <16 x float> %add.i559, %add.i543
  %add.i487 = fadd <16 x float> %add.i551, %add.i489
  %add.i486 = fadd <16 x float> %add.i543, %add.i488
  %add.ptr149 = getelementptr inbounds float, float* %add.ptr141, i64 %outstride, !intel-tbaa !6
  %__v.i485 = bitcast float* %add.ptr149 to <16 x float>*
  store <16 x float> %add.i489, <16 x float>* %__v.i485, align 1, !tbaa !3
  %add.ptr150 = getelementptr inbounds float, float* %add.ptr149, i64 %mul2, !intel-tbaa !6
  %__v.i484 = bitcast float* %add.ptr150 to <16 x float>*
  store <16 x float> %add.i488, <16 x float>* %__v.i484, align 1, !tbaa !3
  %add.ptr151 = getelementptr inbounds float, float* %add.ptr149, i64 %mul3, !intel-tbaa !6
  %__v.i483 = bitcast float* %add.ptr151 to <16 x float>*
  store <16 x float> %add.i487, <16 x float>* %__v.i483, align 1, !tbaa !3
  %add.ptr152 = getelementptr inbounds float, float* %add.ptr149, i64 %add4, !intel-tbaa !6
  %__v.i482 = bitcast float* %add.ptr152 to <16 x float>*
  store <16 x float> %add.i486, <16 x float>* %__v.i482, align 1, !tbaa !3
  %add.i481 = fadd <16 x float> %add.i566, %add.i550
  %add.i480 = fadd <16 x float> %add.i558, %add.i542
  %add.i479 = fadd <16 x float> %add.i550, %add.i481
  %add.i = fadd <16 x float> %add.i542, %add.i480
  %add.ptr157 = getelementptr inbounds float, float* %add.ptr149, i64 %outstride, !intel-tbaa !6
  %__v.i478 = bitcast float* %add.ptr157 to <16 x float>*
  store <16 x float> %add.i481, <16 x float>* %__v.i478, align 1, !tbaa !3
  %add.ptr158 = getelementptr inbounds float, float* %add.ptr157, i64 %mul2, !intel-tbaa !6
  %__v.i477 = bitcast float* %add.ptr158 to <16 x float>*
  store <16 x float> %add.i480, <16 x float>* %__v.i477, align 1, !tbaa !3
  %add.ptr159 = getelementptr inbounds float, float* %add.ptr157, i64 %mul3, !intel-tbaa !6
  %__v.i476 = bitcast float* %add.ptr159 to <16 x float>*
  store <16 x float> %add.i479, <16 x float>* %__v.i476, align 1, !tbaa !3
  %add.ptr160 = getelementptr inbounds float, float* %add.ptr157, i64 %add4, !intel-tbaa !6
  %__v.i475 = bitcast float* %add.ptr160 to <16 x float>*
  store <16 x float> %add.i, <16 x float>* %__v.i475, align 1, !tbaa !3
  %add.ptr161 = getelementptr inbounds float, float* %in.addr.0604, i64 16, !intel-tbaa !6
  %add.ptr162 = getelementptr inbounds float, float* %out.addr.0603, i64 16, !intel-tbaa !6
  %add163 = add nuw nsw i64 %i.0602, 16
  %cmp = icmp slt i64 %add163, %N
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !llvm.loop !8
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="512" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+avx,+avx2,+avx512f,+crc32,+cx8,+f16c,+fma,+fxsr,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave" "tune-cpu"="generic" "unsafe-fp-math"="false" }

!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!3 = !{!4, !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"float", !4, i64 0}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
