; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-const-ub,print<hir>" -disable-output < %s 2>&1 | FileCheck %s

; Note that %tmp528 has 3 phis which results in the pass trying to MV where
; %tmp528 = 16, resulting in LoopTC = 0. Check that MV does not happen. Prior
; wrong calculation created zero loops which would assert.

; CHECK: BEGIN REGION
; CHECK-NOT: modified
; CHECK-NOT: if (%tmp528 == 16)

; Incoming HIR
;   BEGIN REGION { }
;         + DO i1 = 0, -1 * zext.i32.i64(%tmp528) + 15, 1
;         |   %tmp553.out = %tmp553;
;         |   %tmp555 = 1  <<  i1 + zext.i32.i64(%tmp528);
;         |   %tmp556 = %tmp555  &  %tmp495;
;         |   if (%tmp556 != 0)
;         |   {
;         |      @llvm.memcpy.p0.p0.i64(&((i8*)(%tmp8)[0][%tmp113][%tmp471][%tmp553.out]),  &((i8*)(%tmp10)[0][i1 + zext.i32.i64(%tmp528)]),  104,  0);
;         |   }
;         |   %tmp565 = %tmp472  >>  i1 + zext.i32.i64(%tmp528);
;         |   %tmp553 = %tmp553.out + zext.i1.i32(trunc.i32.i1(%tmp565));
;         + END LOOP
;   END REGION

%struct.snork = type { i32, i32, i32, i32, i32, i32, i32, i32, i32, float, %struct.snork.0, %struct.eggs, %struct.wobble, ptr, ptr, ptr, ptr, ptr, ptr, double }
%struct.snork.0 = type { ptr, ptr, ptr, ptr }
%struct.eggs = type { ptr, ptr, ptr, ptr }
%struct.wobble = type { float}
%struct.quux = type { float, float, float, i32 }
%struct.blam = type { double, double, double, double }
%struct.zot = type { [3 x %struct.quux], i32, [3 x float], [3 x float], [3 x float], [4 x i32] }
%struct.spam.1 = type { <8 x i64> }
%struct.quux.2 = type { <16 x float> }

declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #0

; Function Attrs: uwtable
define dso_local void @wobble.bb469(i16 %tmp460, i16 %tmp461, <16 x i32> %tmp462, <16 x i32> %tmp111, <16 x float> %tmp366, ptr %tmp9, i64 %tmp113, ptr %tmp5, ptr %tmp6, ptr %tmp7, i16 %tmp374, ptr %tmp10, ptr %tmp8, ptr %arg, i32 %tmp96) #2 {
newFuncRoot:
  br label %bb469

bb469:                                            ; preds = %newFuncRoot, %bb571
  %tmp470 = phi i1 [ true, %newFuncRoot ], [ false, %bb571 ]
  %tmp471 = phi i64 [ 0, %newFuncRoot ], [ 1, %bb571 ]
  %tmp472 = select i1 %tmp470, i16 %tmp460, i16 %tmp461
  %tmp473 = bitcast i16 %tmp472 to <16 x i1>
  %tmp477 = getelementptr inbounds [2 x [2 x i32]], ptr %tmp9, i64 0, i64 %tmp113, i64 %tmp471
  %tmp478 = load i32, ptr %tmp477, align 4
  %tmp479 = shl i32 65535, %tmp478
  %tmp480 = trunc i32 %tmp479 to i16
  %tmp481 = getelementptr inbounds [2 x [2 x %struct.spam.1]], ptr %tmp5, i64 0, i64 %tmp113, i64 %tmp471, i32 0
  %tmp483 = load <16 x i32>, ptr %tmp481, align 64
  %tmp484 = bitcast i16 %tmp480 to <16 x i1>
  %tmp486 = bitcast <16 x i32> %tmp483 to <8 x i64>
  store <16 x i32> %tmp483, ptr %tmp481, align 64
  %tmp487 = getelementptr inbounds [2 x [2 x %struct.spam.1]], ptr %tmp6, i64 0, i64 %tmp113, i64 %tmp471, i32 0
  %tmp489 = load <16 x i32>, ptr %tmp487, align 64
  %tmp491 = bitcast <16 x i32> %tmp489 to <8 x i64>
  store <16 x i32> %tmp489, ptr %tmp487, align 64
  %tmp492 = getelementptr inbounds [2 x [2 x %struct.quux.2]], ptr %tmp7, i64 0, i64 %tmp113, i64 %tmp471, i32 0
  %tmp493 = load <16 x float>, ptr %tmp492, align 64
  store <16 x float> %tmp493, ptr %tmp492, align 64
  %tmp495 = and i16 %tmp472, %tmp374
  %tmp496 = icmp eq i16 %tmp495, 0
  br i1 %tmp496, label %bb497, label %bb499

bb497:                                            ; preds = %bb469
  %tmp498 = zext i16 %tmp472 to i32
  br label %bb526

