; RUN: opt -hir-ssa-deconstruction -hir-cost-model-throttling=0 -hir-temp-cleanup -tbaa -hir-pm-symbolic-tripcount-completeunroll -print-after=hir-pm-symbolic-tripcount-completeunroll -disable-output < %s 2>&1 | FileCheck %s

; This test checks if loops at add_neighbour() and remove_neighbour() functions inside cpu2017/541.leela/FastBoard.cpp get unrolled.

; *** IR Dump Before HIR Symbolic TripCount CompleteUnroll Pattern Match Pass ***
; Function: _ZN9FastBoard17update_board_fastEii

; BEGIN REGION { }
;       + DO i1 = 0, 3, 1   <DO_LOOP>
;       |   %t38.out = %t38;
;       |   %t40 = (%0)[0].12.0[i1];
;       |   %t44 = (%0)[0].10.0[%t40 + %2];
;       |   (%0)[0].10.0[%t40 + %2] = %t44 + %t35;
;       |   %t48 = (%0)[0].7.0[%t40 + %2];
;       |   if (%t38 > 0)
;       |   {
;       |      + DO i2 = 0, smax(1, sext.i32.i64(%t38)) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 4>
;       |      |   if ((%t4)[0].0[i2] == %t48)
;       |      |   {
;       |      |      goto t68;
;       |      |   }
;       |      + END LOOP
;       |
;       |      goto t61;
;       |      t68:
;       |      goto t69;
;       |   }
;       |   t61:
;       |   %t64 = (%0)[0].8.0[%t48];
;       |   (%0)[0].8.0[%t48] = %t64 + -1;
;       |   %t38 = %t38  +  1;
;       |   (%t4)[0].0[%t38.out] = %t48;
;       |   t69:
;       + END LOOP
; END REGION
;
; BEGIN REGION { }
;       + UNKNOWN LOOP i1
;       |   <i1 = 0>
;       |   t481:
;       |   (%0)[0].5.0[%t484] = 2;
;       |   (%0)[0].7.0[%t484] = 441;
;       |   @llvm.lifetime.start.p0i8(16,  &((%t474)[0]));
;       |   %t490 = 0;
;       |
;       |   + DO i2 = 0, 3, 1   <DO_LOOP>
;       |   |   %t490.out = %t490;
;       |   |   %t492 = (%0)[0].12.0[i2];
;       |   |   %t496 = (%0)[0].10.0[%t492 + %t484];
;       |   |   (%0)[0].10.0[%t492 + %t484] = %t496 + %t479;
;       |   |   %t500 = (%0)[0].7.0[%t492 + %t484];
;       |   |   if (%t490 > 0)
;       |   |   {
;       |   |      + DO i3 = 0, smax(1, sext.i32.i64(%t490)) + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 4>
;       |   |      |   if ((%t8)[0].0[i3] == %t500)
;       |   |      |   {
;       |   |      |      goto t520;
;       |   |      |   }
;       |   |      + END LOOP
;       |   |
;       |   |      goto t513;
;       |   |      t520:
;       |   |      goto t521;
;       |   |   }
;       |   |   t513:
;       |   |   %t516 = (%0)[0].8.0[%t500];
;       |   |   (%0)[0].8.0[%t500] = %t516 + 1;
;       |   |   %t490 = %t490  +  1;
;       |   |   (%t8)[0].0[%t490.out] = %t500;
;       |   |   t521:
;       |   + END LOOP
;       |
;       |   %t526 = -1 * i1 + %t480  +  -1;
;       |   @llvm.lifetime.end.p0i8(16,  &((%t474)[0]));
;       |   (%0)[0].1.0[%t484] = i1 + trunc.i64.i16(%t260);
;       |   (%0)[0].0.0[i1 + %t260] = %t484;
;       |   %t531 = i1 + %t260  +  1;
;       |   %t533 = (%0)[0].6.0[%t484];
;       |   %t484 = %t533;
;       |   if (%t533 != %2)
;       |   {
;       |      <i1 = i1 + 1>
;       |      goto t481;
;       |   }
;       + END LOOP
; END REGION

