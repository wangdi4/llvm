; Test the case of a large region. If two store refs (%t12)[0][i2] are in different
; parent loops, we need to check whether the store ref (%t12)[0][i2]'s parent loop's
; top sort number is larger than load ref (%t12)[0][i3]. If not, the store ref cannot
; be the elimination candidate.
;
; RUN: opt -hir-ssa-deconstruction -hir-dead-store-elimination -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir-framework>,hir-dead-store-elimination,print<hir-framework>" 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Dead Store Elimination (hir-dead-store-elimination) ***
;Function: crout_.19|_.250
;
;<0>          BEGIN REGION { }
;<177>              + DO i1 = 0, 248, 1   <DO_LOOP>
;<178>              |   + DO i2 = 0, -1 * i1 + 248, 1   <DO_LOOP>  <MAX_TC_EST = 249>
;<7>                |   |   %t521 = @llvm.smax.i64(i2 + 1,  2);
;<11>               |   |   %t525 = (%t15)[i1 + i2 + 1][i1];
;<14>               |   |   %t545 = 0.000000e+00;
;<15>               |   |   if (i1 + i2 + 2 != i1 + 2)
;<15>               |   |   {
;<19>               |   |      %t530 = 0.000000e+00;
;<179>              |   |
;<179>              |   |      + DO i3 = 0, %t521 + -2, 1   <DO_LOOP>  <MAX_TC_EST = 125>
;<25>               |   |      |   %t534 = (%t12)[0][i3];
;<27>               |   |      |   %t536 = (%t15)[i1 + i2 + 1][i1 + i3 + 1];
;<28>               |   |      |   %t537 = %t536  *  %t534;
;<29>               |   |      |   %t530 = %t537  +  %t530;
;<179>              |   |      + END LOOP
;<179>              |   |
;<39>               |   |      %t545 = %t530;
;<15>               |   |   }
;<42>               |   |   %t546 = %t545  +  %t525;
;<43>               |   |   %t547 =  - %t546;
;<44>               |   |   (%t15)[i1 + i2 + 1][i1] = %t547;
;<46>               |   |   (%t12)[0][i2] = %t547;
;<178>              |   + END LOOP
;<177>              + END LOOP
;<177>
;<180>
;<180>              + DO i1 = 0, 248, 1   <DO_LOOP>
;<181>              |   + DO i2 = 0, -1 * i1 + 248, 1   <DO_LOOP>  <MAX_TC_EST = 249>
;<76>               |   |   %t566 = (%t15)[i1 + i2 + 1][i1];
;<78>               |   |   (%t12)[0][i2] = %t566;
;<181>              |   + END LOOP
;<181>              |
;<182>              |
;<182>              |   + DO i2 = 0, i1, 1   <DO_LOOP>  <MAX_TC_EST = 249>
;<93>               |   |   %t576 = (%t15)[i2][i1];
;<94>               |   |   %t578 = 0.000000e+00;
;<183>              |   |
;<183>              |   |   + DO i3 = 0, -1 * i1 + 248, 1   <DO_LOOP>  <MAX_TC_EST = 125>
;<100>              |   |   |   %t582 = (%t12)[0][i3];
;<102>              |   |   |   %t584 = (%t15)[i2][i1 + i3 + 1];
;<103>              |   |   |   %t585 = %t584  *  %t582;
;<104>              |   |   |   %t578 = %t585  +  %t578;
;<183>              |   |   + END LOOP
;<183>              |   |
;<114>              |   |   %t592 = %t578  +  %t576;
;<115>              |   |   (%t15)[i2][i1] = %t592;
;<182>              |   + END LOOP
;<182>              |
;<184>              |
;<184>              |   + DO i2 = 0, -1 * i1 + 248, 1   <DO_LOOP>  <MAX_TC_EST = 249>
;<126>              |   |   %t598 = 251  -  i1 + i2 + 2;
;<129>              |   |   %t621 = 0.000000e+00;
;<130>              |   |   if (i1 + i2 + 2 <=u 250)
;<130>              |   |   {
;<135>              |   |      %t604 = 0.000000e+00;
;<185>              |   |
;<185>              |   |      + DO i3 = 0, smax(2, (1 + %t598)) + -2, 1   <DO_LOOP>  <MAX_TC_EST = 126>
;<142>              |   |      |   %t609 = (%t12)[0][i2 + i3];
;<144>              |   |      |   %t611 = (%t15)[i1 + i2 + 1][i1 + i2 + i3 + 1];
;<145>              |   |      |   %t612 = %t611  *  %t609;
;<146>              |   |      |   %t604 = %t612  +  %t604;
;<185>              |   |      + END LOOP
;<185>              |   |
;<158>              |   |      %t621 = %t604;
;<130>              |   |   }
;<162>              |   |   (%t15)[i1 + i2 + 1][i1] = %t621;
;<184>              |   + END LOOP
;<180>              + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Dead Store Elimination (hir-dead-store-elimination) ***
;Function: crout_.19|_.250
;
; CHECK-NOT:        BEGIN REGION { modified }
;
; ModuleID = 'region_module.ll'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"test_fpu_$A" = external hidden global [250 x [250 x double]], align 16
@"crout_$IMAX" = external hidden unnamed_addr global [1 x i32], align 16

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #0

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.fabs.f64(double) #1

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #0