bb499:                                            ; preds = %bb469
  %tmp500 = zext i16 %tmp495 to i32
  %tmp501 = zext i16 %tmp472 to i32
  br label %bb502

bb502:                                            ; preds = %bb517, %bb499
  %tmp503 = phi i64 [ 0, %bb499 ], [ %tmp521, %bb517 ]
  %tmp504 = phi i32 [ %tmp478, %bb499 ], [ %tmp520, %bb517 ]
  %tmp505 = icmp sgt i32 %tmp504, 15
  %tmp506 = trunc i64 %tmp503 to i32
  br i1 %tmp505, label %bb523, label %bb517

bb517:                                            ; preds = %bb502
  %tmp518 = lshr i32 %tmp501, %tmp506
  %tmp519 = and i32 %tmp518, 1
  %tmp520 = add nsw i32 %tmp519, %tmp504
  %tmp521 = add nuw nsw i64 %tmp503, 1
  %tmp522 = icmp eq i64 %tmp521, 16
  br i1 %tmp522, label %bb525, label %bb502

bb523:                                            ; preds = %bb502
  %tmp524 = phi i32 [ %tmp506, %bb502 ]
  br label %bb526

bb525:                                            ; preds = %bb517
  br label %bb526

bb526:                                            ; preds = %bb525, %bb523, %bb497
  %tmp527 = phi i32 [ %tmp498, %bb497 ], [ %tmp501, %bb523 ], [ %tmp501, %bb525 ]
  %tmp528 = phi i32 [ 16, %bb497 ], [ %tmp524, %bb523 ], [ 16, %bb525 ]
  %tmp530 = add nsw i32 %tmp478, %tmp528
  store i32 %tmp530, ptr %tmp477, align 4
  %tmp531 = icmp sgt i32 %tmp530, 15
  br i1 %tmp531, label %bb532, label %bb571

bb532:                                            ; preds = %bb526
  %tmp533 = getelementptr inbounds [2 x [2 x [16 x %struct.zot]]], ptr %tmp8, i64 0, i64 %tmp113, i64 %tmp471, i64 0
  %tmp534 = trunc i64 %tmp471 to i32
  %tmp535 = add nsw i32 %tmp530, -16
  store i32 %tmp535, ptr %tmp477, align 4
  %tmp536 = sub nsw i32 %tmp528, %tmp535
  %tmp537 = sub nsw i32 32, %tmp530
  %tmp538 = lshr i32 65535, %tmp537
  %tmp539 = shl i32 %tmp538, %tmp536
  %tmp540 = trunc i32 %tmp539 to i16
  %tmp541 = bitcast i16 %tmp540 to <16 x i1>
  %tmp545 = xor i1 %tmp496, true
  %tmp546 = zext i16 %tmp495 to i32
  %tmp547 = icmp ult i32 %tmp528, 16
  %tmp548 = select i1 %tmp545, i1 %tmp547, i1 false
  br i1 %tmp548, label %bb549, label %bb571

bb549:                                            ; preds = %bb532
  %tmp550 = zext i32 %tmp528 to i64
  br label %bb551

bb551:                                            ; preds = %bb564, %bb549
  %tmp552 = phi i64 [ %tmp550, %bb549 ], [ %tmp568, %bb564 ]
  %tmp553 = phi i32 [ 0, %bb549 ], [ %tmp567, %bb564 ]
  %tmp554 = trunc i64 %tmp552 to i32
  %tmp555 = shl nuw nsw i32 1, %tmp554
  %tmp556 = and i32 %tmp555, %tmp546
  %tmp557 = icmp eq i32 %tmp556, 0
  br i1 %tmp557, label %bb564, label %bb558

bb558:                                            ; preds = %bb551
  %tmp559 = getelementptr inbounds [16 x %struct.zot], ptr %tmp10, i64 0, i64 %tmp552
  %tmp560 = zext i32 %tmp553 to i64
  %tmp561 = getelementptr inbounds [2 x [2 x [16 x %struct.zot]]], ptr %tmp8, i64 0, i64 %tmp113, i64 %tmp471, i64 %tmp560
  call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 8 dereferenceable(104) %tmp561, ptr noundef nonnull align 8 dereferenceable(104) %tmp559, i64 104, i1 false)
  br label %bb564

bb564:                                            ; preds = %bb558, %bb551
  %tmp565 = lshr i32 %tmp527, %tmp554
  %tmp566 = and i32 %tmp565, 1
  %tmp567 = add nuw nsw i32 %tmp566, %tmp553
  %tmp568 = add nuw nsw i64 %tmp552, 1
  %tmp569 = icmp eq i64 %tmp568, 16
  br i1 %tmp569, label %bb570, label %bb551

bb570:                                            ; preds = %bb564
  br label %bb571

bb571:                                            ; preds = %bb570, %bb532, %bb526
  br i1 %tmp470, label %bb469, label %bb463.exitStub

bb463.exitStub:                                   ; preds = %bb571
  ret void
}
