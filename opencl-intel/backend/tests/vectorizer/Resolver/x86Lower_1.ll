; RUN: llvm-as %s -o %t.bc
; RUN: opt -runtimelib %p/../Full/dxruntime.bc  -x86lower -verify %t.bc -S -o %t1.ll -runtime=dx

;;LLVMIR start 
; ModuleID = 'dx2llvm'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

@icb = internal constant [1 x <4 x float>] [<4 x float> <float 0x36DC000000000000, float 0xFFFFFFFFE0000000, float 0x36D4000000000000, float 0xFFFFFFFFE0000000>]

define fastcc void @aos_shader(i8* noalias nocapture %gc, i8* noalias nocapture %lc) nounwind {
entry:
  %0 = alloca [1024 x <4 x float>], align 32
  %1 = alloca [1024 x <4 x float>], align 32
  %2 = alloca [1024 x <4 x float>], align 32
  %3 = alloca [1024 x <4 x float>], align 32
  %4 = alloca [1024 x <4 x float>], align 32
  %5 = alloca [1024 x <4 x float>], align 32
  %6 = alloca [1024 x <4 x float>], align 32
  %7 = alloca [1024 x <4 x float>], align 32
  %8 = alloca [1024 x <4 x float>], align 32
  %9 = alloca [1024 x <4 x float>], align 32
  %10 = alloca [1024 x <4 x float>], align 32
  %11 = alloca [1024 x <4 x float>], align 32
  %12 = alloca [512 x <4 x float>], align 32
  %13 = alloca [512 x <4 x float>], align 32
  %14 = alloca [512 x <4 x float>], align 32
  %15 = alloca [512 x <4 x float>], align 32
  %16 = alloca [512 x <4 x float>], align 32
  %17 = alloca [512 x <4 x float>], align 32
  %18 = alloca [512 x <4 x float>], align 32
  %19 = alloca [512 x <4 x float>], align 32
  %20 = call i1 @allZero_v4(<4 x i1> <i1 true, i1 true, i1 true, i1 true>)
  br i1 %20, label %header1568, label %bgnloop

bgnloop:                                          ; preds = %entry, %postload2488
  %vectorPHI = phi <4 x i1> [ zeroinitializer, %entry ], [ %loop_mask10291591, %postload2488 ]
  %vectorPHI1584 = phi <4 x i1> [ zeroinitializer, %entry ], [ %ever_left_loop1590, %postload2488 ]
  %vectorPHI1585 = phi <4 x i1> [ <i1 true, i1 true, i1 true, i1 true>, %entry ], [ %local_edge1593, %postload2488 ]
  %vectorPHI1586 = phi <4 x float> [ <float 0x3736E00000000000, float 0x3736E00000000000, float 0x3736E00000000000, float 0x3736E00000000000>, %entry ], [ %bitcast103211595, %postload2488 ]
  %bitcast1587 = bitcast <4 x float> %vectorPHI1586 to <4 x i32>
  %21 = icmp eq <4 x i32> %bitcast1587, zeroinitializer
  %notCond1588 = xor <4 x i1> %21, <i1 true, i1 true, i1 true, i1 true>
  %who_left_tr1589 = and <4 x i1> %vectorPHI1585, %21
  %ever_left_loop1590 = or <4 x i1> %vectorPHI1584, %who_left_tr1589
  %loop_mask10291591 = or <4 x i1> %vectorPHI, %who_left_tr1589
  %local_edge1593 = and <4 x i1> %vectorPHI1585, %notCond1588
  %extract1601 = extractelement <4 x i1> %local_edge1593, i32 1
  %extract1602 = extractelement <4 x i1> %local_edge1593, i32 2
  %extract1603 = extractelement <4 x i1> %local_edge1593, i32 3
  %extract1600 = extractelement <4 x i1> %local_edge1593, i32 0
  %bitcast93171594 = bitcast <4 x float> %vectorPHI1586 to <4 x i32>
  %22 = add <4 x i32> %bitcast93171594, <i32 -1, i32 -1, i32 -1, i32 -1>
  %bitcast103211595 = bitcast <4 x i32> %22 to <4 x float>
  %23 = and <4 x i32> %22, <i32 1023, i32 1023, i32 1023, i32 1023>
  %extract1597 = extractelement <4 x i32> %23, i32 1
  %extract1598 = extractelement <4 x i32> %23, i32 2
  %extract1599 = extractelement <4 x i32> %23, i32 3
  %24 = getelementptr inbounds [1024 x <4 x float>]* %1, i32 0, i32 %extract1597
  %25 = getelementptr inbounds [1024 x <4 x float>]* %2, i32 0, i32 %extract1598
  %26 = getelementptr inbounds [1024 x <4 x float>]* %3, i32 0, i32 %extract1599
  br i1 %extract1600, label %deload2493, label %postload2494

deload2493:                                       ; preds = %bgnloop
  %extract = extractelement <4 x i32> %23, i32 0
  %27 = getelementptr inbounds [1024 x <4 x float>]* %0, i32 0, i32 %extract
  store <4 x float> zeroinitializer, <4 x float>* %27
  br label %postload2494

postload2494:                                     ; preds = %bgnloop, %deload2493
  br i1 %extract1601, label %deload2491, label %postload2492

deload2491:                                       ; preds = %postload2494
  store <4 x float> zeroinitializer, <4 x float>* %24
  br label %postload2492

postload2492:                                     ; preds = %postload2494, %deload2491
  br i1 %extract1602, label %deload2485, label %postload2486

deload2485:                                       ; preds = %postload2492
  store <4 x float> zeroinitializer, <4 x float>* %25
  br label %postload2486

postload2486:                                     ; preds = %postload2492, %deload2485
  br i1 %extract1603, label %deload2487, label %postload2488

deload2487:                                       ; preds = %postload2486
  store <4 x float> zeroinitializer, <4 x float>* %26
  br label %postload2488

postload2488:                                     ; preds = %postload2486, %deload2487
  %28 = call i1 @allOne_v4(<4 x i1> %loop_mask10291591)
  br i1 %28, label %header1568, label %bgnloop

header1568:                                       ; preds = %entry, %postload2488
  %vectorPHI1604 = phi <4 x i1> [ zeroinitializer, %entry ], [ %ever_left_loop1590, %postload2488 ]
  %negIncomingLoopMask11971605 = xor <4 x i1> %vectorPHI1604, <i1 true, i1 true, i1 true, i1 true>
  %29 = call i1 @allZero_v4(<4 x i1> %vectorPHI1604)
  br i1 %29, label %header, label %bgnloop14

bgnloop14:                                        ; preds = %header1568, %postload
  %vectorPHI1606 = phi <4 x i1> [ %negIncomingLoopMask11971605, %header1568 ], [ %loop_mask10361614, %postload ]
  %vectorPHI1607 = phi <4 x i1> [ zeroinitializer, %header1568 ], [ %ever_left_loop10341613, %postload ]
  %vectorPHI1608 = phi <4 x i1> [ %vectorPHI1604, %header1568 ], [ %local_edge10411616, %postload ]
  %vectorPHI1609 = phi <4 x float> [ <float 0x3730700000000000, float 0x3730700000000000, float 0x3730700000000000, float 0x3730700000000000>, %header1568 ], [ %bitcast203371618, %postload ]
  %bitcast151610 = bitcast <4 x float> %vectorPHI1609 to <4 x i32>
  %30 = icmp eq <4 x i32> %bitcast151610, zeroinitializer
  %notCond10321611 = xor <4 x i1> %30, <i1 true, i1 true, i1 true, i1 true>
  %who_left_tr10331612 = and <4 x i1> %vectorPHI1608, %30
  %ever_left_loop10341613 = or <4 x i1> %vectorPHI1607, %who_left_tr10331612
  %loop_mask10361614 = or <4 x i1> %vectorPHI1606, %who_left_tr10331612
  %local_edge10411616 = and <4 x i1> %vectorPHI1608, %notCond10321611
  %extract1625 = extractelement <4 x i1> %local_edge10411616, i32 1
  %extract1626 = extractelement <4 x i1> %local_edge10411616, i32 2
  %extract1627 = extractelement <4 x i1> %local_edge10411616, i32 3
  %extract1624 = extractelement <4 x i1> %local_edge10411616, i32 0
  %bitcast193331617 = bitcast <4 x float> %vectorPHI1609 to <4 x i32>
  %31 = add <4 x i32> %bitcast193331617, <i32 -1, i32 -1, i32 -1, i32 -1>
  %bitcast203371618 = bitcast <4 x i32> %31 to <4 x float>
  %32 = and <4 x i32> %31, <i32 1023, i32 1023, i32 1023, i32 1023>
  %extract1621 = extractelement <4 x i32> %32, i32 1
  %extract1622 = extractelement <4 x i32> %32, i32 2
  %extract1623 = extractelement <4 x i32> %32, i32 3
  %33 = getelementptr inbounds [1024 x <4 x float>]* %5, i32 0, i32 %extract1621
  %34 = getelementptr inbounds [1024 x <4 x float>]* %6, i32 0, i32 %extract1622
  %35 = getelementptr inbounds [1024 x <4 x float>]* %7, i32 0, i32 %extract1623
  br i1 %extract1624, label %deload2566, label %postload2567

deload2566:                                       ; preds = %bgnloop14
  %extract1620 = extractelement <4 x i32> %32, i32 0
  %36 = getelementptr inbounds [1024 x <4 x float>]* %4, i32 0, i32 %extract1620
  store <4 x float> zeroinitializer, <4 x float>* %36
  br label %postload2567

postload2567:                                     ; preds = %bgnloop14, %deload2566
  br i1 %extract1625, label %deload2481, label %postload2482

deload2481:                                       ; preds = %postload2567
  store <4 x float> zeroinitializer, <4 x float>* %33
  br label %postload2482

postload2482:                                     ; preds = %postload2567, %deload2481
  br i1 %extract1626, label %deload2479, label %postload2480

deload2479:                                       ; preds = %postload2482
  store <4 x float> zeroinitializer, <4 x float>* %34
  br label %postload2480

postload2480:                                     ; preds = %postload2482, %deload2479
  br i1 %extract1627, label %deload, label %postload

deload:                                           ; preds = %postload2480
  store <4 x float> zeroinitializer, <4 x float>* %35
  br label %postload

postload:                                         ; preds = %postload2480, %deload
  %37 = call i1 @allOne_v4(<4 x i1> %loop_mask10361614)
  br i1 %37, label %header, label %bgnloop14

header:                                           ; preds = %header1568, %postload
  %vectorPHI1628 = phi <4 x i1> [ zeroinitializer, %header1568 ], [ %ever_left_loop10341613, %postload ]
  %negIncomingLoopMask12031629 = xor <4 x i1> %vectorPHI1628, <i1 true, i1 true, i1 true, i1 true>
  %38 = call i1 @allZero_v4(<4 x i1> %vectorPHI1628)
  br i1 %38, label %header1553, label %bgnloop26

bgnloop26:                                        ; preds = %header, %postload2508
  %vectorPHI1630 = phi <4 x i1> [ %negIncomingLoopMask12031629, %header ], [ %loop_mask10481638, %postload2508 ]
  %vectorPHI1631 = phi <4 x i1> [ zeroinitializer, %header ], [ %ever_left_loop10461637, %postload2508 ]
  %vectorPHI1632 = phi <4 x i1> [ %vectorPHI1628, %header ], [ %local_edge10531640, %postload2508 ]
  %vectorPHI1633 = phi <4 x float> [ <float 0x3733A80000000000, float 0x3733A80000000000, float 0x3733A80000000000, float 0x3733A80000000000>, %header ], [ %bitcast323531642, %postload2508 ]
  %bitcast271634 = bitcast <4 x float> %vectorPHI1633 to <4 x i32>
  %39 = icmp eq <4 x i32> %bitcast271634, zeroinitializer
  %notCond10441635 = xor <4 x i1> %39, <i1 true, i1 true, i1 true, i1 true>
  %who_left_tr10451636 = and <4 x i1> %vectorPHI1632, %39
  %ever_left_loop10461637 = or <4 x i1> %vectorPHI1631, %who_left_tr10451636
  %loop_mask10481638 = or <4 x i1> %vectorPHI1630, %who_left_tr10451636
  %local_edge10531640 = and <4 x i1> %vectorPHI1632, %notCond10441635
  %extract1649 = extractelement <4 x i1> %local_edge10531640, i32 1
  %extract1650 = extractelement <4 x i1> %local_edge10531640, i32 2
  %extract1651 = extractelement <4 x i1> %local_edge10531640, i32 3
  %extract1648 = extractelement <4 x i1> %local_edge10531640, i32 0
  %bitcast313491641 = bitcast <4 x float> %vectorPHI1633 to <4 x i32>
  %40 = add <4 x i32> %bitcast313491641, <i32 -1, i32 -1, i32 -1, i32 -1>
  %bitcast323531642 = bitcast <4 x i32> %40 to <4 x float>
  %41 = and <4 x i32> %40, <i32 1023, i32 1023, i32 1023, i32 1023>
  %extract1645 = extractelement <4 x i32> %41, i32 1
  %extract1646 = extractelement <4 x i32> %41, i32 2
  %extract1647 = extractelement <4 x i32> %41, i32 3
  %42 = getelementptr inbounds [1024 x <4 x float>]* %9, i32 0, i32 %extract1645
  %43 = getelementptr inbounds [1024 x <4 x float>]* %10, i32 0, i32 %extract1646
  %44 = getelementptr inbounds [1024 x <4 x float>]* %11, i32 0, i32 %extract1647
  br i1 %extract1648, label %deload2516, label %postload2517

deload2516:                                       ; preds = %bgnloop26
  %extract1644 = extractelement <4 x i32> %41, i32 0
  %45 = getelementptr inbounds [1024 x <4 x float>]* %8, i32 0, i32 %extract1644
  store <4 x float> zeroinitializer, <4 x float>* %45
  br label %postload2517

postload2517:                                     ; preds = %bgnloop26, %deload2516
  br i1 %extract1649, label %deload2514, label %postload2515

deload2514:                                       ; preds = %postload2517
  store <4 x float> zeroinitializer, <4 x float>* %42
  br label %postload2515

postload2515:                                     ; preds = %postload2517, %deload2514
  br i1 %extract1650, label %deload2509, label %postload2510

deload2509:                                       ; preds = %postload2515
  store <4 x float> zeroinitializer, <4 x float>* %43
  br label %postload2510

postload2510:                                     ; preds = %postload2515, %deload2509
  br i1 %extract1651, label %deload2507, label %postload2508

deload2507:                                       ; preds = %postload2510
  store <4 x float> zeroinitializer, <4 x float>* %44
  br label %postload2508

postload2508:                                     ; preds = %postload2510, %deload2507
  %46 = call i1 @allOne_v4(<4 x i1> %loop_mask10481638)
  br i1 %46, label %header1553, label %bgnloop26

header1553:                                       ; preds = %header, %postload2508
  %vectorPHI1652 = phi <4 x i1> [ zeroinitializer, %header ], [ %ever_left_loop10461637, %postload2508 ]
  %negIncomingLoopMask12091653 = xor <4 x i1> %vectorPHI1652, <i1 true, i1 true, i1 true, i1 true>
  %47 = call i1 @allZero_v4(<4 x i1> %vectorPHI1652)
  br i1 %47, label %header1571, label %bgnloop38

bgnloop38:                                        ; preds = %header1553, %postload2490
  %vectorPHI1654 = phi <4 x i1> [ %negIncomingLoopMask12091653, %header1553 ], [ %loop_mask10601662, %postload2490 ]
  %vectorPHI1655 = phi <4 x i1> [ zeroinitializer, %header1553 ], [ %ever_left_loop10581661, %postload2490 ]
  %vectorPHI1656 = phi <4 x i1> [ %vectorPHI1652, %header1553 ], [ %local_edge10651664, %postload2490 ]
  %vectorPHI1657 = phi <4 x float> [ <float 0x3724F00000000000, float 0x3724F00000000000, float 0x3724F00000000000, float 0x3724F00000000000>, %header1553 ], [ %bitcast443691666, %postload2490 ]
  %bitcast391658 = bitcast <4 x float> %vectorPHI1657 to <4 x i32>
  %48 = icmp eq <4 x i32> %bitcast391658, zeroinitializer
  %notCond10561659 = xor <4 x i1> %48, <i1 true, i1 true, i1 true, i1 true>
  %who_left_tr10571660 = and <4 x i1> %vectorPHI1656, %48
  %ever_left_loop10581661 = or <4 x i1> %vectorPHI1655, %who_left_tr10571660
  %loop_mask10601662 = or <4 x i1> %vectorPHI1654, %who_left_tr10571660
  %local_edge10651664 = and <4 x i1> %vectorPHI1656, %notCond10561659
  %extract1673 = extractelement <4 x i1> %local_edge10651664, i32 1
  %extract1674 = extractelement <4 x i1> %local_edge10651664, i32 2
  %extract1675 = extractelement <4 x i1> %local_edge10651664, i32 3
  %extract1672 = extractelement <4 x i1> %local_edge10651664, i32 0
  %bitcast433651665 = bitcast <4 x float> %vectorPHI1657 to <4 x i32>
  %49 = add <4 x i32> %bitcast433651665, <i32 -1, i32 -1, i32 -1, i32 -1>
  %bitcast443691666 = bitcast <4 x i32> %49 to <4 x float>
  %50 = and <4 x i32> %49, <i32 511, i32 511, i32 511, i32 511>
  %extract1669 = extractelement <4 x i32> %50, i32 1
  %extract1670 = extractelement <4 x i32> %50, i32 2
  %extract1671 = extractelement <4 x i32> %50, i32 3
  %51 = getelementptr inbounds [512 x <4 x float>]* %13, i32 0, i32 %extract1669
  %52 = getelementptr inbounds [512 x <4 x float>]* %14, i32 0, i32 %extract1670
  %53 = getelementptr inbounds [512 x <4 x float>]* %15, i32 0, i32 %extract1671
  br i1 %extract1672, label %deload2505, label %postload2506

deload2505:                                       ; preds = %bgnloop38
  %extract1668 = extractelement <4 x i32> %50, i32 0
  %54 = getelementptr inbounds [512 x <4 x float>]* %12, i32 0, i32 %extract1668
  store <4 x float> zeroinitializer, <4 x float>* %54
  br label %postload2506

postload2506:                                     ; preds = %bgnloop38, %deload2505
  br i1 %extract1673, label %deload2503, label %postload2504

deload2503:                                       ; preds = %postload2506
  store <4 x float> zeroinitializer, <4 x float>* %51
  br label %postload2504

postload2504:                                     ; preds = %postload2506, %deload2503
  br i1 %extract1674, label %deload2501, label %postload2502

deload2501:                                       ; preds = %postload2504
  store <4 x float> zeroinitializer, <4 x float>* %52
  br label %postload2502

postload2502:                                     ; preds = %postload2504, %deload2501
  br i1 %extract1675, label %deload2489, label %postload2490

deload2489:                                       ; preds = %postload2502
  store <4 x float> zeroinitializer, <4 x float>* %53
  br label %postload2490

postload2490:                                     ; preds = %postload2502, %deload2489
  %55 = call i1 @allOne_v4(<4 x i1> %loop_mask10601662)
  br i1 %55, label %header1571, label %bgnloop38

header1571:                                       ; preds = %header1553, %postload2490
  %vectorPHI1676 = phi <4 x i1> [ zeroinitializer, %header1553 ], [ %ever_left_loop10581661, %postload2490 ]
  %negIncomingLoopMask12151677 = xor <4 x i1> %vectorPHI1676, <i1 true, i1 true, i1 true, i1 true>
  %56 = call i1 @allZero_v4(<4 x i1> %vectorPHI1676)
  br i1 %56, label %header1556, label %bgnloop50

bgnloop50:                                        ; preds = %header1571, %postload2519
  %vectorPHI1678 = phi <4 x i1> [ %negIncomingLoopMask12151677, %header1571 ], [ %loop_mask10721686, %postload2519 ]
  %vectorPHI1679 = phi <4 x i1> [ zeroinitializer, %header1571 ], [ %ever_left_loop10701685, %postload2519 ]
  %vectorPHI1680 = phi <4 x i1> [ %vectorPHI1676, %header1571 ], [ %local_edge10771688, %postload2519 ]
  %vectorPHI1681 = phi <4 x float> [ <float 0x372C700000000000, float 0x372C700000000000, float 0x372C700000000000, float 0x372C700000000000>, %header1571 ], [ %bitcast563851690, %postload2519 ]
  %bitcast511682 = bitcast <4 x float> %vectorPHI1681 to <4 x i32>
  %57 = icmp eq <4 x i32> %bitcast511682, zeroinitializer
  %notCond10681683 = xor <4 x i1> %57, <i1 true, i1 true, i1 true, i1 true>
  %who_left_tr10691684 = and <4 x i1> %vectorPHI1680, %57
  %ever_left_loop10701685 = or <4 x i1> %vectorPHI1679, %who_left_tr10691684
  %loop_mask10721686 = or <4 x i1> %vectorPHI1678, %who_left_tr10691684
  %local_edge10771688 = and <4 x i1> %vectorPHI1680, %notCond10681683
  %extract1697 = extractelement <4 x i1> %local_edge10771688, i32 1
  %extract1698 = extractelement <4 x i1> %local_edge10771688, i32 2
  %extract1699 = extractelement <4 x i1> %local_edge10771688, i32 3
  %extract1696 = extractelement <4 x i1> %local_edge10771688, i32 0
  %bitcast553811689 = bitcast <4 x float> %vectorPHI1681 to <4 x i32>
  %58 = add <4 x i32> %bitcast553811689, <i32 -1, i32 -1, i32 -1, i32 -1>
  %bitcast563851690 = bitcast <4 x i32> %58 to <4 x float>
  %59 = and <4 x i32> %58, <i32 511, i32 511, i32 511, i32 511>
  %extract1693 = extractelement <4 x i32> %59, i32 1
  %extract1694 = extractelement <4 x i32> %59, i32 2
  %extract1695 = extractelement <4 x i32> %59, i32 3
  %60 = getelementptr inbounds [512 x <4 x float>]* %17, i32 0, i32 %extract1693
  %61 = getelementptr inbounds [512 x <4 x float>]* %18, i32 0, i32 %extract1694
  %62 = getelementptr inbounds [512 x <4 x float>]* %19, i32 0, i32 %extract1695
  br i1 %extract1696, label %deload2568, label %postload2569

deload2568:                                       ; preds = %bgnloop50
  %extract1692 = extractelement <4 x i32> %59, i32 0
  %63 = getelementptr inbounds [512 x <4 x float>]* %16, i32 0, i32 %extract1692
  store <4 x float> zeroinitializer, <4 x float>* %63
  br label %postload2569

postload2569:                                     ; preds = %bgnloop50, %deload2568
  br i1 %extract1697, label %deload2576, label %postload2577

deload2576:                                       ; preds = %postload2569
  store <4 x float> zeroinitializer, <4 x float>* %60
  br label %postload2577

postload2577:                                     ; preds = %postload2569, %deload2576
  br i1 %extract1698, label %deload2578, label %postload2579

deload2578:                                       ; preds = %postload2577
  store <4 x float> zeroinitializer, <4 x float>* %61
  br label %postload2579

postload2579:                                     ; preds = %postload2577, %deload2578
  br i1 %extract1699, label %deload2518, label %postload2519

deload2518:                                       ; preds = %postload2579
  store <4 x float> zeroinitializer, <4 x float>* %62
  br label %postload2519

postload2519:                                     ; preds = %postload2579, %deload2518
  %64 = call i1 @allOne_v4(<4 x i1> %loop_mask10721686)
  br i1 %64, label %header1556, label %bgnloop50

header1556:                                       ; preds = %header1571, %postload2519
  %vectorPHI1700 = phi <4 x i1> [ zeroinitializer, %header1571 ], [ %ever_left_loop10701685, %postload2519 ]
  %negIncomingLoopMask12211701 = xor <4 x i1> %vectorPHI1700, <i1 true, i1 true, i1 true, i1 true>
  %65 = call i1 @allZero_v4(<4 x i1> %vectorPHI1700)
  br i1 %65, label %header1559, label %bgnloop62

bgnloop62:                                        ; preds = %header1556, %postload2527
  %vectorPHI1702 = phi <4 x i1> [ %negIncomingLoopMask12211701, %header1556 ], [ %loop_mask10841721, %postload2527 ]
  %vectorPHI1703 = phi <4 x i1> [ zeroinitializer, %header1556 ], [ %ever_left_loop10821720, %postload2527 ]
  %vectorPHI1704 = phi <4 x i1> [ %vectorPHI1700, %header1556 ], [ %local_edge10891723, %postload2527 ]
  %vectorPHI1705 = phi <4 x float> [ <float 0x3736E00000000000, float 0x3736E00000000000, float 0x3736E00000000000, float 0x3736E00000000000>, %header1556 ], [ %bitcast644011707, %postload2527 ]
  %extract1713 = extractelement <4 x i1> %vectorPHI1704, i32 0
  %extract1714 = extractelement <4 x i1> %vectorPHI1704, i32 1
  %extract1715 = extractelement <4 x i1> %vectorPHI1704, i32 2
  %extract1716 = extractelement <4 x i1> %vectorPHI1704, i32 3
  %bitcast633971706 = bitcast <4 x float> %vectorPHI1705 to <4 x i32>
  %66 = add <4 x i32> %bitcast633971706, <i32 -1, i32 -1, i32 -1, i32 -1>
  %bitcast644011707 = bitcast <4 x i32> %66 to <4 x float>
  %67 = and <4 x i32> %66, <i32 1023, i32 1023, i32 1023, i32 1023>
  %extract1710 = extractelement <4 x i32> %67, i32 1
  %extract1711 = extractelement <4 x i32> %67, i32 2
  %extract1712 = extractelement <4 x i32> %67, i32 3
  %68 = getelementptr inbounds [1024 x <4 x float>]* %1, i32 0, i32 %extract1710
  %69 = getelementptr inbounds [1024 x <4 x float>]* %2, i32 0, i32 %extract1711
  %70 = getelementptr inbounds [1024 x <4 x float>]* %3, i32 0, i32 %extract1712
  br i1 %extract1713, label %deload2520, label %postload2521

deload2520:                                       ; preds = %bgnloop62
  %extract1709 = extractelement <4 x i32> %67, i32 0
  %71 = getelementptr inbounds [1024 x <4 x float>]* %0, i32 0, i32 %extract1709
  store <4 x float> <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, <4 x float>* %71
  br label %postload2521

postload2521:                                     ; preds = %bgnloop62, %deload2520
  br i1 %extract1714, label %deload2522, label %postload2523

deload2522:                                       ; preds = %postload2521
  store <4 x float> <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, <4 x float>* %68
  br label %postload2523

postload2523:                                     ; preds = %postload2521, %deload2522
  br i1 %extract1715, label %deload2524, label %postload2525

deload2524:                                       ; preds = %postload2523
  store <4 x float> <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, <4 x float>* %69
  br label %postload2525

postload2525:                                     ; preds = %postload2523, %deload2524
  br i1 %extract1716, label %deload2526, label %postload2527

deload2526:                                       ; preds = %postload2525
  store <4 x float> <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, <4 x float>* %70
  br label %postload2527

postload2527:                                     ; preds = %postload2525, %deload2526
  %72 = icmp eq <4 x i32> %66, zeroinitializer
  %notCond10801718 = xor <4 x i1> %72, <i1 true, i1 true, i1 true, i1 true>
  %who_left_tr10811719 = and <4 x i1> %vectorPHI1704, %72
  %ever_left_loop10821720 = or <4 x i1> %vectorPHI1703, %who_left_tr10811719
  %loop_mask10841721 = or <4 x i1> %vectorPHI1702, %who_left_tr10811719
  %curr_loop_mask10861722 = or <4 x i1> %loop_mask10841721, %who_left_tr10811719
  %73 = call i1 @allOne_v4(<4 x i1> %curr_loop_mask10861722)
  %local_edge10891723 = and <4 x i1> %vectorPHI1704, %notCond10801718
  br i1 %73, label %header1559, label %bgnloop62

header1559:                                       ; preds = %header1556, %postload2527
  %vectorPHI1724 = phi <4 x i1> [ zeroinitializer, %header1556 ], [ %ever_left_loop10821720, %postload2527 ]
  %negIncomingLoopMask12261725 = xor <4 x i1> %vectorPHI1724, <i1 true, i1 true, i1 true, i1 true>
  %74 = call i1 @allZero_v4(<4 x i1> %vectorPHI1724)
  br i1 %74, label %header1574, label %bgnloop74

bgnloop74:                                        ; preds = %header1559, %postload2535
  %vectorPHI1726 = phi <4 x i1> [ %negIncomingLoopMask12261725, %header1559 ], [ %loop_mask10961745, %postload2535 ]
  %vectorPHI1727 = phi <4 x i1> [ zeroinitializer, %header1559 ], [ %ever_left_loop10941744, %postload2535 ]
  %vectorPHI1728 = phi <4 x i1> [ %vectorPHI1724, %header1559 ], [ %local_edge11011747, %postload2535 ]
  %vectorPHI1729 = phi <4 x float> [ <float 0x3730700000000000, float 0x3730700000000000, float 0x3730700000000000, float 0x3730700000000000>, %header1559 ], [ %bitcast764171731, %postload2535 ]
  %extract1737 = extractelement <4 x i1> %vectorPHI1728, i32 0
  %extract1738 = extractelement <4 x i1> %vectorPHI1728, i32 1
  %extract1739 = extractelement <4 x i1> %vectorPHI1728, i32 2
  %extract1740 = extractelement <4 x i1> %vectorPHI1728, i32 3
  %bitcast754131730 = bitcast <4 x float> %vectorPHI1729 to <4 x i32>
  %75 = add <4 x i32> %bitcast754131730, <i32 -1, i32 -1, i32 -1, i32 -1>
  %bitcast764171731 = bitcast <4 x i32> %75 to <4 x float>
  %76 = and <4 x i32> %75, <i32 1023, i32 1023, i32 1023, i32 1023>
  %extract1734 = extractelement <4 x i32> %76, i32 1
  %extract1735 = extractelement <4 x i32> %76, i32 2
  %extract1736 = extractelement <4 x i32> %76, i32 3
  %77 = getelementptr inbounds [1024 x <4 x float>]* %5, i32 0, i32 %extract1734
  %78 = getelementptr inbounds [1024 x <4 x float>]* %6, i32 0, i32 %extract1735
  %79 = getelementptr inbounds [1024 x <4 x float>]* %7, i32 0, i32 %extract1736
  br i1 %extract1737, label %deload2528, label %postload2529

deload2528:                                       ; preds = %bgnloop74
  %extract1733 = extractelement <4 x i32> %76, i32 0
  %80 = getelementptr inbounds [1024 x <4 x float>]* %4, i32 0, i32 %extract1733
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %80
  br label %postload2529

postload2529:                                     ; preds = %bgnloop74, %deload2528
  br i1 %extract1738, label %deload2530, label %postload2531

deload2530:                                       ; preds = %postload2529
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %77
  br label %postload2531

postload2531:                                     ; preds = %postload2529, %deload2530
  br i1 %extract1739, label %deload2532, label %postload2533

deload2532:                                       ; preds = %postload2531
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %78
  br label %postload2533

postload2533:                                     ; preds = %postload2531, %deload2532
  br i1 %extract1740, label %deload2534, label %postload2535

deload2534:                                       ; preds = %postload2533
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %79
  br label %postload2535

postload2535:                                     ; preds = %postload2533, %deload2534
  %81 = icmp eq <4 x i32> %75, zeroinitializer
  %notCond10921742 = xor <4 x i1> %81, <i1 true, i1 true, i1 true, i1 true>
  %who_left_tr10931743 = and <4 x i1> %vectorPHI1728, %81
  %ever_left_loop10941744 = or <4 x i1> %vectorPHI1727, %who_left_tr10931743
  %loop_mask10961745 = or <4 x i1> %vectorPHI1726, %who_left_tr10931743
  %curr_loop_mask10981746 = or <4 x i1> %loop_mask10961745, %who_left_tr10931743
  %82 = call i1 @allOne_v4(<4 x i1> %curr_loop_mask10981746)
  %local_edge11011747 = and <4 x i1> %vectorPHI1728, %notCond10921742
  br i1 %82, label %header1574, label %bgnloop74

header1574:                                       ; preds = %header1559, %postload2535
  %vectorPHI1748 = phi <4 x i1> [ zeroinitializer, %header1559 ], [ %ever_left_loop10941744, %postload2535 ]
  %negIncomingLoopMask12311749 = xor <4 x i1> %vectorPHI1748, <i1 true, i1 true, i1 true, i1 true>
  %83 = call i1 @allZero_v4(<4 x i1> %vectorPHI1748)
  br i1 %83, label %header1565, label %bgnloop86

bgnloop86:                                        ; preds = %header1574, %postload2543
  %vectorPHI1750 = phi <4 x i1> [ %negIncomingLoopMask12311749, %header1574 ], [ %loop_mask11081769, %postload2543 ]
  %vectorPHI1751 = phi <4 x i1> [ zeroinitializer, %header1574 ], [ %ever_left_loop11061768, %postload2543 ]
  %vectorPHI1752 = phi <4 x i1> [ %vectorPHI1748, %header1574 ], [ %local_edge11131771, %postload2543 ]
  %vectorPHI1753 = phi <4 x float> [ <float 0x3733A80000000000, float 0x3733A80000000000, float 0x3733A80000000000, float 0x3733A80000000000>, %header1574 ], [ %bitcast884331755, %postload2543 ]
  %extract1761 = extractelement <4 x i1> %vectorPHI1752, i32 0
  %extract1762 = extractelement <4 x i1> %vectorPHI1752, i32 1
  %extract1763 = extractelement <4 x i1> %vectorPHI1752, i32 2
  %extract1764 = extractelement <4 x i1> %vectorPHI1752, i32 3
  %bitcast874291754 = bitcast <4 x float> %vectorPHI1753 to <4 x i32>
  %84 = add <4 x i32> %bitcast874291754, <i32 -1, i32 -1, i32 -1, i32 -1>
  %bitcast884331755 = bitcast <4 x i32> %84 to <4 x float>
  %85 = and <4 x i32> %84, <i32 1023, i32 1023, i32 1023, i32 1023>
  %extract1758 = extractelement <4 x i32> %85, i32 1
  %extract1759 = extractelement <4 x i32> %85, i32 2
  %extract1760 = extractelement <4 x i32> %85, i32 3
  %86 = getelementptr inbounds [1024 x <4 x float>]* %9, i32 0, i32 %extract1758
  %87 = getelementptr inbounds [1024 x <4 x float>]* %10, i32 0, i32 %extract1759
  %88 = getelementptr inbounds [1024 x <4 x float>]* %11, i32 0, i32 %extract1760
  br i1 %extract1761, label %deload2536, label %postload2537

deload2536:                                       ; preds = %bgnloop86
  %extract1757 = extractelement <4 x i32> %85, i32 0
  %89 = getelementptr inbounds [1024 x <4 x float>]* %8, i32 0, i32 %extract1757
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %89
  br label %postload2537

postload2537:                                     ; preds = %bgnloop86, %deload2536
  br i1 %extract1762, label %deload2538, label %postload2539

deload2538:                                       ; preds = %postload2537
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %86
  br label %postload2539

postload2539:                                     ; preds = %postload2537, %deload2538
  br i1 %extract1763, label %deload2540, label %postload2541

deload2540:                                       ; preds = %postload2539
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %87
  br label %postload2541

postload2541:                                     ; preds = %postload2539, %deload2540
  br i1 %extract1764, label %deload2542, label %postload2543

deload2542:                                       ; preds = %postload2541
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %88
  br label %postload2543

postload2543:                                     ; preds = %postload2541, %deload2542
  %90 = icmp eq <4 x i32> %84, zeroinitializer
  %notCond11041766 = xor <4 x i1> %90, <i1 true, i1 true, i1 true, i1 true>
  %who_left_tr11051767 = and <4 x i1> %vectorPHI1752, %90
  %ever_left_loop11061768 = or <4 x i1> %vectorPHI1751, %who_left_tr11051767
  %loop_mask11081769 = or <4 x i1> %vectorPHI1750, %who_left_tr11051767
  %curr_loop_mask11101770 = or <4 x i1> %loop_mask11081769, %who_left_tr11051767
  %91 = call i1 @allOne_v4(<4 x i1> %curr_loop_mask11101770)
  %local_edge11131771 = and <4 x i1> %vectorPHI1752, %notCond11041766
  br i1 %91, label %header1565, label %bgnloop86

header1565:                                       ; preds = %header1574, %postload2543
  %vectorPHI1772 = phi <4 x i1> [ zeroinitializer, %header1574 ], [ %ever_left_loop11061768, %postload2543 ]
  %negIncomingLoopMask12361773 = xor <4 x i1> %vectorPHI1772, <i1 true, i1 true, i1 true, i1 true>
  %92 = call i1 @allZero_v4(<4 x i1> %vectorPHI1772)
  br i1 %92, label %header1562, label %bgnloop98

bgnloop98:                                        ; preds = %header1565, %postload2551
  %vectorPHI1774 = phi <4 x i1> [ %negIncomingLoopMask12361773, %header1565 ], [ %loop_mask11201793, %postload2551 ]
  %vectorPHI1775 = phi <4 x i1> [ zeroinitializer, %header1565 ], [ %ever_left_loop11181792, %postload2551 ]
  %vectorPHI1776 = phi <4 x i1> [ %vectorPHI1772, %header1565 ], [ %local_edge11251795, %postload2551 ]
  %vectorPHI1777 = phi <4 x float> [ <float 0x3724F00000000000, float 0x3724F00000000000, float 0x3724F00000000000, float 0x3724F00000000000>, %header1565 ], [ %bitcast1004491779, %postload2551 ]
  %extract1785 = extractelement <4 x i1> %vectorPHI1776, i32 0
  %extract1786 = extractelement <4 x i1> %vectorPHI1776, i32 1
  %extract1787 = extractelement <4 x i1> %vectorPHI1776, i32 2
  %extract1788 = extractelement <4 x i1> %vectorPHI1776, i32 3
  %bitcast994451778 = bitcast <4 x float> %vectorPHI1777 to <4 x i32>
  %93 = add <4 x i32> %bitcast994451778, <i32 -1, i32 -1, i32 -1, i32 -1>
  %bitcast1004491779 = bitcast <4 x i32> %93 to <4 x float>
  %94 = and <4 x i32> %93, <i32 511, i32 511, i32 511, i32 511>
  %extract1782 = extractelement <4 x i32> %94, i32 1
  %extract1783 = extractelement <4 x i32> %94, i32 2
  %extract1784 = extractelement <4 x i32> %94, i32 3
  %95 = getelementptr inbounds [512 x <4 x float>]* %13, i32 0, i32 %extract1782
  %96 = getelementptr inbounds [512 x <4 x float>]* %14, i32 0, i32 %extract1783
  %97 = getelementptr inbounds [512 x <4 x float>]* %15, i32 0, i32 %extract1784
  br i1 %extract1785, label %deload2544, label %postload2545

deload2544:                                       ; preds = %bgnloop98
  %extract1781 = extractelement <4 x i32> %94, i32 0
  %98 = getelementptr inbounds [512 x <4 x float>]* %12, i32 0, i32 %extract1781
  store <4 x float> <float -2.000000e+000, float -2.000000e+000, float -2.000000e+000, float -2.000000e+000>, <4 x float>* %98
  br label %postload2545

postload2545:                                     ; preds = %bgnloop98, %deload2544
  br i1 %extract1786, label %deload2546, label %postload2547

deload2546:                                       ; preds = %postload2545
  store <4 x float> <float -2.000000e+000, float -2.000000e+000, float -2.000000e+000, float -2.000000e+000>, <4 x float>* %95
  br label %postload2547

postload2547:                                     ; preds = %postload2545, %deload2546
  br i1 %extract1787, label %deload2548, label %postload2549

deload2548:                                       ; preds = %postload2547
  store <4 x float> <float -2.000000e+000, float -2.000000e+000, float -2.000000e+000, float -2.000000e+000>, <4 x float>* %96
  br label %postload2549

postload2549:                                     ; preds = %postload2547, %deload2548
  br i1 %extract1788, label %deload2550, label %postload2551

deload2550:                                       ; preds = %postload2549
  store <4 x float> <float -2.000000e+000, float -2.000000e+000, float -2.000000e+000, float -2.000000e+000>, <4 x float>* %97
  br label %postload2551

postload2551:                                     ; preds = %postload2549, %deload2550
  %99 = icmp eq <4 x i32> %93, zeroinitializer
  %notCond11161790 = xor <4 x i1> %99, <i1 true, i1 true, i1 true, i1 true>
  %who_left_tr11171791 = and <4 x i1> %vectorPHI1776, %99
  %ever_left_loop11181792 = or <4 x i1> %vectorPHI1775, %who_left_tr11171791
  %loop_mask11201793 = or <4 x i1> %vectorPHI1774, %who_left_tr11171791
  %curr_loop_mask11221794 = or <4 x i1> %loop_mask11201793, %who_left_tr11171791
  %100 = call i1 @allOne_v4(<4 x i1> %curr_loop_mask11221794)
  %local_edge11251795 = and <4 x i1> %vectorPHI1776, %notCond11161790
  br i1 %100, label %header1562, label %bgnloop98

header1562:                                       ; preds = %header1565, %postload2551
  %vectorPHI1796 = phi <4 x i1> [ zeroinitializer, %header1565 ], [ %ever_left_loop11181792, %postload2551 ]
  %negIncomingLoopMask12411797 = xor <4 x i1> %vectorPHI1796, <i1 true, i1 true, i1 true, i1 true>
  %101 = call i1 @allZero_v4(<4 x i1> %vectorPHI1796)
  br i1 %101, label %ifblock119, label %bgnloop110

bgnloop110:                                       ; preds = %header1562, %postload2559
  %vectorPHI1798 = phi <4 x i1> [ %negIncomingLoopMask12411797, %header1562 ], [ %loop_mask11321819, %postload2559 ]
  %vectorPHI1799 = phi <4 x i1> [ zeroinitializer, %header1562 ], [ %ever_left_loop11301818, %postload2559 ]
  %vectorPHI1800 = phi <4 x float> [ undef, %header1562 ], [ %out_sel1805, %postload2559 ]
  %vectorPHI1801 = phi <4 x i1> [ %vectorPHI1796, %header1562 ], [ %local_edge11371821, %postload2559 ]
  %vectorPHI1802 = phi <4 x float> [ <float 0x372C700000000000, float 0x372C700000000000, float 0x372C700000000000, float 0x372C700000000000>, %header1562 ], [ %bitcast1124651804, %postload2559 ]
  %extract1811 = extractelement <4 x i1> %vectorPHI1801, i32 0
  %extract1812 = extractelement <4 x i1> %vectorPHI1801, i32 1
  %extract1813 = extractelement <4 x i1> %vectorPHI1801, i32 2
  %extract1814 = extractelement <4 x i1> %vectorPHI1801, i32 3
  %bitcast1114611803 = bitcast <4 x float> %vectorPHI1802 to <4 x i32>
  %102 = add <4 x i32> %bitcast1114611803, <i32 -1, i32 -1, i32 -1, i32 -1>
  %bitcast1124651804 = bitcast <4 x i32> %102 to <4 x float>
  %out_sel1805 = select <4 x i1> %vectorPHI1801, <4 x float> %bitcast1124651804, <4 x float> %vectorPHI1800
  %103 = and <4 x i32> %102, <i32 511, i32 511, i32 511, i32 511>
  %extract1808 = extractelement <4 x i32> %103, i32 1
  %extract1809 = extractelement <4 x i32> %103, i32 2
  %extract1810 = extractelement <4 x i32> %103, i32 3
  %104 = getelementptr inbounds [512 x <4 x float>]* %17, i32 0, i32 %extract1808
  %105 = getelementptr inbounds [512 x <4 x float>]* %18, i32 0, i32 %extract1809
  %106 = getelementptr inbounds [512 x <4 x float>]* %19, i32 0, i32 %extract1810
  br i1 %extract1811, label %deload2552, label %postload2553

deload2552:                                       ; preds = %bgnloop110
  %extract1807 = extractelement <4 x i32> %103, i32 0
  %107 = getelementptr inbounds [512 x <4 x float>]* %16, i32 0, i32 %extract1807
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %107
  br label %postload2553

postload2553:                                     ; preds = %bgnloop110, %deload2552
  br i1 %extract1812, label %deload2554, label %postload2555

deload2554:                                       ; preds = %postload2553
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %104
  br label %postload2555

postload2555:                                     ; preds = %postload2553, %deload2554
  br i1 %extract1813, label %deload2556, label %postload2557

deload2556:                                       ; preds = %postload2555
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %105
  br label %postload2557

postload2557:                                     ; preds = %postload2555, %deload2556
  br i1 %extract1814, label %deload2558, label %postload2559

deload2558:                                       ; preds = %postload2557
  store <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float>* %106
  br label %postload2559

postload2559:                                     ; preds = %postload2557, %deload2558
  %108 = icmp eq <4 x i32> %102, zeroinitializer
  %notCond11281816 = xor <4 x i1> %108, <i1 true, i1 true, i1 true, i1 true>
  %who_left_tr11291817 = and <4 x i1> %vectorPHI1801, %108
  %ever_left_loop11301818 = or <4 x i1> %vectorPHI1799, %who_left_tr11291817
  %loop_mask11321819 = or <4 x i1> %vectorPHI1798, %who_left_tr11291817
  %curr_loop_mask11341820 = or <4 x i1> %loop_mask11321819, %who_left_tr11291817
  %109 = call i1 @allOne_v4(<4 x i1> %curr_loop_mask11341820)
  %local_edge11371821 = and <4 x i1> %vectorPHI1801, %notCond11281816
  br i1 %109, label %ifblock119, label %bgnloop110

ifblock119:                                       ; preds = %postload2559, %header1562
  %vectorPHI1823 = phi <4 x float> [ undef, %header1562 ], [ %out_sel1805, %postload2559 ]
  %vectorPHI1822 = phi <4 x i1> [ zeroinitializer, %header1562 ], [ %ever_left_loop11301818, %postload2559 ]
  %110 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> %vectorPHI1822, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 1, i32 1, i32 1, i32 1>) nounwind
  %111 = extractvalue [4 x <4 x float>] %110, 0
  %112 = extractvalue [4 x <4 x float>] %110, 1
  %113 = extractvalue [4 x <4 x float>] %110, 2
  %store.val = insertvalue [4 x <4 x float>] undef, <4 x float> %111, 0
  %store.val1836 = insertvalue [4 x <4 x float>] %store.val, <4 x float> %112, 1
  %store.val1837 = insertvalue [4 x <4 x float>] %store.val1836, <4 x float> %113, 2
  %store.val1838 = insertvalue [4 x <4 x float>] %store.val1837, <4 x float> <float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000>, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> %vectorPHI1822, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 1, i32 1, i32 1, i32 1>, [4 x <4 x float>] %store.val1838) nounwind
  %negIncomingLoopMask12461839 = xor <4 x i1> %vectorPHI1822, <i1 true, i1 true, i1 true, i1 true>
  br label %bgnloop123

