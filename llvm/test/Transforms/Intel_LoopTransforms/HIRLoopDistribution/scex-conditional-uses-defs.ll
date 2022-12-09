; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-distribute-memrec,print<hir>" -disable-output -hir-cost-model-throttling=0 %s 2>&1 | FileCheck %s

; Check that we can correctly handle scalar expansion when defs/uses are in both
; paths of if then/else branch. If a scalar expansion candidate is unconditionally
; defined or used, then the scalar expansion load can be done at the beginning
; of the use loop.

; Before Transformation:

;   BEGIN REGION { }
;         + DO i1 = 0, 0, 1   <DO_LOOP>
;         |   + DO i2 = 0, 0, 1   <DO_LOOP>
;         |   |   + DO i3 = 0, -2, 1   <DO_LOOP>
;         |   |   |   %tmp484 = (null)[0]  *  0.000000e+00;
;         |   |   |   %tmp486 = (null)[0]  *  0.000000e+00;
;         |   |   |   if (%tmp488 < 0x4013851EC0000000)
;         |   |   |   {
;         |   |   |      %tmp506 = (null)[0]  *  0.000000e+00;
;         |   |   |      %tmp524 = 0.000000e+00;
;         |   |   |      %tmp525 = %tmp505;
;         |   |   |   }
;         |   |   |   else
;         |   |   |   {
;         |   |   |      %tmp517 = (null)[0]  *  0.000000e+00;
;         |   |   |      %tmp524 = 0.000000e+00;
;         |   |   |      %tmp525 = %tmp516;
;         |   |   |   }
;         |   |   |   %tmp531 = (null)[0]  -  (null)[0];
;         |   |   |   (null)[0] = 0.000000e+00;
;         |   |   |   %tmp534 = 0.000000e+00  -  (null)[0];
;         |   |   |   (null)[0] = 0.000000e+00;
;         |   |   |   %tmp540 = (null)[0];
;         |   |   |   if (%tmp540 > 0.000000e+00)
;         |   |   |   {
;         |   |   |      %tmp547 = (null)[0]  *  0.000000e+00;
;         |   |   |      (%arg4)[0] = 0.000000e+00;
;         |   |   |      (%arg6)[0] = 0.000000e+00;
;         |   |   |      (null)[0] = (null)[0];
;         |   |   |      %tmp585 = (null)[0]  -  (%arg5)[0];
;         |   |   |      %tmp587 = (%tmp446)[0]  *  0.000000e+00;
;         |   |   |      %tmp595 = 0.000000e+00  +  (null)[0];
;         |   |   |      %tmp600 = %tmp540  *  %tmp525;
;         |   |   |   }
;         |   |   |   if (%tmp488 < 0x4013851EC0000000)
;         |   |   |   {
;         |   |   |      %tmp625 = 0.000000e+00  +  0.000000e+00;
;         |   |   |      %tmp627 = (%tmp625 > %tmp524) ? %tmp524 : %tmp625;
;         |   |   |   }
;         |   |   |   else
;         |   |   |   {
;         |   |   |      %tmp633 = 0.000000e+00  *  0.000000e+00;
;         |   |   |      %tmp635 = (%tmp633 > %tmp524) ? %tmp524 : %tmp633;
;         |   |   |   }
;         |   |   |   (null)[0] = 0.000000e+00;
;         |   |   + END LOOP
;         |   + END LOOP
;         + END LOOP

;Scalar Expansion analysis:
;%tmp524 (sb:20) (In/Out 0/0) (26,32) -> (74,74,79,79,74,74,79,79) Recompute: 1
; ( 26 32 -> 74 ) ( 26 32 -> 79 ) ( 26 32 -> 74 ) ( 26 32 -> 79 )
;%tmp525 (sb:22) (In/Out 0/0) (27,33) -> (65,65) Recompute: 1
; ( 27 33 -> 65 )
;%tmp540 (sb:32) (In/Out 0/0) (44) -> (101,65) Recompute: 0
; ( 44 -> 101 ) ( 44 -> 65 )


; HIR after transformation