; Function Attrs: nofree nosync nounwind willreturn
declare i8* @llvm.stacksave() #2

; Function Attrs: nofree nosync nounwind willreturn
declare void @llvm.stackrestore(i8*) #2

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare i64 @llvm.smax.i64(i64, i64) #1

; Function Attrs: nofree nosync nounwind uwtable
define hidden fastcc void @"crout_.19|_.250"() unnamed_addr #3 {
entry:
  %t1 = alloca i32, align 16
  %t2 = alloca [2 x i32], align 16
  %t3 = alloca [2 x i32], align 16
  %t4 = alloca [2 x i32], align 16
  %t5 = alloca [2 x i32], align 16
  %t6 = alloca [2 x i32], align 16
  %t7 = alloca [2 x i32], align 16
  %t8 = alloca [2 x i32], align 16
  %t9 = getelementptr inbounds [2 x i32], [2 x i32]* %t8, i64 0, i64 0
  %t10 = getelementptr inbounds [2 x i32], [2 x i32]* %t6, i64 0, i64 0
  %t11 = getelementptr inbounds [2 x i32], [2 x i32]* %t3, i64 0, i64 0
  %t12 = alloca [250 x double], align 8
  %t13 = alloca [62500 x double], align 8
  %t14 = alloca [250 x i32], align 4
  %t15 = getelementptr inbounds [62500 x double], [62500 x double]* %t13, i64 0, i64 0
  %t16 = tail call i8* @llvm.stacksave()
  %t17 = alloca [250 x i32], align 4
  %t18 = getelementptr inbounds [250 x i32], [250 x i32]* %t17, i64 0, i64 0
  %t30 = getelementptr inbounds [250 x double], [250 x double]* %t12, i64 0, i64 0
  br label %t514

t514:                                              ; preds = %entry
  %t515 = phi i64 [ %t553, %t552 ], [ 2, %entry ]
  %t516 = phi i64 [ %t517, %t552 ], [ 1, %entry ]
  %t517 = add nuw nsw i64 %t516, 1
  br label %t518

t518:                                              ; preds = %t544, %t514
  %t519 = phi i64 [ %t549, %t544 ], [ %t515, %t514 ]
  %t520 = phi i64 [ %t550, %t544 ], [ 1, %t514 ]
  %t521 = tail call i64 @llvm.smax.i64(i64 %t520, i64 2)
  %t522 = add nuw nsw i64 %t516, %t521
  %t523 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 2000, double* nonnull elementtype(double) %t15, i64 %t519)
  %t524 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t523, i64 %t516)
  %t525 = load double, double* %t524, align 1, !tbaa !10
  %t526 = sub nuw nsw i64 %t519, %t516
  %t527 = icmp eq i64 %t519, %t517
  br i1 %t527, label %t544, label %t528