bgnloop123:                                       ; preds = %bgnloop123, %ifblock119
  %vectorPHI1842 = phi <4 x i1> [ %negIncomingLoopMask12461839, %ifblock119 ], [ %loop_mask11441925, %bgnloop123 ]
  %vectorPHI1843 = phi <4 x i1> [ zeroinitializer, %ifblock119 ], [ %ever_left_loop11421924, %bgnloop123 ]
  %vectorPHI1844 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel13931917, %bgnloop123 ]
  %vectorPHI1845 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel13961916, %bgnloop123 ]
  %vectorPHI1846 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel13991915, %bgnloop123 ]
  %vectorPHI1847 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14021914, %bgnloop123 ]
  %vectorPHI1848 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14051913, %bgnloop123 ]
  %vectorPHI1849 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14081912, %bgnloop123 ]
  %vectorPHI1850 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14111911, %bgnloop123 ]
  %vectorPHI1851 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14141910, %bgnloop123 ]
  %vectorPHI1852 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14171909, %bgnloop123 ]
  %vectorPHI1853 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14201908, %bgnloop123 ]
  %vectorPHI1854 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14231907, %bgnloop123 ]
  %vectorPHI1855 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14261906, %bgnloop123 ]
  %vectorPHI1856 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14291905, %bgnloop123 ]
  %vectorPHI1857 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14321904, %bgnloop123 ]
  %vectorPHI1858 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14351903, %bgnloop123 ]
  %vectorPHI1859 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14381902, %bgnloop123 ]
  %vectorPHI1860 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14411901, %bgnloop123 ]
  %vectorPHI1861 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14441900, %bgnloop123 ]
  %vectorPHI1862 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14471920, %bgnloop123 ]
  %vectorPHI1863 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14501950, %bgnloop123 ]
  %vectorPHI1864 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14531952, %bgnloop123 ]
  %vectorPHI1865 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14561954, %bgnloop123 ]
  %vectorPHI1866 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14591956, %bgnloop123 ]
  %vectorPHI1867 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14622168, %bgnloop123 ]
  %vectorPHI1868 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14652166, %bgnloop123 ]
  %vectorPHI1869 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14682164, %bgnloop123 ]
  %vectorPHI1870 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14712162, %bgnloop123 ]
  %vectorPHI1871 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14742157, %bgnloop123 ]
  %vectorPHI1872 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14772155, %bgnloop123 ]
  %vectorPHI1873 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14802153, %bgnloop123 ]
  %vectorPHI1874 = phi <4 x float> [ undef, %ifblock119 ], [ %out_sel14832150, %bgnloop123 ]
  %vectorPHI1875 = phi <4 x i1> [ %vectorPHI1822, %ifblock119 ], [ zeroinitializer, %bgnloop123 ]
  %vectorPHI1876 = phi <4 x float> [ <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %ifblock119 ], [ %172, %bgnloop123 ]
  %vectorPHI1877 = phi <4 x float> [ <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %ifblock119 ], [ %173, %bgnloop123 ]
  %vectorPHI1878 = phi <4 x float> [ <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %ifblock119 ], [ %174, %bgnloop123 ]
  %vectorPHI1879 = phi <4 x float> [ <float 1.000000e+000, float 1.000000e+000, float 1.000000e+000, float 1.000000e+000>, %ifblock119 ], [ %175, %bgnloop123 ]
  %vectorPHI1882 = phi <4 x float> [ <float -2.000000e+000, float -2.000000e+000, float -2.000000e+000, float -2.000000e+000>, %ifblock119 ], [ %merge13582170, %bgnloop123 ]
  %vectorPHI1883 = phi <4 x float> [ <float -2.000000e+000, float -2.000000e+000, float -2.000000e+000, float -2.000000e+000>, %ifblock119 ], [ %merge13602169, %bgnloop123 ]
  %vectorPHI1884 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %vectorPHI1893, %bgnloop123 ]
  %vectorPHI1885 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %vectorPHI1895, %bgnloop123 ]
  %vectorPHI1886 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %vectorPHI1894, %bgnloop123 ]
  %vectorPHI1887 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %merge13682161, %bgnloop123 ]
  %vectorPHI1888 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %merge13702160, %bgnloop123 ]
  %vectorPHI1889 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %merge13722159, %bgnloop123 ]
  %vectorPHI1890 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %merge13742158, %bgnloop123 ]
  %vectorPHI1891 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %merge13762156, %bgnloop123 ]
  %vectorPHI1892 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %merge13782154, %bgnloop123 ]
  %vectorPHI1893 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %merge13802152, %bgnloop123 ]
  %vectorPHI1894 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %merge13822151, %bgnloop123 ]
  %vectorPHI1895 = phi <4 x float> [ <float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000, float 0x36A0000000000000>, %ifblock119 ], [ %merge13842149, %bgnloop123 ]
  %vectorPHI1896 = phi <4 x float> [ %vectorPHI1823, %ifblock119 ], [ %merge13862148, %bgnloop123 ]
  %out_sel14441900 = select <4 x i1> %vectorPHI1875, <4 x float> zeroinitializer, <4 x float> %vectorPHI1861
  %out_sel14411901 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1895, <4 x float> %vectorPHI1860
  %out_sel14381902 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1894, <4 x float> %vectorPHI1859
  %out_sel14351903 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1893, <4 x float> %vectorPHI1858
  %out_sel14321904 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1892, <4 x float> %vectorPHI1857
  %out_sel14291905 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1891, <4 x float> %vectorPHI1856
  %out_sel14261906 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1890, <4 x float> %vectorPHI1855
  %out_sel14231907 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1889, <4 x float> %vectorPHI1854
  %out_sel14201908 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1888, <4 x float> %vectorPHI1853
  %out_sel14171909 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1887, <4 x float> %vectorPHI1852
  %out_sel14141910 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1886, <4 x float> %vectorPHI1851
  %out_sel14111911 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1885, <4 x float> %vectorPHI1850
  %out_sel14081912 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1884, <4 x float> %vectorPHI1849
  %out_sel14051913 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1883, <4 x float> %vectorPHI1848
  %out_sel14021914 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1879, <4 x float> %vectorPHI1847
  %out_sel13991915 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1878, <4 x float> %vectorPHI1846
  %out_sel13961916 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1877, <4 x float> %vectorPHI1845
  %out_sel13931917 = select <4 x i1> %vectorPHI1875, <4 x float> %vectorPHI1876, <4 x float> %vectorPHI1844
  %bitcast1245031918 = bitcast <4 x float> %vectorPHI1896 to <4 x i32>
  %114 = add <4 x i32> %bitcast1245031918, <i32 1, i32 1, i32 1, i32 1>
  %bitcast1255071919 = bitcast <4 x i32> %114 to <4 x float>
  %out_sel14471920 = select <4 x i1> %vectorPHI1875, <4 x float> %bitcast1255071919, <4 x float> %vectorPHI1862
  %ever_left_loop11421924 = or <4 x i1> %vectorPHI1843, %vectorPHI1875
  %extract1990 = extractelement <4 x i1> %ever_left_loop11421924, i32 0
  %extract1991 = extractelement <4 x i1> %ever_left_loop11421924, i32 1
  %extract1992 = extractelement <4 x i1> %ever_left_loop11421924, i32 2
  %extract1993 = extractelement <4 x i1> %ever_left_loop11421924, i32 3
  %loop_mask11441925 = or <4 x i1> %vectorPHI1842, %vectorPHI1875
  %115 = call [4 x <4 x float>] @dx_soa_load_icb_uniform_imm_4_float4_vs(<4 x i1> zeroinitializer, i8* bitcast ([1 x <4 x float>]* @icb to i8*), <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32> zeroinitializer) nounwind
  %bitcast1355321947 = bitcast <4 x float> %vectorPHI1894 to <4 x i32>
  %bitcast1355331948 = bitcast <4 x float> %vectorPHI1892 to <4 x i32>
  %out_sel14501950 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1893, <4 x float> %vectorPHI1863
  %out_sel14531952 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1895, <4 x float> %vectorPHI1864
  %out_sel14561954 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1894, <4 x float> %vectorPHI1865
  %out_sel14591956 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1892, <4 x float> %vectorPHI1866
  %bitcast1406692113 = bitcast <4 x float> %vectorPHI1892 to <4 x i32>
  %bitcast1406712115 = bitcast <4 x float> %vectorPHI1894 to <4 x i32>
  %bitcast1406722116 = bitcast <4 x float> %vectorPHI1895 to <4 x i32>
  %116 = add <4 x i32> %bitcast1406692113, <i32 14, i32 14, i32 14, i32 14>
  %117 = add <4 x i32> %bitcast1406712115, <i32 3, i32 3, i32 3, i32 3>
  %bitcast1416732117 = bitcast <4 x i32> %116 to <4 x float>
  %bitcast1416752119 = bitcast <4 x i32> %117 to <4 x float>
  %bitcast1406702114 = bitcast <4 x float> %vectorPHI1893 to <4 x i32>
  %118 = call [4 x <4 x float>] @dx_soa_load_constant_uniform_imm_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 1, i32 1, i32 1, i32 1>) nounwind
  %119 = extractvalue [4 x <4 x float>] %118, 0
  %120 = extractvalue [4 x <4 x float>] %118, 1
  %121 = extractvalue [4 x <4 x float>] %118, 2
  %122 = extractvalue [4 x <4 x float>] %118, 3
  %123 = icmp ne <4 x i32> %bitcast1406722116, zeroinitializer
  %124 = icmp ne <4 x i32> %bitcast1406722116, zeroinitializer
  %125 = icmp ne <4 x i32> %bitcast1406722116, zeroinitializer
  %126 = icmp ne <4 x i32> %bitcast1406722116, zeroinitializer
  %movcsext7532242 = sext <4 x i1> %123 to <4 x i32>
  %movcsext7542243 = sext <4 x i1> %124 to <4 x i32>
  %movcsext7552244 = sext <4 x i1> %125 to <4 x i32>
  %movcsext7562245 = sext <4 x i1> %126 to <4 x i32>
  %movcbitcast1477572246 = bitcast <4 x float> %119 to <4 x i32>
  %movcbitcast1477582247 = bitcast <4 x float> %120 to <4 x i32>
  %movcbitcast1477592248 = bitcast <4 x float> %121 to <4 x i32>
  %movcbitcast1477602249 = bitcast <4 x float> %122 to <4 x i32>
  %movcand7612250 = and <4 x i32> %116, %movcsext7532242
  %movcand7622251 = and <4 x i32> %bitcast1406702114, %movcsext7542243
  %movcand7632252 = and <4 x i32> %117, %movcsext7552244
  %movcand7642253 = and <4 x i32> %bitcast1406722116, %movcsext7562245
  %movcnot7652254 = xor <4 x i32> %movcsext7532242, <i32 -1, i32 -1, i32 -1, i32 -1>
  %movcnot7662255 = xor <4 x i32> %movcsext7542243, <i32 -1, i32 -1, i32 -1, i32 -1>
  %movcnot7672256 = xor <4 x i32> %movcsext7552244, <i32 -1, i32 -1, i32 -1, i32 -1>
  %movcnot7682257 = xor <4 x i32> %movcsext7562245, <i32 -1, i32 -1, i32 -1, i32 -1>
  %movcand1487692258 = and <4 x i32> %movcbitcast1477572246, %movcnot7652254
  %movcand1487702259 = and <4 x i32> %movcbitcast1477582247, %movcnot7662255
  %movcand1487712260 = and <4 x i32> %movcbitcast1477592248, %movcnot7672256
  %movcand1487722261 = and <4 x i32> %movcbitcast1477602249, %movcnot7682257
  %movcor7732262 = or <4 x i32> %movcand1487692258, %movcand7612250
  %movcor7742263 = or <4 x i32> %movcand1487702259, %movcand7622251
  %movcor7752264 = or <4 x i32> %movcand1487712260, %movcand7632252
  %movcor7762265 = or <4 x i32> %movcand1487722261, %movcand7642253
  %127 = bitcast <4 x i32> %movcor7732262 to <4 x float>
  %128 = bitcast <4 x i32> %movcor7742263 to <4 x float>
  %129 = bitcast <4 x i32> %movcor7752264 to <4 x float>
  %130 = bitcast <4 x i32> %movcor7762265 to <4 x float>
  %131 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 2, i32 2, i32 2, i32 2>) nounwind
  %store.val2282 = insertvalue [4 x <4 x float>] undef, <4 x float> %127, 0
  %store.val2283 = insertvalue [4 x <4 x float>] %store.val2282, <4 x float> %128, 1
  %store.val2284 = insertvalue [4 x <4 x float>] %store.val2283, <4 x float> %129, 2
  %store.val2285 = insertvalue [4 x <4 x float>] %store.val2284, <4 x float> %130, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 2, i32 2, i32 2, i32 2>, [4 x <4 x float>] %store.val2285) nounwind
  %132 = fmul <4 x float> %vectorPHI1876, zeroinitializer
  %133 = fmul <4 x float> %vectorPHI1877, zeroinitializer
  %134 = fmul <4 x float> %vectorPHI1878, zeroinitializer
  %135 = fmul <4 x float> %vectorPHI1879, zeroinitializer
  %136 = fadd <4 x float> %132, %133
  %137 = fadd <4 x float> %136, %134
  %138 = fadd <4 x float> %137, %135
  %139 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 5, i32 5, i32 5, i32 5>) nounwind
  %store.val2290 = insertvalue [4 x <4 x float>] undef, <4 x float> %138, 0
  %store.val2291 = insertvalue [4 x <4 x float>] %store.val2290, <4 x float> %138, 1
  %store.val2292 = insertvalue [4 x <4 x float>] %store.val2291, <4 x float> %138, 2
  %store.val2293 = insertvalue [4 x <4 x float>] %store.val2292, <4 x float> %138, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 5, i32 5, i32 5, i32 5>, [4 x <4 x float>] %store.val2293) nounwind
  %bitcast1527962294 = bitcast <4 x float> %vectorPHI1883 to <4 x i32>
  %140 = icmp eq <4 x i32> %bitcast1527962294, %bitcast1355331948
  %141 = sext <4 x i1> %140 to <4 x i32>
  %bitcast1548002295 = bitcast <4 x i32> %141 to <4 x float>
  %142 = call [4 x <4 x float>] @dx_soa_load_constant_nonuniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> %117) nounwind
  %143 = call [4 x <4 x float>] @dx_soa_load_input_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 4, i32 4, i32 4, i32 4>) nounwind
  %144 = fcmp oge <4 x float> %vectorPHI1883, zeroinitializer
  %145 = sext <4 x i1> %144 to <4 x i32>
  %bitcast1668122296 = bitcast <4 x i32> %145 to <4 x float>
  %146 = add <4 x i32> %bitcast1245031918, <i32 2, i32 2, i32 2, i32 2>
  %bitcast1738172298 = bitcast <4 x i32> %146 to <4 x float>
  %147 = icmp ne <4 x i32> %bitcast1355321947, zeroinitializer
  %148 = icmp ne <4 x i32> %141, zeroinitializer
  %movcsext1838272303 = sext <4 x i1> %147 to <4 x i32>
  %movcsext1838282304 = sext <4 x i1> %148 to <4 x i32>
  %movcbitcast1848312305 = bitcast <4 x float> %vectorPHI1882 to <4 x i32>
  %movcbitcast1848322306 = bitcast <4 x float> %vectorPHI1883 to <4 x i32>
  %movcbitcast1858352307 = bitcast <4 x float> %vectorPHI1876 to <4 x i32>
  %movcbitcast1858362308 = bitcast <4 x float> %vectorPHI1876 to <4 x i32>
  %movcand1868392309 = and <4 x i32> %movcbitcast1848312305, %movcsext1838272303
  %movcand1868402310 = and <4 x i32> %movcbitcast1848322306, %movcsext1838282304
  %movcnot1878432311 = xor <4 x i32> %movcsext1838272303, <i32 -1, i32 -1, i32 -1, i32 -1>
  %movcnot1878442312 = xor <4 x i32> %movcsext1838282304, <i32 -1, i32 -1, i32 -1, i32 -1>
  %movcand1888472313 = and <4 x i32> %movcbitcast1858352307, %movcnot1878432311
  %movcand1888482314 = and <4 x i32> %movcbitcast1858362308, %movcnot1878442312
  %movcor1898512315 = or <4 x i32> %movcand1888472313, %movcand1868392309
  %movcor1898522316 = or <4 x i32> %movcand1888482314, %movcand1868402310
  %149 = bitcast <4 x i32> %movcor1898512315 to <4 x float>
  %150 = bitcast <4 x i32> %movcor1898522316 to <4 x float>
  %merge13202141 = select <4 x i1> zeroinitializer, <4 x float> %bitcast1255071919, <4 x float> %bitcast1738172298
  %merge13222140 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1895, <4 x float> %bitcast1668122296
  %merge13242139 = select <4 x i1> zeroinitializer, <4 x float> %bitcast1416752119, <4 x float> zeroinitializer
  %merge13262138 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1893, <4 x float> zeroinitializer
  %merge13282137 = select <4 x i1> zeroinitializer, <4 x float> %bitcast1416732117, <4 x float> zeroinitializer
  %merge13302136 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1891, <4 x float> zeroinitializer
  %merge13322135 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1890, <4 x float> zeroinitializer
  %merge13342134 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1889, <4 x float> zeroinitializer
  %merge13362133 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1888, <4 x float> zeroinitializer
  %merge13382132 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1892, <4 x float> %bitcast1548002295
  %merge13862148 = select <4 x i1> zeroinitializer, <4 x float> %bitcast1738172298, <4 x float> %merge13202141
  %merge13842149 = select <4 x i1> zeroinitializer, <4 x float> %bitcast1668122296, <4 x float> %merge13222140
  %out_sel14832150 = select <4 x i1> zeroinitializer, <4 x float> %merge13842149, <4 x float> %vectorPHI1874
  %merge13822151 = select <4 x i1> zeroinitializer, <4 x float> zeroinitializer, <4 x float> %merge13242139
  %merge13802152 = select <4 x i1> zeroinitializer, <4 x float> zeroinitializer, <4 x float> %merge13262138
  %out_sel14802153 = select <4 x i1> zeroinitializer, <4 x float> %merge13802152, <4 x float> %vectorPHI1873
  %merge13782154 = select <4 x i1> zeroinitializer, <4 x float> zeroinitializer, <4 x float> %merge13282137
  %out_sel14772155 = select <4 x i1> zeroinitializer, <4 x float> %merge13782154, <4 x float> %vectorPHI1872
  %merge13762156 = select <4 x i1> zeroinitializer, <4 x float> <float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000>, <4 x float> %merge13302136
  %out_sel14742157 = select <4 x i1> zeroinitializer, <4 x float> %merge13762156, <4 x float> %vectorPHI1871
  %merge13742158 = select <4 x i1> zeroinitializer, <4 x float> <float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000>, <4 x float> %merge13322135
  %merge13722159 = select <4 x i1> zeroinitializer, <4 x float> <float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000>, <4 x float> %merge13342134
  %merge13702160 = select <4 x i1> zeroinitializer, <4 x float> <float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000>, <4 x float> %merge13362133
  %merge13682161 = select <4 x i1> zeroinitializer, <4 x float> %bitcast1548002295, <4 x float> %merge13382132
  %out_sel14712162 = select <4 x i1> zeroinitializer, <4 x float> %merge13682161, <4 x float> %vectorPHI1870
  %out_sel14682164 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1894, <4 x float> %vectorPHI1869
  %out_sel14652166 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1895, <4 x float> %vectorPHI1868
  %out_sel14622168 = select <4 x i1> zeroinitializer, <4 x float> %vectorPHI1893, <4 x float> %vectorPHI1867
  %merge13602169 = select <4 x i1> zeroinitializer, <4 x float> %150, <4 x float> %vectorPHI1883
  %merge13582170 = select <4 x i1> zeroinitializer, <4 x float> %149, <4 x float> %vectorPHI1882
  %bitcastindex1937002173 = bitcast <4 x float> %merge13682161 to <4 x i32>
  %151 = add <4 x i32> %bitcastindex1937002173, <i32 2, i32 2, i32 2, i32 2>
  %152 = call [4 x <4 x float>] @dx_soa_load_constant_nonuniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> <i32 5, i32 5, i32 5, i32 5>, <4 x i32> %151) nounwind
  %153 = extractvalue [4 x <4 x float>] %152, 1
  %154 = fcmp ole <4 x float> %153, zeroinitializer
  %155 = fcmp ole <4 x float> %153, zeroinitializer
  %156 = fcmp ole <4 x float> %153, zeroinitializer
  %157 = fcmp ole <4 x float> %153, zeroinitializer
  %158 = sext <4 x i1> %154 to <4 x i32>
  %159 = sext <4 x i1> %155 to <4 x i32>
  %160 = sext <4 x i1> %156 to <4 x i32>
  %161 = sext <4 x i1> %157 to <4 x i32>
  %162 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 2, i32 2, i32 2, i32 2>) nounwind
  %bitcast1967092174 = bitcast <4 x i32> %158 to <4 x float>
  %bitcast1967102175 = bitcast <4 x i32> %159 to <4 x float>
  %bitcast1967112176 = bitcast <4 x i32> %160 to <4 x float>
  %bitcast1967122177 = bitcast <4 x i32> %161 to <4 x float>
  %store.val2194 = insertvalue [4 x <4 x float>] undef, <4 x float> %bitcast1967092174, 0
  %store.val2195 = insertvalue [4 x <4 x float>] %store.val2194, <4 x float> %bitcast1967102175, 1
  %store.val2196 = insertvalue [4 x <4 x float>] %store.val2195, <4 x float> %bitcast1967112176, 2
  %store.val2197 = insertvalue [4 x <4 x float>] %store.val2196, <4 x float> %bitcast1967122177, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 2, i32 2, i32 2, i32 2>, [4 x <4 x float>] %store.val2197) nounwind
  %163 = call [4 x <4 x float>] @dx_soa_load_input_nonuniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  %164 = extractvalue [4 x <4 x float>] %163, 0
  %165 = extractvalue [4 x <4 x float>] %163, 1
  %166 = extractvalue [4 x <4 x float>] %163, 2
  %167 = extractvalue [4 x <4 x float>] %163, 3
  %bitcast1997292206 = bitcast <4 x float> %merge13702160 to <4 x i32>
  %bitcast1997302207 = bitcast <4 x float> %merge13702160 to <4 x i32>
  %bitcast1997312208 = bitcast <4 x float> %merge13702160 to <4 x i32>
  %bitcast1997322209 = bitcast <4 x float> %merge13702160 to <4 x i32>
  %168 = icmp ne <4 x i32> %bitcast1997292206, zeroinitializer
  %169 = icmp ne <4 x i32> %bitcast1997302207, zeroinitializer
  %170 = icmp ne <4 x i32> %bitcast1997312208, zeroinitializer
  %171 = icmp ne <4 x i32> %bitcast1997322209, zeroinitializer
  %movcsext2007332210 = sext <4 x i1> %168 to <4 x i32>
  %movcsext2007342211 = sext <4 x i1> %169 to <4 x i32>
  %movcsext2007352212 = sext <4 x i1> %170 to <4 x i32>
  %movcsext2007362213 = sext <4 x i1> %171 to <4 x i32>
  %movcbitcast2027372214 = bitcast <4 x float> %164 to <4 x i32>
  %movcbitcast2027382215 = bitcast <4 x float> %165 to <4 x i32>
  %movcbitcast2027392216 = bitcast <4 x float> %166 to <4 x i32>
  %movcbitcast2027402217 = bitcast <4 x float> %167 to <4 x i32>
  %movcnot2047412218 = xor <4 x i32> %movcsext2007332210, <i32 -1, i32 -1, i32 -1, i32 -1>
  %movcnot2047422219 = xor <4 x i32> %movcsext2007342211, <i32 -1, i32 -1, i32 -1, i32 -1>
  %movcnot2047432220 = xor <4 x i32> %movcsext2007352212, <i32 -1, i32 -1, i32 -1, i32 -1>
  %movcnot2047442221 = xor <4 x i32> %movcsext2007362213, <i32 -1, i32 -1, i32 -1, i32 -1>
  %movcand2057452222 = and <4 x i32> %movcbitcast2027372214, %movcnot2047412218
  %movcand2057462223 = and <4 x i32> %movcbitcast2027382215, %movcnot2047422219
  %movcand2057472224 = and <4 x i32> %movcbitcast2027392216, %movcnot2047432220
  %movcand2057482225 = and <4 x i32> %movcbitcast2027402217, %movcnot2047442221
  %172 = bitcast <4 x i32> %movcand2057452222 to <4 x float>
  %173 = bitcast <4 x i32> %movcand2057462223 to <4 x float>
  %174 = bitcast <4 x i32> %movcand2057472224 to <4 x float>
  %175 = bitcast <4 x i32> %movcand2057482225 to <4 x float>
  %176 = call i1 @allOne_v4(<4 x i1> %loop_mask11441925)
  br i1 %176, label %ifblock128, label %bgnloop123

