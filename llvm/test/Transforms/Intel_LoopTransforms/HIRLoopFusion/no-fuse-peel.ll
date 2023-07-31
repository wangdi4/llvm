; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-fusion,print<hir>" -aa-pipeline="basic-aa" -disable-output %s 2>&1 | FileCheck %s

; Check that we do not fuse the following loops due to dependence between 7:46:
; (%arg)[0].0[i1 + 1],  (%arg)[0].0[i1 + 156]
; Effectively the first loop runs iterations 0-155, and the 2nd loop runs iterations
; 156-310. When fused, we get a loop with iterations 0-154 and 156-310.
; However because each iteration reads iteration i1 and i1+1, while storing i1,
; the dependency from 7:46 cannot be handled by peel loop with current fusion.

;<0>          BEGIN REGION { }
;<52>               + DO i1 = 0, 155, 1   <DO_LOOP>
;<3>                |   %tmp11 = (%arg)[0].0[i1];
;<7>                |   %tmp15 = (%arg)[0].0[i1 + 1];
;<9>                |   %tmp17 = 2 * zext.i30.i64(trunc.i64.i30((%tmp15 /u 2)))  |  2147483648 * (%tmp11 /u 2147483648);
;<14>               |   %tmp22 = (%tmp17)/u2  ^  (%arg)[0].0[i1 + 156];
;<17>               |   %tmp25 = (trunc.i64.i1(%tmp15) == 0) ? 0 : -5403634167711393303;
;<18>               |   %tmp26 = %tmp22  ^  %tmp25;
;<19>               |   (%arg)[0].0[i1] = %tmp26;
;<52>               + END LOOP
;<52>
;<53>
;<53>               + DO i1 = 0, 154, 1   <DO_LOOP>
;<30>               |   %tmp32 = (%arg)[0].0[i1 + 156];
;<34>               |   %tmp36 = (%arg)[0].0[i1 + 157];
;<36>               |   %tmp38 = 2 * zext.i30.i64(trunc.i64.i30((%tmp36 /u 2)))  |  2147483648 * (%tmp32 /u 2147483648);
;<41>               |   %tmp43 = (%tmp38)/u2  ^  (%arg)[0].0[i1];
;<44>               |   %tmp46 = (trunc.i64.i1(%tmp36) == 0) ? 0 : -5403634167711393303;
;<45>               |   %tmp47 = %tmp43  ^  %tmp46;
;<46>               |   (%arg)[0].0[i1 + 156] = %tmp47;
;<53>               + END LOOP
;<0>          END REGION

; CHECK: BEGIN REGION
; CHECK-NOT: modified

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.baz.2 = type { [312 x i64], i64 }

$t = comdat any

; Function Attrs: mustprogress uwtable
define weak_odr dso_local noundef i64 @barney(ptr noundef nonnull align 8 dereferenceable(2504) %arg) local_unnamed_addr #0 comdat($t) align 2 {
bb:
  %tmp = getelementptr inbounds %struct.baz.2, ptr %arg, i64 0, i32 1, !intel-tbaa !3
  %tmp1 = load i64, ptr %tmp, align 8, !tbaa !3
  %tmp2 = icmp ugt i64 %tmp1, 311
  br i1 %tmp2, label %bb3, label %bb4

bb3:                                              ; preds = %bb
  br label %bb8

bb4:                                              ; preds = %bb
  %tmp5 = getelementptr inbounds %struct.baz.2, ptr %arg, i64 0, i32 0, i64 %tmp1
  %tmp6 = load i64, ptr %tmp5, align 8, !tbaa !9
  %tmp7 = add nuw nsw i64 %tmp1, 1
  br label %bb65

bb8:                                              ; preds = %bb8, %bb3
  %tmp9 = phi i64 [ %tmp13, %bb8 ], [ 0, %bb3 ]
  %tmp10 = getelementptr inbounds %struct.baz.2, ptr %arg, i64 0, i32 0, i64 %tmp9, !intel-tbaa !9
  %tmp11 = load i64, ptr %tmp10, align 8, !tbaa !9
  %tmp12 = and i64 %tmp11, -2147483648
  %tmp13 = add nuw nsw i64 %tmp9, 1
  %tmp14 = getelementptr inbounds %struct.baz.2, ptr %arg, i64 0, i32 0, i64 %tmp13, !intel-tbaa !9
  %tmp15 = load i64, ptr %tmp14, align 8, !tbaa !9
  %tmp16 = and i64 %tmp15, 2147483646
  %tmp17 = or i64 %tmp16, %tmp12
  %tmp18 = add nuw nsw i64 %tmp9, 156
  %tmp19 = getelementptr inbounds %struct.baz.2, ptr %arg, i64 0, i32 0, i64 %tmp18, !intel-tbaa !9
  %tmp20 = load i64, ptr %tmp19, align 8, !tbaa !9
  %tmp21 = lshr exact i64 %tmp17, 1
  %tmp22 = xor i64 %tmp21, %tmp20
  %tmp23 = and i64 %tmp15, 1
  %tmp24 = icmp eq i64 %tmp23, 0
  %tmp25 = select i1 %tmp24, i64 0, i64 -5403634167711393303
  %tmp26 = xor i64 %tmp22, %tmp25
  store i64 %tmp26, ptr %tmp10, align 8, !tbaa !9
  %tmp27 = icmp eq i64 %tmp13, 156
  br i1 %tmp27, label %bb28, label %bb8, !llvm.loop !10