; *** IR Dump After HIR Symbolic TripCount CompleteUnroll Pattern Match Pass ***
; Function: _ZN9FastBoard17update_board_fastEii
;
; CHECK: BEGIN REGION { modified }
; CHECK:       %t40 = (%0)[0].12.0[0];
; CHECK:       %t44 = (%0)[0].10.0[%t40 + %2];
; CHECK:       (%0)[0].10.0[%t40 + %2] = %t44 + %t35;
; CHECK:       %t48 = (%0)[0].7.0[%t40 + %2];
; CHECK:       %t64 = (%0)[0].8.0[%t48];
; CHECK:       %mv = (%0)[0].12.0[1];
; CHECK:       %mv4 = (%0)[0].10.0[%2 + %mv];
; CHECK:       (%0)[0].10.0[%2 + %mv] = %t35 + %mv4;
; CHECK:       %mv5 = (%0)[0].7.0[%2 + %mv];
; CHECK:       %mv6 = (%0)[0].8.0[%mv5];
; CHECK:       %mv7 = (%0)[0].12.0[2];
; CHECK:       %mv8 = (%0)[0].10.0[%2 + %mv7];
; CHECK:       (%0)[0].10.0[%2 + %mv7] = %t35 + %mv8;
; CHECK:       %mv9 = (%0)[0].7.0[%2 + %mv7];
; CHECK:       %mv10 = (%0)[0].8.0[%mv9];
; CHECK:       %mv11 = (%0)[0].12.0[3];
; CHECK:       %mv12 = (%0)[0].10.0[%2 + %mv11];
; CHECK:       (%0)[0].10.0[%2 + %mv11] = %t35 + %mv12;
; CHECK:       %mv13 = (%0)[0].7.0[%2 + %mv11];
; CHECK:       %mv14 = (%0)[0].8.0[%mv13];
; CHECK:       (%0)[0].8.0[%t48] = %t64 + -1;
; CHECK:       (%0)[0].8.0[%mv5] = %mv6 + -1;
; CHECK:       (%0)[0].8.0[%mv9] = %mv10 + -1;
; CHECK:       (%0)[0].8.0[%mv13] = %mv14 + -1;
; CHECK: END REGION
;
; CHECK: BEGIN REGION { modified }
; CHECK:       + UNKNOWN LOOP i1
; CHECK:       |   <i1 = 0>
; CHECK:       |   t481:
; CHECK:       |   (%0)[0].5.0[%t484] = 2;
; CHECK:       |   (%0)[0].7.0[%t484] = 441;
; CHECK:       |   @llvm.lifetime.start.p0i8(16,  &((%t474)[0]));
; CHECK:       |   %t490 = 0;
; CHECK:       |   %t492 = (%0)[0].12.0[0];
; CHECK:       |   %t496 = (%0)[0].10.0[%t492 + %t484];
; CHECK:       |   (%0)[0].10.0[%t492 + %t484] = %t496 + %t479;
; CHECK:       |   %t500 = (%0)[0].7.0[%t492 + %t484];
; CHECK:       |   %t516 = (%0)[0].8.0[%t500];
; CHECK:       |   %mv15 = (%0)[0].12.0[1];
; CHECK:       |   %mv16 = (%0)[0].10.0[%t484 + %mv15];
; CHECK:       |   (%0)[0].10.0[%t484 + %mv15] = %t479 + %mv16;
; CHECK:       |   %mv17 = (%0)[0].7.0[%t484 + %mv15];
; CHECK:       |   %mv18 = (%0)[0].8.0[%mv17];
; CHECK:       |   %mv19 = (%0)[0].12.0[2];
; CHECK:       |   %mv20 = (%0)[0].10.0[%t484 + %mv19];
; CHECK:       |   (%0)[0].10.0[%t484 + %mv19] = %t479 + %mv20;
; CHECK:       |   %mv21 = (%0)[0].7.0[%t484 + %mv19];
; CHECK:       |   %mv22 = (%0)[0].8.0[%mv21];
; CHECK:       |   %mv23 = (%0)[0].12.0[3];
; CHECK:       |   %mv24 = (%0)[0].10.0[%t484 + %mv23];
; CHECK:       |   (%0)[0].10.0[%t484 + %mv23] = %t479 + %mv24;
; CHECK:       |   %mv25 = (%0)[0].7.0[%t484 + %mv23];
; CHECK:       |   %mv26 = (%0)[0].8.0[%mv25];
; CHECK:       |   (%0)[0].8.0[%t500] = %t516 + 1;
; CHECK:       |   (%0)[0].8.0[%mv17] = %mv18 + 1;
; CHECK:       |   (%0)[0].8.0[%mv21] = %mv22 + 1;
; CHECK:       |   (%0)[0].8.0[%mv25] = %mv26 + 1;
; CHECK:       |   %t526 = -1 * i1 + %t480  +  -1;
; CHECK:       |   @llvm.lifetime.end.p0i8(16,  &((%t474)[0]));
; CHECK:       |   (%0)[0].1.0[%t484] = i1 + trunc.i64.i16(%t260);
; CHECK:       |   (%0)[0].0.0[i1 + %t260] = %t484;
; CHECK:       |   %t531 = i1 + %t260  +  1;
; CHECK:       |   %t533 = (%0)[0].6.0[%t484];
; CHECK:       |   %t484 = %t533;
; CHECK:       |   if (%t533 != %2)
; CHECK:       |   {
; CHECK:       |      <i1 = i1 + 1>
; CHECK:       |      goto t481;
; CHECK:       |   }
; CHECK:       + END LOOP
; CHECK: END REGION