ifblock128:                                       ; preds = %bgnloop123
  %bitcast2945411964 = bitcast <4 x float> %out_sel14411901 to <4 x i32>
  %bitcast2955451965 = bitcast <4 x float> %out_sel14171909 to <4 x i32>
  %177 = and <4 x i32> %bitcast2955451965, <i32 31, i32 31, i32 31, i32 31>
  %178 = ashr <4 x i32> %bitcast2945411964, %177
  %179 = call [4 x <4 x float>] @dx_soa_load_constant_uniform_imm_4_float4_vs(<4 x i1> %ever_left_loop11421924, i8* %gc, <4 x i32> <i32 2, i32 2, i32 2, i32 2>, <4 x i32> zeroinitializer) nounwind
  %bitcast3015531969 = bitcast <4 x float> %out_sel14171909 to <4 x i32>
  %180 = mul <4 x i32> %bitcast3015531969, %178
  %181 = add <4 x i32> %180, <i32 4, i32 4, i32 4, i32 4>
  %bitcast3045571973 = bitcast <4 x i32> %181 to <4 x float>
  %182 = shufflevector <4 x float> <float 0x36D6000000000000, float 0x36CC000000000000, float 0x36D4000000000000, float undef>, <4 x float> %bitcast3045571973, <4 x i32> <i32 0, i32 1, i32 2, i32 5>
  %183 = shufflevector <4 x float> <float 0x36D6000000000000, float 0x36CC000000000000, float 0x36D4000000000000, float undef>, <4 x float> %bitcast3045571973, <4 x i32> <i32 0, i32 1, i32 2, i32 6>
  %184 = shufflevector <4 x float> <float 0x36D6000000000000, float 0x36CC000000000000, float 0x36D4000000000000, float undef>, <4 x float> %bitcast3045571973, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %185 = getelementptr inbounds [1024 x <4 x float>]* %1, i32 0, i32 1
  %186 = getelementptr inbounds [1024 x <4 x float>]* %2, i32 0, i32 1
  %187 = getelementptr inbounds [1024 x <4 x float>]* %3, i32 0, i32 1
  br i1 %extract1990, label %deload2580, label %postload2581