; CHECK : BEGIN REGION { modified }
; CHECK:  |   |   |   + DO i4 = 0, %min, 1
;         |   |   |   |   %tmp484 = (null)[0]  *  0.000000e+00;
;         |   |   |   |   %tmp486 = (null)[0]  *  0.000000e+00;
; CHECK:  |   |   |   |   if (%tmp488 < 0x4013851EC0000000)
;         |   |   |   |   {
;         |   |   |   |      %tmp506 = (null)[0]  *  0.000000e+00;
;         |   |   |   |      %tmp524 = 0.000000e+00;
; CHECK:  |   |   |   |      (%.TempArray)[0][i4] = %tmp524;
;         |   |   |   |      %tmp525 = %tmp505;
; CHECK:  |   |   |   |      (%.TempArray9)[0][i4] = %tmp525;
;         |   |   |   |   }
;         |   |   |   |   else
;         |   |   |   |   {
;         |   |   |   |      %tmp517 = (null)[0]  *  0.000000e+00;
;         |   |   |   |      %tmp524 = 0.000000e+00;
; CHECK:  |   |   |   |      (%.TempArray)[0][i4] = %tmp524;
;         |   |   |   |      %tmp525 = %tmp516;
; CHECK:  |   |   |   |      (%.TempArray9)[0][i4] = %tmp525;
;         |   |   |   |   }
;         |   |   |   |   %tmp531 = (null)[0]  -  (null)[0];
;         |   |   |   |   (null)[0] = 0.000000e+00;
;         |   |   |   |   %tmp534 = 0.000000e+00  -  (null)[0];
;         |   |   |   |   (null)[0] = 0.000000e+00;
;         |   |   |   |   %tmp540 = (null)[0];
; CHECK:  |   |   |   |   (%.TempArray11)[0][i4] = %tmp540;
;         |   |   |   |   if (%tmp540 > 0.000000e+00)
;         |   |   |   |   {
;         |   |   |   |      %tmp547 = (null)[0]  *  0.000000e+00;
;         |   |   |   |      (%arg4)[0] = 0.000000e+00;
;         |   |   |   |      (%arg6)[0] = 0.000000e+00;
;         |   |   |   |      (null)[0] = (null)[0];
;         |   |   |   |      %tmp585 = (null)[0]  -  (%arg5)[0];
;         |   |   |   |      %tmp587 = (%tmp446)[0]  *  0.000000e+00;
;         |   |   |   |      %tmp595 = 0.000000e+00  +  (null)[0];
;         |   |   |   |   }
;         |   |   |   |   (null)[0] = 0.000000e+00;
;         |   |   |   + END LOOP
;         |   |   |
; CHECK:  |   |   |   + DO i4 = 0, %min, 1
; CHECK:  |   |   |   |   %tmp524 = (%.TempArray)[0][i4];
; CHECK:  |   |   |   |   %tmp525 = (%.TempArray9)[0][i4];
; CHECK:  |   |   |   |   %tmp540 = (%.TempArray11)[0][i4];
; CHECK:  |   |   |   |   if (%tmp540 > 0.000000e+00)
;         |   |   |   |   {
; CHECK:  |   |   |   |      %tmp600 = %tmp540  *  %tmp525;
;         |   |   |   |   }
; CHECK:  |   |   |   |   if (%tmp488 < 0x4013851EC0000000)
;         |   |   |   |   {
;         |   |   |   |      %tmp625 = 0.000000e+00  +  0.000000e+00;
; CHECK:  |   |   |   |      %tmp627 = (%tmp625 > %tmp524) ? %tmp524 : %tmp625;
;         |   |   |   |   }
;         |   |   |   |   else
;         |   |   |   |   {
;         |   |   |   |      %tmp633 = 0.000000e+00  *  0.000000e+00;
; CHECK:  |   |   |   |      %tmp635 = (%tmp633 > %tmp524) ? %tmp524 : %tmp633;
;         |   |   |   |   }
;         |   |   |   + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @quux(float* noalias %arg4, float* noalias %arg5, float* noalias %arg6, float %tmp488, float %tmp505, float %tmp516, float* %tmp446) {
bb:
  br label %bb440

bb440:                                            ; preds = %bb730, %bb
  br label %bb452

bb452:                                            ; preds = %bb727, %bb440
  %tmp460 = call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) null, i64 0)
  br label %bb469

bb469:                                            ; preds = %bb637, %bb452
  %tmp470 = phi i64 [ 1, %bb452 ], [ %tmp538, %bb637 ]
  %tmp471 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) null, i64 0)
  %tmp472 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) null, i64 0)
  %tmp473 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) null, i64 0)
  %tmp476 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) null, i64 0)
  %tmp478 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) null, i64 0)
  %tmp480 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) null, i64 0)
  %tmp482 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) null, i64 0)
  %tmp483 = load float, float* %tmp471, align 1
  %tmp484 = fmul fast float %tmp483, 0.000000e+00
  %tmp485 = load float, float* %tmp472, align 1
  %tmp486 = fmul fast float %tmp485, 0.000000e+00
  %tmp501 = fcmp fast olt float %tmp488, 0x4013851EC0000000
  br i1 %tmp501, label %bb502, label %bb512

