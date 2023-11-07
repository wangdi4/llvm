; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that HIRLoopReroll doesn't happen.
; Note that final live-in base CE doesn't match.
; (%tmp30)[2 * i1 + 1] vs (%tmp36)[2 * i1 + 2] - %tmp30 vs %tmp36
;
; This lit-test is quite similar to ptrtoint.ll except for the differences noted above.

; BEGIN REGION { }
;       + DO i1 = 0, %tmp446 + -1, 1   <DO_LOOP>
;       |   %tmp451 = (%tmp30)[2 * i1 + 1];
;       |   %tmp456 = (%tmp30)[-2 * i1 + %tmp233 + -1];
;       |   %tmp460 = inttoptr.i64.%struct.zot*(-4 * (ptrtoint.%struct.zot*.i64(%tmp456) /u 4) + 1);
;       |   (%tmp30)[2 * i1 + 1] = &((%tmp460)[0]);
;       |   %tmp462 = inttoptr.i64.%struct.zot*(-4 * (ptrtoint.%struct.zot*.i64(%tmp451) /u 4) + 1);
;       |   (%tmp30)[-2 * i1 + %tmp233 + -1] = &((%tmp462)[0]);
;       |   %tmp465 = (%tmp36)[2 * i1 + 2];
;       |   %tmp470 = (%tmp30)[-2 * i1 + %tmp233 + -2];
;       |   %tmp474 = inttoptr.i64.%struct.zot*(-4 * (ptrtoint.%struct.zot*.i64(%tmp470) /u 4) + 1);
;       |   (%tmp36)[2 * i1 + 2] = &((%tmp474)[0]);
;       |   %tmp476 = inttoptr.i64.%struct.zot*(-4 * (ptrtoint.%struct.zot*.i64(%tmp465) /u 4) + 1);
;       |   (%tmp30)[-2 * i1 + %tmp233 + -2] = &((%tmp476)[0]);
;       + END LOOP
; END REGION

; CHECK: [2 * i1 + 1]
; CHECK-NOT:     [i1 + 1]


; ModuleID = 'ptrtoint'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.zot = type { i64, ptr, [3 x i8], i8 }

; Function Attrs: nounwind uwtable
define dso_local void @spam.bb447(i64 %tmp446, ptr %tmp36, ptr %tmp30, i64 %tmp233) {
newFuncRoot:
  br label %bb447

bb480.exitStub:                                   ; preds = %bb447
  ret void

bb447:                                            ; preds = %newFuncRoot, %bb447
  %tmp448 = phi i64 [ 1, %newFuncRoot ], [ %tmp477, %bb447 ]
  %tmp449 = phi i64 [ %tmp446, %newFuncRoot ], [ %tmp478, %bb447 ]
  %tmp450 = getelementptr ptr, ptr %tmp30, i64 %tmp448
  %tmp451 = load ptr, ptr %tmp450, align 8
  %tmp452 = ptrtoint ptr %tmp451 to i64
  %tmp453 = and i64 %tmp452, -4
  %tmp454 = sub i64 %tmp233, %tmp448
  %tmp455 = getelementptr ptr, ptr %tmp30, i64 %tmp454
  %tmp456 = load ptr, ptr %tmp455, align 8
  %tmp457 = ptrtoint ptr %tmp456 to i64
  %tmp458 = and i64 %tmp457, -4
  %tmp459 = sub i64 1, %tmp458
  %tmp460 = inttoptr i64 %tmp459 to ptr
  store ptr %tmp460, ptr %tmp450, align 8
  %tmp461 = sub i64 1, %tmp453
  %tmp462 = inttoptr i64 %tmp461 to ptr
  store ptr %tmp462, ptr %tmp455, align 8
  %tmp463 = add nuw nsw i64 %tmp448, 1
  %tmp464 = getelementptr ptr, ptr %tmp36, i64 %tmp463
  %tmp465 = load ptr, ptr %tmp464, align 8
  %tmp466 = ptrtoint ptr %tmp465 to i64
  %tmp467 = and i64 %tmp466, -4
  %tmp468 = sub i64 %tmp233, %tmp463
  %tmp469 = getelementptr ptr, ptr %tmp30, i64 %tmp468
  %tmp470 = load ptr, ptr %tmp469, align 8
  %tmp471 = ptrtoint ptr %tmp470 to i64
  %tmp472 = and i64 %tmp471, -4
  %tmp473 = sub i64 1, %tmp472
  %tmp474 = inttoptr i64 %tmp473 to ptr
  store ptr %tmp474, ptr %tmp464, align 8
  %tmp475 = sub i64 1, %tmp467
  %tmp476 = inttoptr i64 %tmp475 to ptr
  store ptr %tmp476, ptr %tmp469, align 8
  %tmp477 = add nuw nsw i64 %tmp448, 2
  %tmp478 = add i64 %tmp449, -1
  %tmp479 = icmp eq i64 %tmp478, 0
  br i1 %tmp479, label %bb480.exitStub, label %bb447
}