deload2580:                                       ; preds = %ifblock128
  %188 = getelementptr inbounds [1024 x <4 x float>]* %0, i32 0, i32 1
  %189 = shufflevector <4 x float> <float 0x36D6000000000000, float 0x36CC000000000000, float 0x36D4000000000000, float undef>, <4 x float> %bitcast3045571973, <4 x i32> <i32 0, i32 1, i32 2, i32 4>
  store <4 x float> %189, <4 x float>* %188
  br label %postload2581

postload2581:                                     ; preds = %ifblock128, %deload2580
  br i1 %extract1991, label %deload2582, label %postload2583

deload2582:                                       ; preds = %postload2581
  store <4 x float> %182, <4 x float>* %185
  br label %postload2583

postload2583:                                     ; preds = %postload2581, %deload2582
  br i1 %extract1992, label %deload2584, label %postload2585

deload2584:                                       ; preds = %postload2583
  store <4 x float> %183, <4 x float>* %186
  br label %postload2585

postload2585:                                     ; preds = %postload2583, %deload2584
  br i1 %extract1993, label %deload2586, label %postload2587

deload2586:                                       ; preds = %postload2585
  store <4 x float> %184, <4 x float>* %187
  br label %postload2587

postload2587:                                     ; preds = %postload2585, %deload2586
  %190 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> %ever_left_loop11421924, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 2, i32 2, i32 2, i32 2>) nounwind
  %store.val1994 = insertvalue [4 x <4 x float>] undef, <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, 0
  %store.val1995 = insertvalue [4 x <4 x float>] %store.val1994, <4 x float> <float 2.000000e+000, float 2.000000e+000, float 2.000000e+000, float 2.000000e+000>, 1
  %store.val1996 = insertvalue [4 x <4 x float>] %store.val1995, <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, 2
  %store.val1997 = insertvalue [4 x <4 x float>] %store.val1996, <4 x float> <float -2.000000e+000, float -2.000000e+000, float -2.000000e+000, float -2.000000e+000>, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> %ever_left_loop11421924, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 2, i32 2, i32 2, i32 2>, [4 x <4 x float>] %store.val1997) nounwind
  %191 = call [4 x <4 x float>] @dx_soa_load_constant_uniform_imm_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> <i32 1, i32 1, i32 1, i32 1>, <4 x i32> <i32 3, i32 3, i32 3, i32 3>) nounwind
  %192 = call [4 x <4 x float>] @dx_soa_load_constant_uniform_imm_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 5, i32 5, i32 5, i32 5>) nounwind
  %193 = extractvalue [4 x <4 x float>] %192, 1
  %bitcast2188812342 = bitcast <4 x float> %out_sel14712162 to <4 x i32>
  %bitcast2198852343 = bitcast <4 x float> %193 to <4 x i32>
  %194 = icmp ult <4 x i32> %bitcast2188812342, %bitcast2198852343
  %195 = sext <4 x i1> %194 to <4 x i32>
  %196 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 2, i32 2, i32 2, i32 2>) nounwind
  %197 = extractvalue [4 x <4 x float>] %196, 1
  %198 = extractvalue [4 x <4 x float>] %196, 2
  %199 = extractvalue [4 x <4 x float>] %196, 3
  %store.val2356 = insertvalue [4 x <4 x float>] undef, <4 x float> zeroinitializer, 0
  %store.val2357 = insertvalue [4 x <4 x float>] %store.val2356, <4 x float> %197, 1
  %store.val2358 = insertvalue [4 x <4 x float>] %store.val2357, <4 x float> %198, 2
  %store.val2359 = insertvalue [4 x <4 x float>] %store.val2358, <4 x float> %199, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 2, i32 2, i32 2, i32 2>, [4 x <4 x float>] %store.val2359) nounwind
  %200 = call [4 x <4 x float>] @dx_soa_load_constant_uniform_imm_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> <i32 5, i32 5, i32 5, i32 5>, <4 x i32> zeroinitializer) nounwind
  %201 = extractvalue [4 x <4 x float>] %200, 0
  %202 = extractvalue [4 x <4 x float>] %200, 2
  %203 = fmul <4 x float> %201, zeroinitializer
  %204 = fmul <4 x float> %201, zeroinitializer
  %205 = fmul <4 x float> %202, zeroinitializer
  %206 = fmul <4 x float> %201, zeroinitializer
  %207 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 6, i32 6, i32 6, i32 6>) nounwind
  %store.val2376 = insertvalue [4 x <4 x float>] undef, <4 x float> %203, 0
  %store.val2377 = insertvalue [4 x <4 x float>] %store.val2376, <4 x float> %204, 1
  %store.val2378 = insertvalue [4 x <4 x float>] %store.val2377, <4 x float> %205, 2
  %store.val2379 = insertvalue [4 x <4 x float>] %store.val2378, <4 x float> %206, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 6, i32 6, i32 6, i32 6>, [4 x <4 x float>] %store.val2379) nounwind
  %208 = icmp ne <4 x i32> %195, zeroinitializer
  %movcsext2269092380 = sext <4 x i1> %208 to <4 x i32>
  %movcbitcast2289132381 = bitcast <4 x float> %out_sel14772155 to <4 x i32>
  %movcnot2309172382 = xor <4 x i32> %movcsext2269092380, <i32 -1, i32 -1, i32 -1, i32 -1>
  %movcand2319212383 = and <4 x i32> %movcbitcast2289132381, %movcnot2309172382
  %movcor2329252384 = or <4 x i32> %movcand2319212383, %movcsext2269092380
  %209 = bitcast <4 x i32> %movcor2329252384 to <4 x float>
  %210 = call [4 x <4 x float>] @dx_soa_load_input_uniform_indirect_4_float4_vs(<4 x i1> zeroinitializer, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 3, i32 3, i32 3, i32 3>) nounwind
  %211 = call i1 @allZero_v4(<4 x i1> zeroinitializer)
  br i1 %211, label %ifblock240, label %bgnloop235