; ModuleID = 'func.bc'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.boost::array.4" = type { [2 x i32] }
%class.FastBoard = type <{ %"class.boost::array", %"class.boost::array", i32, i32, i32, %"class.boost::array.0", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array.1", %"class.boost::array", [2 x i8], %"class.boost::array.2", %"class.boost::array.3", %"class.boost::array.4", %"class.boost::array.4", %"class.std::vector", i32, [4 x i8] }>
%"class.boost::array.0" = type { [441 x i32] }
%"class.boost::array.1" = type { [442 x i16] }
%"class.boost::array" = type { [441 x i16] }
%"class.boost::array.2" = type { [4 x i32] }
%"class.boost::array.3" = type { [8 x i32] }
%"class.std::vector" = type { %"struct.std::_Vector_base" }
%"struct.std::_Vector_base" = type { %"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" }
%"struct.std::_Vector_base<int, std::allocator<int> >::_Vector_impl" = type { i32*, i32*, i32* }

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #0

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #0

; Function Attrs: nounwind uwtable
define hidden fastcc void @_ZN9FastBoard17update_board_fastEii(%class.FastBoard*, i32, i32, i16 %t35, i64 %t260, i32* %t473, i32 %t480, i8* nonnull %t474, i16 %t479) unnamed_addr #1 align 2 {
header:
  %t4 = alloca %"class.boost::array.2", align 4
  %t5 = alloca %"class.boost::array.2", align 4
  %t6 = alloca %"class.boost::array.2", align 4
  %t7 = alloca %"class.boost::array.2", align 4
  %t8 = alloca %"class.boost::array.2", align 4
  %t9 = sext i32 %2 to i64
  %t10 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 10, i32 0, i64 %t9
  %t11 = load i16, i16* %t10, align 2, !tbaa !3
  %t12 = zext i16 %t11 to i32
  %t13 = icmp eq i32 %1, 0
  %t14 = zext i1 %t13 to i64
  %t250 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 2
  br label %t36

t36:                                              ; preds = %t69, %header
  %t37 = phi i64 [ 0, %header ], [ %t71, %t69 ]
  %t38 = phi i32 [ 0, %header ], [ %t70, %t69 ]
  %t39 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 12, i32 0, i64 %t37, !intel-tbaa !23
  %t40 = load i32, i32* %t39, align 4, !tbaa !23
  %t41 = add nsw i32 %t40, %2
  %t42 = sext i32 %t41 to i64
  %t43 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 10, i32 0, i64 %t42, !intel-tbaa !3
  %t44 = load i16, i16* %t43, align 2, !tbaa !3
  %t45 = add i16 %t35, %t44
  store i16 %t45, i16* %t43, align 2, !tbaa !3
  %t46 = icmp sgt i32 %t38, 0
  %t47 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 7, i32 0, i64 %t42
  %t48 = load i16, i16* %t47, align 2, !tbaa !24
  %t49 = zext i16 %t48 to i32
  %t50 = sext i32 %t38 to i64
  br i1 %t46, label %t51, label %t61

t51:                                              ; preds = %t36
  br label %t54

t52:                                              ; preds = %t54
  %t53 = icmp slt i64 %t59, %t50
  br i1 %t53, label %t54, label %t60

t54:                                              ; preds = %t52, %t51
  %t55 = phi i64 [ %t59, %t52 ], [ 0, %t51 ]
  %t56 = getelementptr inbounds %"class.boost::array.2", %"class.boost::array.2"* %t4, i64 0, i32 0, i64 %t55, !intel-tbaa !25
  %t57 = load i32, i32* %t56, align 4, !tbaa !25
  %t58 = icmp eq i32 %t57, %t49
  %t59 = add nuw nsw i64 %t55, 1
  br i1 %t58, label %t68, label %t52

t60:                                              ; preds = %t52
  br label %t61

t61:                                              ; preds = %t60, %t36
  %t62 = zext i16 %t48 to i64
  %t63 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 8, i32 0, i64 %t62, !intel-tbaa !26
  %t64 = load i16, i16* %t63, align 2, !tbaa !26
  %t65 = add i16 %t64, -1
  store i16 %t65, i16* %t63, align 2, !tbaa !26
  %t66 = add nsw i32 %t38, 1
  %t67 = getelementptr inbounds %"class.boost::array.2", %"class.boost::array.2"* %t4, i64 0, i32 0, i64 %t50, !intel-tbaa !25
  store i32 %t49, i32* %t67, align 4, !tbaa !25
  br label %t69

t68:                                              ; preds = %t54
  br label %t69

t69:                                              ; preds = %t68, %t61
  %t70 = phi i32 [ %t66, %t61 ], [ %t38, %t68 ]
  %t71 = add nuw nsw i64 %t37, 1
  %t72 = icmp eq i64 %t71, 4
  br i1 %t72, label %t481.preheader, label %t36

t481.preheader:                                   ; preds = %t69
  br label %t481

t481:                                             ; preds = %t481.preheader, %t525
  %t482 = phi i64 [ %t531, %t525 ], [ %t260, %t481.preheader ]
  %t483 = phi i32 [ %t526, %t525 ], [ %t480, %t481.preheader ]
  %t484 = phi i32 [ %t534, %t525 ], [ %2, %t481.preheader ]
  %t485 = sext i32 %t484 to i64
  %t486 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 5, i32 0, i64 %t485, !intel-tbaa !27
  store i32 2, i32* %t486, align 4, !tbaa !27
  %t487 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 7, i32 0, i64 %t485, !intel-tbaa !24
  store i16 441, i16* %t487, align 2, !tbaa !24
  call void @llvm.lifetime.start.p0i8(i64 16, i8* nonnull %t474) #2
  br label %t488

t488:                                             ; preds = %t521, %t481
  %t489 = phi i64 [ 0, %t481 ], [ %t523, %t521 ]
  %t490 = phi i32 [ 0, %t481 ], [ %t522, %t521 ]
  %t491 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 12, i32 0, i64 %t489, !intel-tbaa !23
  %t492 = load i32, i32* %t491, align 4, !tbaa !23
  %t493 = add nsw i32 %t492, %t484
  %t494 = sext i32 %t493 to i64
  %t495 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 10, i32 0, i64 %t494, !intel-tbaa !3
  %t496 = load i16, i16* %t495, align 2, !tbaa !3
  %t497 = add i16 %t479, %t496
  store i16 %t497, i16* %t495, align 2, !tbaa !3
  %t498 = icmp sgt i32 %t490, 0
  %t499 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 7, i32 0, i64 %t494
  %t500 = load i16, i16* %t499, align 2, !tbaa !24
  %t501 = zext i16 %t500 to i32
  %t502 = sext i32 %t490 to i64
  br i1 %t498, label %t503, label %t513

t503:                                             ; preds = %t488
  br label %t506

t504:                                             ; preds = %t506
  %t505 = icmp slt i64 %t511, %t502
  br i1 %t505, label %t506, label %t512

t506:                                             ; preds = %t504, %t503
  %t507 = phi i64 [ %t511, %t504 ], [ 0, %t503 ]
  %t508 = getelementptr inbounds %"class.boost::array.2", %"class.boost::array.2"* %t8, i64 0, i32 0, i64 %t507, !intel-tbaa !25
  %t509 = load i32, i32* %t508, align 4, !tbaa !25
  %t510 = icmp eq i32 %t509, %t501
  %t511 = add nuw nsw i64 %t507, 1
  br i1 %t510, label %t520, label %t504

t512:                                             ; preds = %t504
  br label %t513

t513:                                             ; preds = %t512, %t488
  %t514 = zext i16 %t500 to i64
  %t515 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 8, i32 0, i64 %t514, !intel-tbaa !26
  %t516 = load i16, i16* %t515, align 2, !tbaa !26
  %t517 = add i16 %t516, 1
  store i16 %t517, i16* %t515, align 2, !tbaa !26
  %t518 = add nsw i32 %t490, 1
  %t519 = getelementptr inbounds %"class.boost::array.2", %"class.boost::array.2"* %t8, i64 0, i32 0, i64 %t502, !intel-tbaa !25
  store i32 %t501, i32* %t519, align 4, !tbaa !25
  br label %t521

t520:                                             ; preds = %t506
  br label %t521

t521:                                             ; preds = %t520, %t513
  %t522 = phi i32 [ %t518, %t513 ], [ %t490, %t520 ]
  %t523 = add nuw nsw i64 %t489, 1
  %t524 = icmp eq i64 %t523, 4
  br i1 %t524, label %t525, label %t488

t525:                                             ; preds = %t521
  %t526 = add nsw i32 %t483, -1
  call void @llvm.lifetime.end.p0i8(i64 16, i8* nonnull %t474) #2
  %t527 = trunc i64 %t482 to i16
  %t528 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 1, i32 0, i64 %t485, !intel-tbaa !28
  store i16 %t527, i16* %t528, align 2, !tbaa !28
  %t529 = trunc i32 %t484 to i16
  %t530 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 0, i32 0, i64 %t482, !intel-tbaa !29
  store i16 %t529, i16* %t530, align 2, !tbaa !29
  %t531 = add i64 %t482, 1
  %t532 = getelementptr inbounds %class.FastBoard, %class.FastBoard* %0, i64 0, i32 6, i32 0, i64 %t485, !intel-tbaa !30
  %t533 = load i16, i16* %t532, align 2, !tbaa !30
  %t534 = zext i16 %t533 to i32
  %t535 = icmp eq i32 %t534, %2
  br i1 %t535, label %t536, label %t481

t536:                                             ; preds = %t525
  %t537 = phi i32 [ %t526, %t525 ]
  %t538 = phi i64 [ %t531, %t525 ]
  %t539 = trunc i64 %t538 to i32
  store i32 %t537, i32* %t473, align 4, !tbaa !31
  store i32 %t539, i32* %t250, align 4, !tbaa !32
  br label %t540

t540:                                             ; preds = %t536
  ret void
}

