; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose,print<hir>" -hir-temp-array-transpose-max-const-dimsize=500 -disable-output 2>&1 | FileCheck %s

; Check that we successfully transpose the array for non-unit stride access
;  %tmp340 = &((getelementptr inbounds ([291652 x i8], ptr @global.7, i64 0, i64 192000))[i3][i2]);
; We ignore address of refs, as such we cannot remove it as live-in ref. However,
; in most cases, we are replacing all other uses and the SB should no longer
; be marked as live-in.

; HIR Before Transformation
;          BEGIN REGION { }
;                + DO i1 = 0, 8, 1   <DO_LOOP>
;                |   + DO i2 = 0, 8, 1   <DO_LOOP>
;                |   |   %tmp327 = 0;
;                |   |   %tmp328 = 0;
;                |   |
;                |   |   + DO i3 = 0, 8, 1   <DO_LOOP>
;                |   |   |   %tmp331 = (getelementptr inbounds ([291652 x i8], ptr @global.7, i64 0, i64 192000))[i3][i1];
;                |   |   |   if (%tmp331 > %tmp327)
;                |   |   |   {
;                |   |   |      %tmp327 = %tmp331;
;                |   |   |      %tmp328 = (getelementptr inbounds ([291652 x i8], ptr @global.7, i64 0, i64 192000))[i3][i2];
;                |   |   |      %tmp340 = &((getelementptr inbounds ([291652 x i8], ptr @global.7, i64 0, i64 192000))[i3][i2]);
;                |   |   |   }
;                |   |   |   else
;                |   |   |   {
;                |   |   |      %tmp340 = &((getelementptr inbounds ([291652 x i8], ptr @global.7, i64 0, i64 192000))[i3][i2]);
;                |   |   |   }
;                |   |   |   if (%tmp331 >= %tmp327 & %tmp344 > %tmp328)
;                |   |   |   {
;                |   |   |      %tmp327 = %tmp331;
;                |   |   |   }
;                |   |   + END LOOP
;                |   + END LOOP
;                + END LOOP
;          END REGION


; CHECK:   BEGIN REGION { modified }
; CHECK:         %call = @llvm.stacksave.p0();
;                %TranspTmpArr = alloca 96000;
;
; CHECK:         + DO i1 = 0, 499, 1   <DO_LOOP>
; CHECK:         |   + DO i2 = 0, 47, 1   <DO_LOOP>
; CHECK:         |   |   (%TranspTmpArr)[i1][i2] = (getelementptr inbounds ([291652 x i8], ptr @global.7, i64 0, i64 192000))[i2][i1];
;                |   + END LOOP
;                + END LOOP
;
;
; CHECK:         + DO i1 = 0, 8, 1   <DO_LOOP>
; CHECK:         |   + DO i2 = 0, 8, 1   <DO_LOOP>
;                |   |   %tmp327 = 0;
;                |   |   %tmp328 = 0;
;                |   |
; CHECK:         |   |   + DO i3 = 0, 8, 1   <DO_LOOP>
; CHECK:         |   |   |   %tmp331 = (%TranspTmpArr)[i1][i3];
;                |   |   |   if (%tmp331 > %tmp327)
;                |   |   |   {
;                |   |   |      %tmp327 = %tmp331;
; CHECK:         |   |   |      %tmp328 = (%TranspTmpArr)[i2][i3];
;                |   |   |      %tmp340 = &((getelementptr inbounds ([291652 x i8], ptr @global.7, i64 0, i64 192000))[i3][i2]);
;                |   |   |   }
;                |   |   |   else
;                |   |   |   {
;                |   |   |      %tmp340 = &((getelementptr inbounds ([291652 x i8], ptr @global.7, i64 0, i64 192000))[i3][i2]);
;                |   |   |   }
;                |   |   |   if (%tmp331 >= %tmp327 & %tmp344 > %tmp328)
;                |   |   |   {
;                |   |   |      %tmp327 = %tmp331;
;                |   |   |   }
;                |   |   + END LOOP
;                |   + END LOOP
;                + END LOOP
;
; CHECK:         @llvm.stackrestore.p0(&((%call)[0]));
;          END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global.7 = external global [291652 x i8]

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

define void @baz(i32 %tmp344) {
bb:
  br label %bb315

bb315:                                            ; preds = %bb361, %bb
  %tmp311 = phi i64 [ 1, %bb ], [ %tmp363, %bb361 ]
  br label %bb323

bb323:                                            ; preds = %bb355, %bb315
  %tmp320 = phi i64 [ 1, %bb315 ], [ %tmp357, %bb355 ]
  br label %bb325

bb325:                                            ; preds = %bb349, %bb323
  %tmp326 = phi i64 [ 1, %bb323 ], [ %tmp352, %bb349 ]
  %tmp327 = phi i32 [ 0, %bb323 ], [ %tmp350, %bb349 ]
  %tmp328 = phi i32 [ 0, %bb323 ], [ %tmp342, %bb349 ]
  %tmp329 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 2000, ptr elementtype(i32) getelementptr inbounds ([291652 x i8], ptr @global.7, i64 0, i64 192000), i64 %tmp326), !ifx.array_extent !0
  %tmp330 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %tmp329, i64 %tmp311)
  %tmp331 = load i32, ptr %tmp330, align 4
  %tmp332 = icmp sgt i32 %tmp331, %tmp327
  br i1 %tmp332, label %bb335, label %bb333

bb333:                                            ; preds = %bb325
  %tmp334 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 0, ptr elementtype(i32) %tmp329, i64 %tmp320)
  br label %bb339

bb335:                                            ; preds = %bb325
  %tmp336 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %tmp329, i64 %tmp320)
  %tmp337 = load i32, ptr %tmp336, align 4
  br label %bb339

bb339:                                            ; preds = %bb335, %bb333
  %tmp340 = phi ptr [ %tmp334, %bb333 ], [ %tmp336, %bb335 ]
  %tmp341 = phi i32 [ %tmp327, %bb333 ], [ %tmp331, %bb335 ]
  %tmp342 = phi i32 [ %tmp328, %bb333 ], [ %tmp337, %bb335 ]
  %tmp343 = icmp sge i32 %tmp331, %tmp341
  %tmp3441 = load i32, ptr %tmp340, align 4
  %tmp345 = icmp sgt i32 %tmp344, %tmp342
  %tmp346 = and i1 %tmp343, %tmp345
  br i1 %tmp346, label %bb347, label %bb349

bb347:                                            ; preds = %bb339
  br label %bb349

bb349:                                            ; preds = %bb347, %bb339
  %tmp350 = phi i32 [ %tmp331, %bb347 ], [ %tmp341, %bb339 ]
  %tmp352 = add nuw nsw i64 %tmp326, 1
  %tmp353 = icmp eq i64 %tmp352, 10
  br i1 %tmp353, label %bb355, label %bb325

bb355:                                            ; preds = %bb349
  %tmp357 = add nuw nsw i64 %tmp320, 1
  %tmp358 = icmp eq i64 %tmp357, 10
  br i1 %tmp358, label %bb361, label %bb323

bb361:                                            ; preds = %bb355
  %tmp363 = add nuw nsw i64 %tmp311, 1
  %tmp364 = trunc i64 %tmp363 to i32
  %tmp365 = icmp eq i32 10, %tmp364
  br i1 %tmp365, label %bb366, label %bb315

bb366:                                            ; preds = %bb361
  ret void
}

attributes #0 = { nounwind readnone speculatable }

!0 = !{i64 48}
