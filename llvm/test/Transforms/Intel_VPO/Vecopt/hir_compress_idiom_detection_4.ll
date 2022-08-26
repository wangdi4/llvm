; INTEL_FEATURE_SW_ADVANCED
; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -print-after=hir-vplan-vec -vplan-force-vf=8 2>&1 | FileCheck %s

; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -disable-vplan-codegen -vplan-cost-model-print-analysis-for-vf=4 -enable-intel-advanced-opts 2>&1 | FileCheck %s --check-prefix=CM4
; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -enable-compress-expand-idiom -hir-vplan-vec -disable-vplan-codegen -vplan-cost-model-print-analysis-for-vf=8 -enable-intel-advanced-opts 2>&1 | FileCheck %s --check-prefix=CM8

; <0>          BEGIN REGION { }
; <28>               + DO i1 = 0, zext.i32.i64(%N) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <6>                |   if ((%C)[i1] != 0)
; <6>                |   {
; <11>               |      %1 = (%A)[i1];
; <14>               |      (%B)[%j.014] = %1;
; <16>               |      %j.014 = %j.014 + 2  +  3;
; <18>               |      (%B)[%j.014] = %1;
; <19>               |      %j.014 = 4  +  %j.014;
; <6>                |   }
; <28>               + END LOOP
; <0>          END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+5 detected: <16>         [[J_0140:%.*]] = [[J_0140]] + 2  +  3
; CHECK:      [Compress/Expand Idiom] Increment {sb:3}+4 detected: <19>         [[J_0140]] = 4  +  [[J_0140]]
; CHECK-NOT:  [Compress/Expand Idiom] Increment rejected
; CHECK:      Idiom List
; CHECK-NEXT: CEIndexIncFirst: <16>         [[J_0140]] = [[J_0140]] + 2  +  3
; CHECK-NEXT:   CEIndexIncNext: <19>         [[J_0140]] = 4  +  [[J_0140]]
; CHECK-DAG:    CEStore: <14>         ([[B0:%.*]])[%j.014] = [[TMP1:%.*]];
; CHECK-DAG:      CELdStIndex: [[J_0140]]
; CHECK-DAG:    CEStore: <18>         ([[B0]])[%j.014] = [[TMP1]]
; CHECK-DAG:      CELdStIndex: [[J_0140]]

; CHECK-LABEL: VPlan after importing plain CFG:
; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: ? BinOp: i64 [[VP5:%.*]] = add i64 [[VP6:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP6]], i64 [[VP5]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: i32 [[VP7:%.*]] = phi  [ i32 [[J_0140]], [[BB1:BB[0-9]+]] ],  [ i32 [[VP8:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: i32 [[J_0140]]
; CHECK-NEXT:    TotalStride: 9
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP9:%.*]] = add i32 [[VP10:%.*]] i32 3
; CHECK-NEXT:      i32 [[VP11:%.*]] = add i32 4 i32 [[VP9]]
; CHECK-NEXT:    Stores:
; CHECK-NEXT:      store double [[VP_LOAD:%.*]] double* [[VP_SUBSCRIPT:%.*]]
; CHECK-NEXT:      store double [[VP_LOAD]] double* [[VP_SUBSCRIPT_1:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP12:%.*]] = sext i32 [[VP9]] to i64
; CHECK-NEXT:      i64 [[VP13:%.*]] = sext i32 [[VP7]] to i64
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP7]], i32 [[J_0140]], i32 [[VP9]], i32 [[VP11]], void [[VP_STORE:%.*]], void [[VP_STORE_1:%.*]], i64 [[VP12]], i64 [[VP13]],

; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:         [[BB3:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB3]]
; CHECK-NEXT:     i64 [[VP14:%.*]] = add i64 [[VP2:%.*]] i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     i32 [[VP17:%.*]] = compress-expand-index-init i32 [[J_0140]]
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2]]
; CHECK-NEXT:     i32 [[VP7]] = phi  [ i32 [[VP17]], [[BB1]] ],  [ i32 [[VP21:%.*]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP6]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP5]], [[BB2]] ]
; CHECK-NEXT:     i32* [[VP_SUBSCRIPT_2:%.*]] = subscript inbounds i32* [[C0:%.*]] i64 [[VP6]]
; CHECK-NEXT:     i32 [[VP_LOAD_1:%.*]] = load i32* [[VP_SUBSCRIPT_2]]
; CHECK-NEXT:     i1 [[VP15:%.*]] = icmp ne i32 [[VP_LOAD_1]] i32 0
; CHECK-NEXT:     br i1 [[VP15]], [[BB4:BB[0-9]+]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB4]]: # preds: [[BB0]]
; CHECK-NEXT:       double* [[VP_SUBSCRIPT_3:%.*]] = subscript inbounds double* [[A0:%.*]] i64 [[VP6]]
; CHECK-NEXT:       double [[VP_LOAD]] = load double* [[VP_SUBSCRIPT_3]]
; CHECK-NEXT:       i64 [[VP13]] = sext i32 [[VP7]] to i64
; CHECK-NEXT:       i64 [[VP18:%.*]] = compress-expand-index i64 [[VP13]] i64 9
; CHECK-NEXT:       double* [[VP_SUBSCRIPT_1]] = subscript inbounds double* [[B0]] i64 [[VP18]]
; CHECK-NEXT:       compress-store-nonu double [[VP_LOAD]] double* [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:       i32 [[VP10]] = add i32 [[VP7]] i32 2
; CHECK-NEXT:       i32 [[VP9]] = add i32 [[VP10]] i32 3
; CHECK-NEXT:       i64 [[VP12]] = sext i32 [[VP9]] to i64
; CHECK-NEXT:       i64 [[VP20:%.*]] = compress-expand-index i64 [[VP12]] i64 9
; CHECK-NEXT:       double* [[VP_SUBSCRIPT]] = subscript inbounds double* [[B0]] i64 [[VP20]]
; CHECK-NEXT:       compress-store-nonu double [[VP_LOAD]] double* [[VP_SUBSCRIPT]]
; CHECK-NEXT:       i32 [[VP11]] = add i32 4 i32 [[VP9]]
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB4]], [[BB0]]
; CHECK-NEXT:     i32 [[VP8]] = phi  [ i32 [[VP11]], [[BB4]] ],  [ i32 [[VP7]], [[BB0]] ]
; CHECK-NEXT:     i32 [[VP21]] = compress-expand-index-inc i32 [[VP8]]
; CHECK-NEXT:     i64 [[VP5]] = add i64 [[VP6]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP16:%.*]] = icmp slt i64 [[VP5]] i64 [[VP14]]
; CHECK-NEXT:     br i1 [[VP16]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB2]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     i32 [[VP22:%.*]] = compress-expand-index-final i32 [[VP21]]
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[TGU0:%.*]] = zext.i32.i64([[N0:%.*]])  /u  8
; CHECK-NEXT:        [[VEC_TC0:%.*]] = [[TGU0]]  *  8
; CHECK-NEXT:        [[DOTVEC0:%.*]] = 0 == [[VEC_TC0]]
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = 0
; CHECK-NEXT:        [[PHI_TEMP10:%.*]] = [[J_0140]]
; CHECK-NEXT:        [[EXTRACT_0_0:%.*]] = extractelement [[DOTVEC0]],  0
; CHECK-NEXT:        if ([[EXTRACT_0_0]] == 1)
; CHECK-NEXT:        {
; CHECK-NEXT:           goto [[MERGE_BLK0:merge.blk[0-9]+]].40
; CHECK-NEXT:        }
; CHECK-NEXT:        [[TGU30:%.*]] = zext.i32.i64([[N0]])  /u  8
; CHECK-NEXT:        [[VEC_TC40:%.*]] = [[TGU30]]  *  8
; CHECK-NEXT:        [[INSERT0:%.*]] = insertelement zeroinitializer,  [[J_0140]],  0
; CHECK-NEXT:        [[PHI_TEMP50:%.*]] = [[INSERT0]]
; CHECK-NEXT:        [[LOOP_UB0:%.*]] = [[VEC_TC40]]  -  1
; CHECK:             + DO i1 = 0, [[LOOP_UB0]], 8   <DO_LOOP>  <MAX_TC_EST = 268435455>  <LEGAL_MAX_TC = 268435455> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC90:%.*]] = undef
; CHECK-NEXT:        |   [[DOTVEC70:%.*]] = (<8 x i32>*)([[C0]])[i1]
; CHECK-NEXT:        |   [[DOTVEC80:%.*]] = [[DOTVEC70]] != 0
; CHECK-NEXT:        |   [[DOTVEC90]] = (<8 x double>*)([[A0]])[i1], Mask = @{[[DOTVEC80]]}
; CHECK-NEXT:        |   [[SHUFFLE0:%.*]] = shufflevector [[PHI_TEMP50]],  undef,  zeroinitializer
; CHECK-NEXT:        |   [[ADD0:%.*]] = [[SHUFFLE0]]  +  <i64 0, i64 9, i64 18, i64 27, i64 36, i64 45, i64 54, i64 63>
; CHECK-NEXT:        |   [[COMPRESS0:%.*]] = @llvm.x86.avx512.mask.compress.v8f64([[DOTVEC90]],  undef,  [[DOTVEC80]])
; CHECK-NEXT:        |   [[CAST0:%.*]] = bitcast.<8 x i1>.i8([[DOTVEC80]])
; CHECK-NEXT:        |   [[POPCNT0:%.*]] = @llvm.ctpop.i8([[CAST0]])
; CHECK-NEXT:        |   [[SHL0:%.*]] = -1  <<  [[POPCNT0]]
; CHECK-NEXT:        |   [[XOR0:%.*]] = [[SHL0]]  ^  -1
; CHECK-NEXT:        |   [[CAST100:%.*]] = bitcast.i8.<8 x i1>([[XOR0]])
; CHECK-NEXT:        |   @llvm.masked.scatter.v8f64.v8p0f64([[COMPRESS0]],  &((<8 x double*>)([[B0]])[%add]),  0,  [[CAST100]])
; CHECK-NEXT:        |   [[SHUFFLE110:%.*]] = shufflevector [[PHI_TEMP50]] + 5,  undef,  zeroinitializer
; CHECK-NEXT:        |   [[ADD120:%.*]] = [[SHUFFLE110]]  +  <i64 0, i64 9, i64 18, i64 27, i64 36, i64 45, i64 54, i64 63>
; CHECK-NEXT:        |   [[COMPRESS130:%.*]] = @llvm.x86.avx512.mask.compress.v8f64([[DOTVEC90]],  undef,  [[DOTVEC80]])
; CHECK-NEXT:        |   [[CAST140:%.*]] = bitcast.<8 x i1>.i8([[DOTVEC80]])
; CHECK-NEXT:        |   [[POPCNT150:%.*]] = @llvm.ctpop.i8([[CAST140]])
; CHECK-NEXT:        |   [[SHL160:%.*]] = -1  <<  [[POPCNT150]]
; CHECK-NEXT:        |   [[XOR170:%.*]] = [[SHL160]]  ^  -1
; CHECK-NEXT:        |   [[CAST180:%.*]] = bitcast.i8.<8 x i1>([[XOR170]])
; CHECK-NEXT:        |   @llvm.masked.scatter.v8f64.v8p0f64([[COMPRESS130]],  &((<8 x double*>)([[B0]])[%add12]),  0,  [[CAST180]])
; CHECK-NEXT:        |   [[SELECT0:%.*]] = ([[DOTVEC80]] == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? [[PHI_TEMP50]] + 9 : [[PHI_TEMP50]]
; CHECK-NEXT:        |   [[VEC_REDUCE0:%.*]] = @llvm.vector.reduce.add.v8i32([[SELECT0]])
; CHECK-NEXT:        |   [[INSERT190:%.*]] = insertelement zeroinitializer,  [[VEC_REDUCE0]],  0
; CHECK-NEXT:        |   [[PHI_TEMP50]] = [[INSERT190]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[IND_FINAL0:%.*]] = [[LOOP_UB0]]  +  1
; CHECK-NEXT:        [[EXTRACT_0_210:%.*]] = extractelement [[INSERT190]],  0
; CHECK-NEXT:        [[J_0140]] = [[EXTRACT_0_210]]
; CHECK-NEXT:        [[DOTVEC230:%.*]] = zext.i32.i64([[N0]]) == [[VEC_TC40]]
; CHECK-NEXT:        [[PHI_TEMP0]] = [[IND_FINAL0]]
; CHECK-NEXT:        [[PHI_TEMP10]] = [[EXTRACT_0_210]]
; CHECK-NEXT:        [[PHI_TEMP260:%.*]] = [[IND_FINAL0]]
; CHECK-NEXT:        [[PHI_TEMP280:%.*]] = [[EXTRACT_0_210]]
; CHECK-NEXT:        [[EXTRACT_0_300:%.*]] = extractelement [[DOTVEC230]],  0
; CHECK-NEXT:        if ([[EXTRACT_0_300]] == 1)
; CHECK-NEXT:        {
; CHECK-NEXT:           goto [[FINAL_MERGE:final.merge.[0-9]+]]
; CHECK-NEXT:        }
; CHECK-NEXT:        [[MERGE_BLK0]].40:
; CHECK-NEXT:        [[LB_TMP0:%.*]] = [[PHI_TEMP0]]
; CHECK-NEXT:        [[J_0140]] = [[PHI_TEMP10]]
; CHECK:             + DO i1 = [[LB_TMP0]], zext.i32.i64([[N0]]) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 7>  <LEGAL_MAX_TC = 7> <nounroll> <novectorize> <max_trip_count = 7>
; CHECK-NEXT:        |   if (([[C0]])[i1] != 0)
; CHECK-NEXT:        |   {
; CHECK-NEXT:        |      [[TMP1]] = ([[A0]])[i1]
; CHECK-NEXT:        |      ([[B0]])[%j.014] = [[TMP1]]
; CHECK-NEXT:        |      [[J_0140]] = [[J_0140]] + 2  +  3
; CHECK-NEXT:        |      ([[B0]])[%j.014] = [[TMP1]]
; CHECK-NEXT:        |      [[J_0140]] = 4  +  [[J_0140]]
; CHECK-NEXT:        |   }
; CHECK-NEXT:        + END LOOP
; CHECK:             [[PHI_TEMP260]] = zext.i32.i64([[N0]]) + -1
; CHECK-NEXT:        [[PHI_TEMP280]] = [[J_0140]]
; CHECK-NEXT:        [[FINAL_MERGE]]:
; CHECK-NEXT:  END REGION

; CM4: Cost 1 for i32 [[VP_INIT:%.*]] = compress-expand-index-init i32 live-in1
; CM4: Cost 2 for i64 [[VP9:%.*]] = compress-expand-index i64 [[VP8:%.*]] i64 9
; CM4: Cost 10 for compress-store-nonu double [[VP_LOAD_1:%.*]] double* [[VP_SUBSCRIPT_2:%.*]] *GS*
; CM4: Cost 2 for i64 [[VP13:%.*]] = compress-expand-index i64 [[VP12:%.*]] i64 9
; CM4: Cost 5 for compress-store-nonu double [[VP_LOAD_1]] double* [[VP_SUBSCRIPT_3:%.*]] *GS*
; CM4: Block total cost includes GS Cost: 8
; CM4: Cost 4 for i32 [[VP3:%.*]] = compress-expand-index-inc i32 [[VP__BLEND_BB4:%.*]]
; CM4: Cost Unknown for i32 [[VP_FINAL:%.*]] = compress-expand-index-final i32 [[VP3]]

; CM8: Cost 1 for i32 [[VP_INIT:%.*]] = compress-expand-index-init i32 live-in1
; CM8: Cost 3 for i64 [[VP9:%.*]] = compress-expand-index i64 [[VP8:%.*]] i64 9
; CM8: Cost 14 for compress-store-nonu double [[VP_LOAD_1:%.*]] double* [[VP_SUBSCRIPT_2:%.*]] *GS*
; CM8: Cost 3 for i64 [[VP13:%.*]] = compress-expand-index i64 [[VP12:%.*]] i64 9
; CM8: Cost 9 for compress-store-nonu double [[VP_LOAD_1]] double* [[VP_SUBSCRIPT_3:%.*]] *GS*
; CM8: Block total cost includes GS Cost: 16
; CM8: Cost 6 for i32 [[VP3:%.*]] = compress-expand-index-inc i32 [[VP__BLEND_BB4:%.*]]
; CM8: Cost Unknown for i32 [[VP_FINAL:%.*]] = compress-expand-index-final i32 [[VP3]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind uwtable
define dso_local void @_Z3fooPdS_Pii(double* noalias nocapture noundef readonly %A, double* noalias nocapture noundef writeonly %B, i32* noalias nocapture noundef readonly %C, i32 noundef %N) local_unnamed_addr #0 {
entry:
  %cmp13 = icmp sgt i32 %N, 0
  br i1 %cmp13, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count16 = zext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %j.014 = phi i32 [ 0, %for.body.preheader ], [ %j.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %C, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %cmp1.not = icmp eq i32 %0, 0
  br i1 %cmp1.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx3 = getelementptr inbounds double, double* %A, i64 %indvars.iv
  %1 = load double, double* %arrayidx3, align 8, !tbaa !7
  %idxprom4 = sext i32 %j.014 to i64
  %arrayidx5 = getelementptr inbounds double, double* %B, i64 %idxprom4
  store double %1, double* %arrayidx5, align 8, !tbaa !7
  %inc0 = add nsw i32 2, %j.014
  %inc1 = add nsw i32 %inc0, 3
  %arrayidx6 = getelementptr inbounds double, double* %B, i32 %inc1
  store double %1, double* %arrayidx6
  %inc = add nsw i32 4, %inc1
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %j.1 = phi i32 [ %inc, %if.then ], [ %j.014, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count16
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !9
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"double", !5, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
; end INTEL_FEATURE_SW_ADVANCED