bb28:                                             ; preds = %bb8
  br label %bb29

bb29:                                             ; preds = %bb29, %bb28
  %tmp30 = phi i64 [ %tmp34, %bb29 ], [ 156, %bb28 ]
  %tmp31 = getelementptr inbounds %struct.baz.2, ptr %arg, i64 0, i32 0, i64 %tmp30, !intel-tbaa !9
  %tmp32 = load i64, ptr %tmp31, align 8, !tbaa !9
  %tmp33 = and i64 %tmp32, -2147483648
  %tmp34 = add nuw nsw i64 %tmp30, 1
  %tmp35 = getelementptr inbounds %struct.baz.2, ptr %arg, i64 0, i32 0, i64 %tmp34, !intel-tbaa !9
  %tmp36 = load i64, ptr %tmp35, align 8, !tbaa !9
  %tmp37 = and i64 %tmp36, 2147483646
  %tmp38 = or i64 %tmp37, %tmp33
  %tmp39 = add nsw i64 %tmp30, -156
  %tmp40 = getelementptr inbounds %struct.baz.2, ptr %arg, i64 0, i32 0, i64 %tmp39, !intel-tbaa !9
  %tmp41 = load i64, ptr %tmp40, align 8, !tbaa !9
  %tmp42 = lshr exact i64 %tmp38, 1
  %tmp43 = xor i64 %tmp42, %tmp41
  %tmp44 = and i64 %tmp36, 1
  %tmp45 = icmp eq i64 %tmp44, 0
  %tmp46 = select i1 %tmp45, i64 0, i64 -5403634167711393303
  %tmp47 = xor i64 %tmp43, %tmp46
  store i64 %tmp47, ptr %tmp31, align 8, !tbaa !9
  %tmp48 = icmp eq i64 %tmp34, 311
  br i1 %tmp48, label %bb49, label %bb29, !llvm.loop !12

bb49:                                             ; preds = %bb29
  %tmp50 = getelementptr inbounds %struct.baz.2, ptr %arg, i64 0, i32 0, i64 311, !intel-tbaa !9
  %tmp51 = load i64, ptr %tmp50, align 8, !tbaa !9
  %tmp52 = and i64 %tmp51, -2147483648
  %tmp53 = getelementptr inbounds %struct.baz.2, ptr %arg, i64 0, i32 0, i64 0, !intel-tbaa !9
  %tmp54 = load i64, ptr %tmp53, align 8, !tbaa !9
  %tmp55 = and i64 %tmp54, 2147483646
  %tmp56 = or i64 %tmp55, %tmp52
  %tmp57 = getelementptr inbounds %struct.baz.2, ptr %arg, i64 0, i32 0, i64 155, !intel-tbaa !9
  %tmp58 = load i64, ptr %tmp57, align 8, !tbaa !9
  %tmp59 = lshr exact i64 %tmp56, 1
  %tmp60 = xor i64 %tmp59, %tmp58
  %tmp61 = and i64 %tmp54, 1
  %tmp62 = icmp eq i64 %tmp61, 0
  %tmp63 = select i1 %tmp62, i64 0, i64 -5403634167711393303
  %tmp64 = xor i64 %tmp60, %tmp63
  store i64 %tmp64, ptr %tmp50, align 8, !tbaa !9
  br label %bb65

bb65:                                             ; preds = %bb49, %bb4
  %tmp66 = phi i64 [ %tmp54, %bb49 ], [ %tmp6, %bb4 ]
  %tmp67 = phi i64 [ 1, %bb49 ], [ %tmp7, %bb4 ]
  store i64 %tmp67, ptr %tmp, align 8, !tbaa !3
  %tmp68 = lshr i64 %tmp66, 29
  %tmp69 = and i64 %tmp68, 22906492245
  %tmp70 = xor i64 %tmp69, %tmp66
  %tmp71 = shl i64 %tmp70, 17
  %tmp72 = and i64 %tmp71, 8202884508482404352
  %tmp73 = xor i64 %tmp72, %tmp70
  %tmp74 = shl i64 %tmp73, 37
  %tmp75 = and i64 %tmp74, -2270628950310912
  %tmp76 = xor i64 %tmp75, %tmp73
  %tmp77 = lshr i64 %tmp76, 43
  %tmp78 = xor i64 %tmp77, %tmp76
  ret i64 %tmp78
}


!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !6, i64 2496}
!4 = !{!"struct@t", !5, i64 0, !6, i64 2496}
!5 = !{!"array@_ZTSA312_m", !6, i64 0}
!6 = !{!"long", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!4, !6, i64 0}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
!12 = distinct !{!12, !11}