bgnloop235:                                       ; preds = %postload2587, %bgnloop235
  %vectorPHI2386 = phi <4 x i1> [ zeroinitializer, %postload2587 ], [ %ever_left_loop11802395, %bgnloop235 ]
  %vectorPHI2387 = phi <4 x i1> [ <i1 true, i1 true, i1 true, i1 true>, %postload2587 ], [ %loop_mask11822396, %bgnloop235 ]
  %vectorPHI2388 = phi <4 x i1> [ zeroinitializer, %postload2587 ], [ %local_edge11872398, %bgnloop235 ]
  %vectorPHI2389 = phi <4 x float> [ %out_sel14471920, %postload2587 ], [ %bitcast2379412391, %bgnloop235 ]
  %bitcast2369372390 = bitcast <4 x float> %vectorPHI2389 to <4 x i32>
  %212 = add <4 x i32> %bitcast2369372390, <i32 1, i32 1, i32 1, i32 1>
  %bitcast2379412391 = bitcast <4 x i32> %212 to <4 x float>
  %bitcast2382392 = bitcast <4 x float> %out_sel14441900 to <4 x i32>
  %213 = icmp eq <4 x i32> %bitcast2382392, zeroinitializer
  %notCond11782393 = xor <4 x i1> %213, <i1 true, i1 true, i1 true, i1 true>
  %who_left_tr11792394 = and <4 x i1> %vectorPHI2388, %213
  %ever_left_loop11802395 = or <4 x i1> %vectorPHI2386, %who_left_tr11792394
  %loop_mask11822396 = or <4 x i1> %vectorPHI2387, %who_left_tr11792394
  %curr_loop_mask11842397 = or <4 x i1> %loop_mask11822396, %who_left_tr11792394
  %214 = call i1 @allOne_v4(<4 x i1> %curr_loop_mask11842397)
  %local_edge11872398 = and <4 x i1> %vectorPHI2388, %notCond11782393
  br i1 %214, label %ifblock240, label %bgnloop235