attributes #0 = { argmemonly nounwind }
attributes #1 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "prefer-vector-width"="256" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+mpx,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+rtm,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2}

!0 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang a53756907774b7d85a523756d285be3e3ac08d1c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm de14ad0abb4862f88789109c251684247249d57b)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{!4, !7, i64 7076}
!4 = !{!"struct@_ZTS9FastBoard", !5, i64 0, !5, i64 882, !10, i64 1764, !10, i64 1768, !10, i64 1772, !11, i64 1776, !14, i64 3540, !14, i64 4424, !14, i64 5308, !14, i64 6192, !5, i64 7076, !16, i64 7960, !18, i64 7976, !20, i64 8008, !20, i64 8016, !22, i64 8024, !10, i64 8048}
!5 = !{!"struct@_ZTSN5boost5arrayItLm441EEE", !6, i64 0}
!6 = !{!"array@_ZTSA441_t", !7, i64 0}
!7 = !{!"short", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C++ TBAA"}
!10 = !{!"int", !8, i64 0}
!11 = !{!"struct@_ZTSN5boost5arrayIN9FastBoard8square_tELm441EEE", !12, i64 0}
!12 = !{!"array@_ZTSA441_N9FastBoard8square_tE", !13, i64 0}
!13 = !{!"_ZTSN9FastBoard8square_tE", !8, i64 0}
!14 = !{!"struct@_ZTSN5boost5arrayItLm442EEE", !15, i64 0}
!15 = !{!"array@_ZTSA442_t", !7, i64 0}
!16 = !{!"struct@_ZTSN5boost5arrayIiLm4EEE", !17, i64 0}
!17 = !{!"array@_ZTSA4_i", !10, i64 0}
!18 = !{!"struct@_ZTSN5boost5arrayIiLm8EEE", !19, i64 0}
!19 = !{!"array@_ZTSA8_i", !10, i64 0}
!20 = !{!"struct@_ZTSN5boost5arrayIiLm2EEE", !21, i64 0}
!21 = !{!"array@_ZTSA2_i", !10, i64 0}
!22 = !{!"struct@_ZTSSt6vectorIiSaIiEE"}
!23 = !{!20, !10, i64 0}
!24 = !{!4, !13, i64 1776}
!25 = !{!4, !7, i64 3540}
!26 = !{!4, !7, i64 4424}
!27 = !{!4, !7, i64 5308}
!28 = !{!4, !7, i64 6192}
!29 = !{!4, !10, i64 8016}
!30 = !{!4, !10, i64 7960}
!31 = !{!16, !10, i64 0}
!32 = !{!4, !10, i64 1764}
!33 = !{!4, !7, i64 882}
!34 = !{!4, !7, i64 0}
!35 = !{!4, !10, i64 8008}