t528:                                              ; preds = %t518
  br label %t529

t529:                                              ; preds = %t529, %t528
  %t530 = phi double [ %t538, %t529 ], [ 0.000000e+00, %t528 ]
  %t531 = phi i64 [ %t539, %t529 ], [ %t517, %t528 ]
  %t532 = phi i64 [ %t540, %t529 ], [ 1, %t528 ]
  %t533 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t30, i64 %t532)
  %t534 = load double, double* %t533, align 1, !tbaa !14
  %t535 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t523, i64 %t531)
  %t536 = load double, double* %t535, align 1, !tbaa !10
  %t537 = fmul fast double %t536, %t534
  %t538 = fadd fast double %t537, %t530
  %t539 = add nuw nsw i64 %t531, 1
  %t540 = add nuw nsw i64 %t532, 1
  %t541 = icmp eq i64 %t539, %t522
  br i1 %t541, label %t542, label %t529

t542:                                              ; preds = %t529
  %t543 = phi double [ %t538, %t529 ]
  br label %t544

t544:                                              ; preds = %t542, %t518
  %t545 = phi double [ 0.000000e+00, %t518 ], [ %t543, %t542 ]
  %t546 = fadd fast double %t545, %t525
  %t547 = fneg fast double %t546
  store double %t547, double* %t524, align 1, !tbaa !10
  %t548 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t30, i64 %t526)
  store double %t547, double* %t548, align 1, !tbaa !14
  %t549 = add nuw nsw i64 %t519, 1
  %t550 = add nuw nsw i64 %t520, 1
  %t551 = icmp eq i64 %t549, 251
  br i1 %t551, label %t552, label %t518

t552:                                              ; preds = %t544
  %t553 = add nuw nsw i64 %t515, 1
  %t554 = icmp eq i64 %t553, 251
  br i1 %t554, label %t555, label %t514

t555:                                              ; preds = %t552
  br label %t556

t556:                                              ; preds = %t625, %t555
  %t557 = phi i64 [ %t560, %t625 ], [ 1, %t555 ]
  %t558 = phi i64 [ %t626, %t625 ], [ 2, %t555 ]
  %t559 = sub nuw nsw i64 250, %t557
  %t560 = add nuw nsw i64 %t557, 1
  br label %t561

t561:                                              ; preds = %t561, %t556
  %t562 = phi i64 [ %t568, %t561 ], [ %t560, %t556 ]
  %t563 = phi i64 [ %t569, %t561 ], [ 1, %t556 ]
  %t564 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 2000, double* nonnull elementtype(double) %t15, i64 %t562)
  %t565 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t564, i64 %t557)
  %t566 = load double, double* %t565, align 1, !tbaa !10
  %t567 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t30, i64 %t563)
  store double %t566, double* %t567, align 1, !tbaa !14
  %t568 = add nuw nsw i64 %t562, 1
  %t569 = add nuw nsw i64 %t563, 1
  %t570 = icmp ugt i64 %t569, %t559
  br i1 %t570, label %t571, label %t561

t571:                                              ; preds = %t561
  br label %t572

t572:                                              ; preds = %t590, %t571
  %t573 = phi i64 [ %t593, %t590 ], [ 1, %t571 ]
  %t574 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 2000, double* nonnull elementtype(double) %t15, i64 %t573)
  %t575 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t574, i64 %t557)
  %t576 = load double, double* %t575, align 1, !tbaa !10
  br label %t577

