; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-collapse,print<hir>" -aa-pipeline="basic-aa" -disable-output  < %s 2>&1 | FileCheck %s

; Test checks that loop collapse pass works with near-perfect loop nest of this particular type.

; HIR before transformation:
;            BEGIN REGION { }
;                  + DO i1 = 0, %i395 + -1, 1   <DO_LOOP>
;                  |   %t2 = %t1;
;                  |
;                  |   + DO i2 = 0, zext.i32.i64(%i395) + -1, 1   <DO_LOOP>
;                  |   |   %t3 = (%i397)[%i395 * i1 + i2];
;                  |   |   %i462 = %t3 > %t2;
;                  |   |   %i465 = (%t3 + %i448 > %i402) ? 10 : %i462;
;                  |   |   %i448 = (%t3 + %i448 > %i402) ? 0 : %t3 + %i448;
;                  |   |   %i446 = %i465  +  %i446;
;                  |   |   %t2 = %t3;
;                  |   + END LOOP
;                  |
;                  |   %t1 = %t3;
;                  + END LOOP
;            END REGION

; HIR after transformation:
; CHECK:     BEGIN REGION { modified }
; CHECK:           %t2 = %t1;
;    
; CHECK:           + DO i1 = 0, (zext.i32.i64(%i395) * zext.i32.i64(%i395)) + -1, 1   <DO_LOOP>
; CHECK:           |   %t3 = (%i397)[i1];
; CHECK:           |   %i462 = %t3 > %t2;
; CHECK:           |   %i465 = (%t3 + %i448 > %i402) ? 10 : %i462;
; CHECK:           |   %i448 = (%t3 + %i448 > %i402) ? 0 : %t3 + %i448;
; CHECK:           |   %i446 = %i465  +  %i446;
; CHECK:           |   %t2 = %t3;
; CHECK:           + END LOOP
; CHECK:     END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define hidden fastcc signext i16 @calc_func(i32 %i395, ptr nocapture noundef %i397, i16 %i402) {
entry:
  br label %bb442

bb442:                                            ; preds = %entry
  %i443 = zext i16 %i402 to i32
  %i407 = zext i32 %i395 to i64
  br label %bb444

bb444:                                            ; preds = %bb469, %bb442
  %i445 = phi i32 [ 0, %bb442 ], [ %i473, %bb469 ]
  %i446 = phi i16 [ 0, %bb442 ], [ %i472, %bb469 ]
  %t1 = phi i32 [ 0, %bb442 ], [ %i470, %bb469 ]
  %i448 = phi i32 [ 0, %bb442 ], [ %i471, %bb469 ]
  %i449 = mul i32 %i445, %i395
  br label %bb450

bb450:                                            ; preds = %bb450, %bb444
  %i451 = phi i64 [ 0, %bb444 ], [ %i467, %bb450 ]
  %i452 = phi i16 [ %i446, %bb444 ], [ %i466, %bb450 ]
  %t2 = phi i32 [ %t1, %bb444 ], [ %t3, %bb450 ]
  %i454 = phi i32 [ %i448, %bb444 ], [ %i464, %bb450 ]
  %i455 = trunc i64 %i451 to i32
  %i456 = add i32 %i449, %i455
  %i457 = zext i32 %i456 to i64
  %i458 = getelementptr inbounds i32, ptr %i397, i64 %i457
  %t3 = load i32, ptr %i458, align 4
  %i460 = add nsw i32 %t3, %i454
  %i461 = icmp sgt i32 %i460, %i443
  %i462 = icmp sgt i32 %t3, %t2
  %i463 = zext i1 %i462 to i16
  %i464 = select i1 %i461, i32 0, i32 %i460
  %i465 = select i1 %i461, i16 10, i16 %i463
  %i466 = add i16 %i465, %i452
  %i467 = add nuw nsw i64 %i451, 1
  %i468 = icmp eq i64 %i467, %i407
  br i1 %i468, label %bb469, label %bb450

bb469:                                            ; preds = %bb450
  %i470 = phi i32 [ %t3, %bb450 ]
  %i471 = phi i32 [ %i464, %bb450 ]
  %i472 = phi i16 [ %i466, %bb450 ]
  %i473 = add nuw i32 %i445, 1
  %i474 = icmp eq i32 %i473, %i395
  br i1 %i474, label %bb475, label %bb444

bb475:
;  %ret = trunc i32 %i471 to i16
  ret i16 %i472
}