ifblock240:                                       ; preds = %bgnloop235, %postload2587
  %vectorPHI2399 = phi <4 x i1> [ zeroinitializer, %postload2587 ], [ %ever_left_loop11802395, %bgnloop235 ]
  %bitcast2429452400 = bitcast <4 x float> %out_sel14201908 to <4 x i32>
  %bitcast2429472401 = bitcast <4 x float> %out_sel14261906 to <4 x i32>
  %bitcast2429482402 = bitcast <4 x float> %out_sel14291905 to <4 x i32>
  %bitcast2439492403 = bitcast <4 x float> %out_sel14051913 to <4 x i32>
  %bitcast2439512404 = bitcast <4 x float> %out_sel14051913 to <4 x i32>
  %bitcast2439522405 = bitcast <4 x float> %out_sel14051913 to <4 x i32>
  %215 = icmp eq <4 x i32> %bitcast2429452400, %bitcast2439492403
  %216 = icmp eq <4 x i32> %bitcast2429472401, %bitcast2439512404
  %217 = icmp eq <4 x i32> %bitcast2429482402, %bitcast2439522405
  %218 = sext <4 x i1> %215 to <4 x i32>
  %219 = sext <4 x i1> %216 to <4 x i32>
  %220 = sext <4 x i1> %217 to <4 x i32>
  %221 = icmp eq <4 x i32> %219, zeroinitializer
  %ifblock240_to_ifblock2472408 = and <4 x i1> %vectorPHI2399, %221
  %extract2429 = extractelement <4 x i1> %ifblock240_to_ifblock2472408, i32 1
  %extract2430 = extractelement <4 x i1> %ifblock240_to_ifblock2472408, i32 2
  %extract2431 = extractelement <4 x i1> %ifblock240_to_ifblock2472408, i32 3
  %extract2428 = extractelement <4 x i1> %ifblock240_to_ifblock2472408, i32 0
  %bitcast2489592409 = bitcast <4 x float> %out_sel14381902 to <4 x i32>
  %bitcast2499632410 = bitcast <4 x float> %out_sel14261906 to <4 x i32>
  %222 = icmp uge <4 x i32> %bitcast2489592409, %bitcast2499632410
  %223 = sext <4 x i1> %222 to <4 x i32>
  %bitcast2509672411 = bitcast <4 x i32> %223 to <4 x float>
  %224 = fadd <4 x float> %out_sel13961916, zeroinitializer
  %225 = fadd <4 x float> %out_sel13961916, zeroinitializer
  %226 = fadd <4 x float> %out_sel13961916, zeroinitializer
  %227 = fadd <4 x float> %out_sel13961916, zeroinitializer
  %228 = shufflevector <4 x float> %224, <4 x float> %225, <4 x i32> <i32 1, i32 5, i32 undef, i32 undef>
  %229 = shufflevector <4 x float> %224, <4 x float> %225, <4 x i32> <i32 2, i32 6, i32 undef, i32 undef>
  %230 = shufflevector <4 x float> %224, <4 x float> %225, <4 x i32> <i32 3, i32 7, i32 undef, i32 undef>
  %231 = shufflevector <4 x float> %228, <4 x float> %226, <4 x i32> <i32 0, i32 1, i32 5, i32 undef>
  %232 = shufflevector <4 x float> %229, <4 x float> %226, <4 x i32> <i32 0, i32 1, i32 6, i32 undef>
  %233 = shufflevector <4 x float> %230, <4 x float> %226, <4 x i32> <i32 0, i32 1, i32 7, i32 undef>
  %234 = shufflevector <4 x float> %231, <4 x float> %227, <4 x i32> <i32 0, i32 1, i32 2, i32 5>
  %235 = shufflevector <4 x float> %232, <4 x float> %227, <4 x i32> <i32 0, i32 1, i32 2, i32 6>
  %236 = shufflevector <4 x float> %233, <4 x float> %227, <4 x i32> <i32 0, i32 1, i32 2, i32 7>
  %237 = getelementptr inbounds [512 x <4 x float>]* %13, i32 0, i32 4
  %238 = getelementptr inbounds [512 x <4 x float>]* %14, i32 0, i32 4
  %239 = getelementptr inbounds [512 x <4 x float>]* %15, i32 0, i32 4
  br i1 %extract2428, label %deload2608, label %postload2609