t577:                                              ; preds = %t577, %t572
  %t578 = phi double [ %t586, %t577 ], [ 0.000000e+00, %t572 ]
  %t579 = phi i64 [ %t587, %t577 ], [ %t560, %t572 ]
  %t580 = phi i64 [ %t588, %t577 ], [ 1, %t572 ]
  %t581 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t30, i64 %t580)
  %t582 = load double, double* %t581, align 1, !tbaa !14
  %t583 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t574, i64 %t579)
  %t584 = load double, double* %t583, align 1, !tbaa !10
  %t585 = fmul fast double %t584, %t582
  %t586 = fadd fast double %t585, %t578
  %t587 = add nuw nsw i64 %t579, 1
  %t588 = add nuw nsw i64 %t580, 1
  %t589 = icmp ugt i64 %t588, %t559
  br i1 %t589, label %t590, label %t577

t590:                                              ; preds = %t577
  %t591 = phi double [ %t586, %t577 ]
  %t592 = fadd fast double %t591, %t576
  store double %t592, double* %t575, align 1, !tbaa !10
  %t593 = add nuw nsw i64 %t573, 1
  %t594 = icmp eq i64 %t593, %t558
  br i1 %t594, label %t595, label %t572

t595:                                              ; preds = %t590
  br label %t596

t596:                                              ; preds = %t620, %t595
  %t597 = phi i64 [ %t623, %t620 ], [ %t558, %t595 ]
  %t598 = sub nsw i64 251, %t597
  %t599 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 2000, double* nonnull elementtype(double) %t15, i64 %t597)
  %t600 = icmp ugt i64 %t597, 250
  br i1 %t600, label %t620, label %t601

t601:                                              ; preds = %t596
  %t602 = sub nuw nsw i64 %t597, %t557
  br label %t603

t603:                                              ; preds = %t603, %t601
  %t604 = phi double [ %t613, %t603 ], [ 0.000000e+00, %t601 ]
  %t605 = phi i64 [ %t614, %t603 ], [ %t597, %t601 ]
  %t606 = phi i64 [ %t615, %t603 ], [ %t602, %t601 ]
  %t607 = phi i64 [ %t616, %t603 ], [ 1, %t601 ]
  %t608 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t30, i64 %t606)
  %t609 = load double, double* %t608, align 1, !tbaa !14
  %t610 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t599, i64 %t605)
  %t611 = load double, double* %t610, align 1, !tbaa !10
  %t612 = fmul fast double %t611, %t609
  %t613 = fadd fast double %t612, %t604
  %t614 = add nuw nsw i64 %t605, 1
  %t615 = add nuw nsw i64 %t606, 1
  %t616 = add nuw nsw i64 %t607, 1
  %t617 = icmp sgt i64 %t616, %t598
  br i1 %t617, label %t618, label %t603

t618:                                              ; preds = %t603
  %t619 = phi double [ %t613, %t603 ]
  br label %t620

t620:                                              ; preds = %t618, %t596
  %t621 = phi double [ 0.000000e+00, %t596 ], [ %t619, %t618 ]
  %t622 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* nonnull elementtype(double) %t599, i64 %t557)
  store double %t621, double* %t622, align 1, !tbaa !10
  %t623 = add nuw nsw i64 %t597, 1
  %t624 = icmp eq i64 %t623, 251
  br i1 %t624, label %t625, label %t596

t625:                                              ; preds = %t620
  %t626 = add nuw nsw i64 %t558, 1
  %t627 = icmp eq i64 %t560, 250
  br i1 %t627, label %t628, label %t556

t628:
  ret void
}
attributes #0 = { nounwind readnone speculatable }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nofree nosync nounwind willreturn }
attributes #3 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }

!omp_offload.info = !{}
!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"ThinLTO", i32 0}
!1 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!2 = !{i32 1, !"LTOPostLink", i32 1}
!3 = !{!4, !4, i64 0}
!4 = !{!"Generic Fortran Symbol", !5, i64 0}
!5 = !{!"ifx$root$3$crout_"}
!6 = !{!7, !7, i64 0}
!7 = !{!"ifx$unique_sym$114", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"ifx$unique_sym$117", !4, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"ifx$unique_sym$118", !4, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"ifx$unique_sym$119", !4, i64 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"ifx$unique_sym$121", !4, i64 0}
