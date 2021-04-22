; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-reroll  -print-after=hir-loop-reroll  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s

; Verify that ptrtoint/inttoptr can be handled if applicable.

; CHECK-NOT: [2 * i1 + 1]
; CHECK:     [i1 + 1]

; *** IR Dump Before HIR Loop Reroll ***
; Function: spam.bb447
;
;          BEGIN REGION { }
;                + DO i1 = 0, %tmp446 + -1, 1   <DO_LOOP>
;                |   %tmp451 = (%tmp36)[2 * i1 + 1];
;                |   %tmp456 = (%tmp36)[-2 * i1 + %tmp233 + -1];
;                |   %tmp460 = inttoptr.i64.%struct.zot*(-4 * (ptrtoint.%struct.zot*.i64(%tmp456) /u 4) + 1);
;                |   (%tmp36)[2 * i1 + 1] = &((%tmp460)[0]);
;                |   %tmp462 = inttoptr.i64.%struct.zot*(-4 * (ptrtoint.%struct.zot*.i64(%tmp451) /u 4) + 1);
;                |   (%tmp36)[-2 * i1 + %tmp233 + -1] = &((%tmp462)[0]);
;                |   %tmp465 = (%tmp36)[2 * i1 + 2];
;                |   %tmp470 = (%tmp36)[-2 * i1 + %tmp233 + -2];
;                |   %tmp474 = inttoptr.i64.%struct.zot*(-4 * (ptrtoint.%struct.zot*.i64(%tmp470) /u 4) + 1);
;                |   (%tmp36)[2 * i1 + 2] = &((%tmp474)[0]);
;                |   %tmp476 = inttoptr.i64.%struct.zot*(-4 * (ptrtoint.%struct.zot*.i64(%tmp465) /u 4) + 1);
;                |   (%tmp36)[-2 * i1 + %tmp233 + -2] = &((%tmp476)[0]);
;                + END LOOP
;          END REGION
;
; *** IR Dump After HIR Loop Reroll ***
; Function: spam.bb447
;
;         BEGIN REGION { }
;               + DO i1 = 0, 2 * %tmp446 + -1, 1   <DO_LOOP>
;               |   %tmp451 = (%tmp36)[i1 + 1];
;               |   %tmp456 = (%tmp36)[-1 * i1 + %tmp233 + -1];
;               |   %tmp460 = inttoptr.i64.%struct.zot*(-4 * (ptrtoint.%struct.zot*.i64(%tmp456) /u 4) + 1);
;               |   (%tmp36)[i1 + 1] = &((%tmp460)[0]);
;               |   %tmp462 = inttoptr.i64.%struct.zot*(-4 * (ptrtoint.%struct.zot*.i64(%tmp451) /u 4) + 1);
;               |   (%tmp36)[-1 * i1 + %tmp233 + -1] = &((%tmp462)[0]);
;               + END LOOP
;           END REGION


; ModuleID = 'ptrtoint'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.zot = type { i64, %struct.zot**, [3 x i8], i8 }

; Function Attrs: nounwind uwtable
define dso_local void @spam.bb447(i64 %tmp446, %struct.zot** %tmp36, i64 %tmp233) #0 {
newFuncRoot:
  br label %bb447

bb480.exitStub:                                   ; preds = %bb447
  ret void

bb447:                                            ; preds = %newFuncRoot, %bb447
  %tmp448 = phi i64 [ 1, %newFuncRoot ], [ %tmp477, %bb447 ]
  %tmp449 = phi i64 [ %tmp446, %newFuncRoot ], [ %tmp478, %bb447 ]
  %tmp450 = getelementptr %struct.zot*, %struct.zot** %tmp36, i64 %tmp448
  %tmp451 = load %struct.zot*, %struct.zot** %tmp450, align 8, !tbaa !5
  %tmp452 = ptrtoint %struct.zot* %tmp451 to i64
  %tmp453 = and i64 %tmp452, -4
  %tmp454 = sub i64 %tmp233, %tmp448
  %tmp455 = getelementptr %struct.zot*, %struct.zot** %tmp36, i64 %tmp454
  %tmp456 = load %struct.zot*, %struct.zot** %tmp455, align 8, !tbaa !5
  %tmp457 = ptrtoint %struct.zot* %tmp456 to i64
  %tmp458 = and i64 %tmp457, -4
  %tmp459 = sub i64 1, %tmp458
  %tmp460 = inttoptr i64 %tmp459 to %struct.zot*
  store %struct.zot* %tmp460, %struct.zot** %tmp450, align 8, !tbaa !5
  %tmp461 = sub i64 1, %tmp453
  %tmp462 = inttoptr i64 %tmp461 to %struct.zot*
  store %struct.zot* %tmp462, %struct.zot** %tmp455, align 8, !tbaa !5
  %tmp463 = add nuw nsw i64 %tmp448, 1
  %tmp464 = getelementptr %struct.zot*, %struct.zot** %tmp36, i64 %tmp463
  %tmp465 = load %struct.zot*, %struct.zot** %tmp464, align 8, !tbaa !5
  %tmp466 = ptrtoint %struct.zot* %tmp465 to i64
  %tmp467 = and i64 %tmp466, -4
  %tmp468 = sub i64 %tmp233, %tmp463
  %tmp469 = getelementptr %struct.zot*, %struct.zot** %tmp36, i64 %tmp468
  %tmp470 = load %struct.zot*, %struct.zot** %tmp469, align 8, !tbaa !5
  %tmp471 = ptrtoint %struct.zot* %tmp470 to i64
  %tmp472 = and i64 %tmp471, -4
  %tmp473 = sub i64 1, %tmp472
  %tmp474 = inttoptr i64 %tmp473 to %struct.zot*
  store %struct.zot* %tmp474, %struct.zot** %tmp464, align 8, !tbaa !5
  %tmp475 = sub i64 1, %tmp467
  %tmp476 = inttoptr i64 %tmp475 to %struct.zot*
  store %struct.zot* %tmp476, %struct.zot** %tmp469, align 8, !tbaa !5
  %tmp477 = add nuw nsw i64 %tmp448, 2
  %tmp478 = add i64 %tmp449, -1
  %tmp479 = icmp eq i64 %tmp478, 0
  br i1 %tmp479, label %bb480.exitStub, label %bb447, !llvm.loop !9
}

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }

!llvm.ident = !{!0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0, !0}
!llvm.module.flags = !{!1, !2, !3, !4}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.2.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 1, !"ThinLTO", i32 0}
!3 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!4 = !{i32 1, !"LTOPostLink", i32 1}
!5 = !{!6, !6, i64 0}
!6 = !{!"pointer@_ZTSP9TypHeader", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