deload2608:                                       ; preds = %ifblock240
  %240 = getelementptr inbounds [512 x <4 x float>]* %12, i32 0, i32 4
  %241 = shufflevector <4 x float> %224, <4 x float> %225, <4 x i32> <i32 0, i32 4, i32 undef, i32 undef>
  %242 = shufflevector <4 x float> %241, <4 x float> %226, <4 x i32> <i32 0, i32 1, i32 4, i32 undef>
  %243 = shufflevector <4 x float> %242, <4 x float> %227, <4 x i32> <i32 0, i32 1, i32 2, i32 4>
  store <4 x float> %243, <4 x float>* %240
  br label %postload2609

postload2609:                                     ; preds = %ifblock240, %deload2608
  br i1 %extract2429, label %deload2610, label %postload2611

deload2610:                                       ; preds = %postload2609
  store <4 x float> %234, <4 x float>* %237
  br label %postload2611

postload2611:                                     ; preds = %postload2609, %deload2610
  br i1 %extract2430, label %deload2612, label %postload2613

deload2612:                                       ; preds = %postload2611
  store <4 x float> %235, <4 x float>* %238
  br label %postload2613

postload2613:                                     ; preds = %postload2611, %deload2612
  br i1 %extract2431, label %deload2614, label %postload2615

deload2614:                                       ; preds = %postload2613
  store <4 x float> %236, <4 x float>* %239
  br label %postload2615