bb502:                                            ; preds = %bb469
  %tmp5053 = load float, float* null, align 1
  %tmp506 = fmul fast float %tmp5053, 0.000000e+00
  br label %bb522

bb512:                                            ; preds = %bb469
  %tmp5164 = load float, float* null, align 1
  %tmp517 = fmul fast float %tmp5164, 0.000000e+00
  br label %bb522

bb522:                                            ; preds = %bb512, %bb502
  %tmp524 = phi float [ 0.000000e+00, %bb502 ], [ 0.000000e+00, %bb512 ]
  %tmp525 = phi float [ %tmp505, %bb502 ], [ %tmp516, %bb512 ]
  %tmp529 = load float, float* %tmp473, align 1
  %tmp530 = load float, float* %tmp476, align 1
  %tmp531 = fsub fast float %tmp529, %tmp530
  store float 0.000000e+00, float* %tmp480, align 1
  %tmp533 = load float, float* %tmp478, align 1
  %tmp534 = fsub fast float 0.000000e+00, %tmp533
  store float 0.000000e+00, float* %tmp482, align 1
  %tmp538 = add nuw nsw i64 %tmp470, 1
  %tmp540 = load float, float* null, align 1
  %tmp541 = fcmp fast ogt float %tmp540, 0.000000e+00
  br i1 %tmp541, label %bb542, label %bb618

bb542:                                            ; preds = %bb522
  %tmp546 = load float, float* %tmp460, align 4
  %tmp547 = fmul fast float %tmp546, 0.000000e+00
  store float 0.000000e+00, float* %arg4, align 1
  store float 0.000000e+00, float* %arg6, align 1
  %tmp569 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) null, i64 0)
  %tmp570 = load float, float* null, align 1
  store float %tmp570, float* %tmp569, align 1
  %tmp575 = tail call float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8 0, i64 0, i64 0, float* elementtype(float) null, i64 0)
  %tmp576 = load float, float* %tmp575, align 1
  %tmp582 = load float, float* null, align 1
  %tmp584 = load float, float* %arg5, align 1
  %tmp585 = fsub fast float %tmp582, %tmp584
  %tmp586 = load float, float* %tmp446, align 1
  %tmp587 = fmul fast float %tmp586, 0.000000e+00
  %tmp595 = fadd fast float 0.000000e+00, %tmp576
  %tmp600 = fmul fast float %tmp540, %tmp525
  br label %bb618

bb618:                                            ; preds = %bb542, %bb522
  br i1 %tmp501, label %bb623, label %bb630

bb623:                                            ; preds = %bb618
  %tmp625 = fadd fast float 0.000000e+00, 0.000000e+00
  %tmp626 = fcmp fast ogt float %tmp625, %tmp524
  %tmp627 = select i1 %tmp626, float %tmp524, float %tmp625
  br label %bb637

bb630:                                            ; preds = %bb618
  %tmp633 = fmul fast float 0.000000e+00, 0.000000e+00
  %tmp634 = fcmp fast ogt float %tmp633, %tmp524
  %tmp635 = select i1 %tmp634, float %tmp524, float %tmp633
  br label %bb637

bb637:                                            ; preds = %bb630, %bb623
  store float 0.000000e+00, float* null, align 1
  %tmp725 = icmp eq i64 %tmp538, 0
  br i1 %tmp725, label %bb727, label %bb469

bb727:                                            ; preds = %bb637
  %tmp729 = icmp eq i64 0, 0
  br i1 %tmp729, label %bb730, label %bb452

bb730:                                            ; preds = %bb727
  %tmp733 = icmp eq i64 0, 0
  br i1 %tmp733, label %bb734, label %bb440

bb734:                                            ; preds = %bb730
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare float* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64(i8, i64, i64, float*, i64) #0

; uselistorder directives
uselistorder float* (i8, i64, i64, float*, i64)* @llvm.intel.subscript.p0f32.i64.i64.p0f32.i64, { 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 }

attributes #0 = { nounwind readnone speculatable }