postload2615:                                     ; preds = %postload2613, %deload2614
  %244 = icmp ne <4 x i1> %216, %222
  %245 = sext <4 x i1> %244 to <4 x i32>
  %246 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 4, i32 4, i32 4, i32 4>) nounwind
  %247 = extractvalue [4 x <4 x float>] %246, 0
  %248 = extractvalue [4 x <4 x float>] %246, 1
  %249 = extractvalue [4 x <4 x float>] %246, 3
  %bitcast2569832433 = bitcast <4 x i32> %245 to <4 x float>
  %store.val2450 = insertvalue [4 x <4 x float>] undef, <4 x float> %247, 0
  %store.val2451 = insertvalue [4 x <4 x float>] %store.val2450, <4 x float> %248, 1
  %store.val2452 = insertvalue [4 x <4 x float>] %store.val2451, <4 x float> %bitcast2569832433, 2
  %store.val2453 = insertvalue [4 x <4 x float>] %store.val2452, <4 x float> %249, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 4, i32 4, i32 4, i32 4>, [4 x <4 x float>] %store.val2453) nounwind
  %bitcast2572454 = bitcast <4 x float> %out_sel14291905 to <4 x i32>
  %250 = icmp eq <4 x i32> %bitcast2572454, zeroinitializer
  %ifblock247_to_ifblock2592455 = and <4 x i1> %ifblock240_to_ifblock2472408, %250
  %251 = call [4 x <4 x float>] @dx_soa_load_input_uniform_indirect_4_float4_vs(<4 x i1> %ifblock247_to_ifblock2592455, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 2, i32 2, i32 2, i32 2>) nounwind
  %252 = extractvalue [4 x <4 x float>] %251, 1
  %253 = extractvalue [4 x <4 x float>] %251, 2
  %254 = fmul <4 x float> %252, %out_sel13991915
  %255 = fmul <4 x float> %253, %out_sel13991915
  %256 = fmul <4 x float> %252, %out_sel13991915
  %257 = fmul <4 x float> %253, %out_sel13991915
  %258 = fadd <4 x float> %254, %255
  %259 = fadd <4 x float> %258, %256
  %260 = fadd <4 x float> %259, %257
  %261 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock247_to_ifblock2592455, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 7, i32 7, i32 7, i32 7>) nounwind
  %store.val2464 = insertvalue [4 x <4 x float>] undef, <4 x float> %260, 0
  %store.val2465 = insertvalue [4 x <4 x float>] %store.val2464, <4 x float> %260, 1
  %store.val2466 = insertvalue [4 x <4 x float>] %store.val2465, <4 x float> %260, 2
  %store.val2467 = insertvalue [4 x <4 x float>] %store.val2466, <4 x float> %260, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock247_to_ifblock2592455, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 7, i32 7, i32 7, i32 7>, [4 x <4 x float>] %store.val2467) nounwind
  %262 = mul <4 x i32> %220, %220
  %263 = mul <4 x i32> %220, %218
  %bitcast2679922457 = bitcast <4 x float> %out_sel14291905 to <4 x i32>
  %264 = add <4 x i32> %262, %bitcast2679922457
  %265 = add <4 x i32> %263, %223
  %bitcast2689962459 = bitcast <4 x i32> %264 to <4 x float>
  %bitcast2689932458 = bitcast <4 x i32> %265 to <4 x float>
  %merge13082000 = select <4 x i1> zeroinitializer, <4 x float> %out_sel14802153, <4 x float> %out_sel14351903
  %merge13042001 = select <4 x i1> zeroinitializer, <4 x float> <float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000>, <4 x float> %bitcast2689962459
  %merge13022002 = select <4 x i1> zeroinitializer, <4 x float> <float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000>, <4 x float> %bitcast2509672411
  %merge13002003 = select <4 x i1> zeroinitializer, <4 x float> <float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000>, <4 x float> %out_sel14231907
  %merge12982004 = select <4 x i1> zeroinitializer, <4 x float> <float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000, float 0xFFFFFFFFE0000000>, <4 x float> %bitcast2689932458
  %merge12942005 = select <4 x i1> zeroinitializer, <4 x float> %out_sel14682164, <4 x float> %out_sel14561954
  %merge12862006 = select <4 x i1> zeroinitializer, <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float> %out_sel13991915
  %merge12822007 = select <4 x i1> zeroinitializer, <4 x float> <float -1.000000e+000, float -1.000000e+000, float -1.000000e+000, float -1.000000e+000>, <4 x float> %out_sel13931917
  %merge2008 = select <4 x i1> zeroinitializer, <4 x float> %209, <4 x float> zeroinitializer
  %266 = call [4 x <4 x float>] @dx_soa_load_input_uniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> zeroinitializer) nounwind
  %bitcast2785932009 = bitcast <4 x float> %merge12942005 to <4 x i32>
  %bitcast2785942010 = bitcast <4 x float> %merge12942005 to <4 x i32>
  %bitcast2785952011 = bitcast <4 x float> %merge12942005 to <4 x i32>
  %bitcast2785962012 = bitcast <4 x float> %merge12942005 to <4 x i32>
  %bitcast2795972013 = bitcast <4 x float> %merge12982004 to <4 x i32>
  %bitcast2795982014 = bitcast <4 x float> %merge13002003 to <4 x i32>
  %bitcast2795992015 = bitcast <4 x float> %merge13022002 to <4 x i32>
  %bitcast2796002016 = bitcast <4 x float> %merge13042001 to <4 x i32>
  %267 = and <4 x i32> %bitcast2785932009, %bitcast2795972013
  %268 = and <4 x i32> %bitcast2785942010, %bitcast2795982014
  %269 = and <4 x i32> %bitcast2785952011, %bitcast2795992015
  %270 = and <4 x i32> %bitcast2785962012, %bitcast2796002016
  %271 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 2, i32 2, i32 2, i32 2>) nounwind
  %bitcast2816052017 = bitcast <4 x i32> %267 to <4 x float>
  %bitcast2816062018 = bitcast <4 x i32> %268 to <4 x float>
  %bitcast2816072019 = bitcast <4 x i32> %269 to <4 x float>
  %bitcast2816082020 = bitcast <4 x i32> %270 to <4 x float>
  %store.val2037 = insertvalue [4 x <4 x float>] undef, <4 x float> %bitcast2816052017, 0
  %store.val2038 = insertvalue [4 x <4 x float>] %store.val2037, <4 x float> %bitcast2816062018, 1
  %store.val2039 = insertvalue [4 x <4 x float>] %store.val2038, <4 x float> %bitcast2816072019, 2
  %store.val2040 = insertvalue [4 x <4 x float>] %store.val2039, <4 x float> %bitcast2816082020, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 2, i32 2, i32 2, i32 2>, [4 x <4 x float>] %store.val2040) nounwind
  %bitcastindex2826162041 = bitcast <4 x float> %merge13042001 to <4 x i32>
  %272 = call [4 x <4 x float>] @dx_soa_load_constant_nonuniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> <i32 2, i32 2, i32 2, i32 2>, <4 x i32> %bitcastindex2826162041) nounwind
  %273 = extractvalue [4 x <4 x float>] %272, 0
  %274 = extractvalue [4 x <4 x float>] %272, 1
  %275 = extractvalue [4 x <4 x float>] %272, 2
  %276 = extractvalue [4 x <4 x float>] %272, 3
  %bitcast2846212042 = bitcast <4 x float> %273 to <4 x i32>
  %bitcast2846222043 = bitcast <4 x float> %274 to <4 x i32>
  %bitcast2846232044 = bitcast <4 x float> %275 to <4 x i32>
  %bitcast2846242045 = bitcast <4 x float> %276 to <4 x i32>
  %277 = sitofp <4 x i32> %bitcast2846212042 to <4 x float>
  %278 = sitofp <4 x i32> %bitcast2846222043 to <4 x float>
  %279 = sitofp <4 x i32> %bitcast2846232044 to <4 x float>
  %280 = sitofp <4 x i32> %bitcast2846242045 to <4 x float>
  %281 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 6, i32 6, i32 6, i32 6>) nounwind
  %store.val2062 = insertvalue [4 x <4 x float>] undef, <4 x float> %277, 0
  %store.val2063 = insertvalue [4 x <4 x float>] %store.val2062, <4 x float> %278, 1
  %store.val2064 = insertvalue [4 x <4 x float>] %store.val2063, <4 x float> %279, 2
  %store.val2065 = insertvalue [4 x <4 x float>] %store.val2064, <4 x float> %280, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 6, i32 6, i32 6, i32 6>, [4 x <4 x float>] %store.val2065) nounwind
  %282 = fmul <4 x float> %merge2008, %merge12822007
  %283 = fmul <4 x float> %merge2008, %merge12862006
  %284 = fmul <4 x float> %merge2008, %merge12862006
  %285 = fadd <4 x float> %282, %283
  %286 = fadd <4 x float> %285, %284
  %287 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 5, i32 5, i32 5, i32 5>) nounwind
  %288 = extractvalue [4 x <4 x float>] %287, 0
  %289 = extractvalue [4 x <4 x float>] %287, 1
  %290 = extractvalue [4 x <4 x float>] %287, 3
  %store.val2082 = insertvalue [4 x <4 x float>] undef, <4 x float> %288, 0
  %store.val2083 = insertvalue [4 x <4 x float>] %store.val2082, <4 x float> %289, 1
  %store.val2084 = insertvalue [4 x <4 x float>] %store.val2083, <4 x float> %286, 2
  %store.val2085 = insertvalue [4 x <4 x float>] %store.val2084, <4 x float> %290, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 5, i32 5, i32 5, i32 5>, [4 x <4 x float>] %store.val2085) nounwind
  %bitcastindex2876422086 = bitcast <4 x float> %merge13082000 to <4 x i32>
  %291 = call [4 x <4 x float>] @dx_soa_load_constant_nonuniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> %bitcastindex2876422086) nounwind
  %292 = extractvalue [4 x <4 x float>] %291, 1
  %bitcast2896502087 = bitcast <4 x float> %merge13002003 to <4 x i32>
  %bitcast2896522088 = bitcast <4 x float> %merge13042001 to <4 x i32>
  %bitcast2906542089 = bitcast <4 x float> %292 to <4 x i32>
  %bitcast2906562090 = bitcast <4 x float> %292 to <4 x i32>
  %293 = add <4 x i32> %bitcast2896502087, %bitcast2906542089
  %294 = add <4 x i32> %bitcast2896522088, %bitcast2906562090
  %295 = call [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 4, i32 4, i32 4, i32 4>) nounwind
  %296 = extractvalue [4 x <4 x float>] %295, 0
  %297 = extractvalue [4 x <4 x float>] %295, 2
  %bitcast2926622091 = bitcast <4 x i32> %293 to <4 x float>
  %bitcast2926642092 = bitcast <4 x i32> %294 to <4 x float>
  %store.val2109 = insertvalue [4 x <4 x float>] undef, <4 x float> %296, 0
  %store.val2110 = insertvalue [4 x <4 x float>] %store.val2109, <4 x float> %bitcast2926622091, 1
  %store.val2111 = insertvalue [4 x <4 x float>] %store.val2110, <4 x float> %297, 2
  %store.val2112 = insertvalue [4 x <4 x float>] %store.val2111, <4 x float> %bitcast2926642092, 3
  call void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1> %ifblock240_to_ifblock2472408, i8* %gc, <4 x i32> zeroinitializer, <4 x i32> <i32 4, i32 4, i32 4, i32 4>, [4 x <4 x float>] %store.val2112) nounwind
  ret void
}


declare [4 x <4 x float>] @dx_soa_load_output_uniform_indirect_4_float4_vs(<4 x i1>, i8*, <4 x i32>, <4 x i32>)

declare void @dx_soa_store_output_uniform_indirect_4_float4_vs(<4 x i1>, i8*, <4 x i32>, <4 x i32>, [4 x <4 x float>])

declare [4 x <4 x float>] @dx_soa_load_icb_uniform_imm_4_float4_vs(<4 x i1>, i8*, <4 x i32>, <4 x i32>)

declare [4 x <4 x float>] @dx_soa_load_constant_uniform_imm_4_float4_vs(<4 x i1>, i8*, <4 x i32>, <4 x i32>)

declare [4 x <4 x float>] @dx_soa_load_constant_nonuniform_indirect_4_float4_vs(<4 x i1>, i8*, <4 x i32>, <4 x i32>)

declare [4 x <4 x float>] @dx_soa_load_input_uniform_indirect_4_float4_vs(<4 x i1>, i8*, <4 x i32>, <4 x i32>)

declare [4 x <4 x float>] @dx_soa_load_input_nonuniform_indirect_4_float4_vs(<4 x i1>, i8*, <4 x i32>, <4 x i32>)

declare i1 @allZero_v4(<4 x i1>)
declare i1 @allOne_v4(<4 x i1>)
;;LLVMIR end 
