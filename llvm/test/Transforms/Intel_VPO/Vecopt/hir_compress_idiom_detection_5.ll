; RUN: opt %s -mattr=+avx512f,+avx512vl -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -disable-output -debug-only=parvec-analysis -hir-vplan-vec -vplan-print-after-plain-cfg -vplan-print-after-vpentity-instrs -vplan-entities-dump -print-after=hir-vplan-vec -vplan-force-vf=8 2>&1 | FileCheck %s

; <0>          BEGIN REGION { }
; <25>               + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <6>                |   if ((%a)[i1] != 0)
; <6>                |   {
; <13>               |      %1 = (%f)[%c.014];
; <10>               |      %c.014 = %c.014  +  1;
; <14>               |      %r.015 = %1  +  %r.015;
; <6>                |   }
; <25>               + END LOOP
; <0>          END REGION

; CHECK:      [Compress/Expand Idiom] Increment {sb:4}+1 detected: <10>         [[C_0140:%.*]] = [[C_0140]]  +  1
; CHECK-NOT:  [Compress/Expand Idiom] Increment rejected
; CHECK-NEXT: Idiom List
; CHECK-NEXT: CEIndexIncFirst: <10>         [[C_0140]] = [[C_0140]]  +  1
; CHECK-NEXT:   CELoad: ([[F0:%.*]])[%c.014]
; CHECK-NEXT:     CELdStIndex: [[C_0140]]

; CHECK-LABEL: VPlan after importing plain CFG:
; CHECK:       Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:  Reduction list
; CHECK-NEXT:   (+) Start: float [[R_0150:%.*]] Exit: float [[VP5:%.*]]
; CHECK-NEXT:    Linked values: float [[VP6:%.*]], float [[VP5]], float [[VP7:%.*]],
; CHECK:       Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: ? BinOp: i64 [[VP8:%.*]] = add i64 [[VP9:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP9]], i64 [[VP8]],
; CHECK:       Compress/expand idiom list
; CHECK-NEXT:    Phi: i32 [[VP10:%.*]] = phi  [ i32 [[C_0140]], [[BB1:BB[0-9]+]] ],  [ i32 [[VP11:%.*]], [[BB2:BB[0-9]+]] ]
; CHECK-NEXT:    LiveIn: i32 [[C_0140]]
; CHECK-NEXT:    LiveOut: i32 [[VP11]] = phi  [ i32 [[VP12:%.*]], [[BB3:BB[0-9]+]] ],  [ i32 [[VP10]], [[BB0]] ]
; CHECK-NEXT:    TotalStride: 1
; CHECK-NEXT:    Increments:
; CHECK-NEXT:      i32 [[VP12]] = add i32 [[VP10]] i32 1
; CHECK-NEXT:    Loads:
; CHECK-NEXT:      float [[VP_LOAD:%.*]] = load float* [[VP_SUBSCRIPT:%.*]]
; CHECK-NEXT:    Indices:
; CHECK-NEXT:      i64 [[VP13:%.*]] = sext i32 [[VP10]] to i64
; CHECK-EMPTY:
; CHECK-NEXT:    Linked values: i32 [[VP10]], i32 [[C_0140]], i32 [[VP11]], i32 [[VP12]], float [[VP_LOAD]], i64 [[VP13]],

; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:         [[BB4:BB[0-9]+]]: # preds:
; CHECK-NEXT:     br [[BB1]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB1]]: # preds: [[BB4]]
; CHECK-NEXT:     i64 [[VP14:%.*]] = add i64 [[VP4:%.*]] i64 1
; CHECK-NEXT:     float [[VP_RED_INIT:%.*]] = reduction-init float 0.000000e+00
; CHECK-NEXT:     i64 [[VP__IND_INIT:%.*]] = induction-init{add} i64 0 i64 1
; CHECK-NEXT:     i64 [[VP__IND_INIT_STEP:%.*]] = induction-init-step{add} i64 1
; CHECK-NEXT:     i32 [[VP19:%.*]] = compress-expand-index-init i32 [[C_0140]]
; CHECK-NEXT:     br [[BB0]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB0]]: # preds: [[BB1]], [[BB2:BB[0-9]+]]
; CHECK-NEXT:     float [[VP6]] = phi  [ float [[VP_RED_INIT]], [[BB1]] ],  [ float [[VP5:%.*]], [[BB2]] ]
; CHECK-NEXT:     i32 [[VP10]] = phi  [ i32 [[VP19]], [[BB1]] ],  [ i32 [[VP21:%.*]], [[BB2]] ]
; CHECK-NEXT:     i64 [[VP9]] = phi  [ i64 [[VP__IND_INIT]], [[BB1]] ],  [ i64 [[VP8]], [[BB2]] ]
; CHECK-NEXT:     i32* [[VP_SUBSCRIPT_1:%.*]] = subscript inbounds i32* [[A0:%.*]] i64 [[VP9]]
; CHECK-NEXT:     i32 [[VP_LOAD_1:%.*]] = load i32* [[VP_SUBSCRIPT_1]]
; CHECK-NEXT:     i1 [[VP15:%.*]] = icmp ne i32 [[VP_LOAD_1]] i32 0
; CHECK-NEXT:     br i1 [[VP15]], [[BB3]], [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:      [[BB3]]: # preds: [[BB0]]
; CHECK-NEXT:       i64 [[VP13]] = sext i32 [[VP10]] to i64
; CHECK-NEXT:       float* [[VP_SUBSCRIPT]] = subscript inbounds float* [[F0]] i64 [[VP13]]
; CHECK-NEXT:       float [[VP22:%.*]] = expand-load float* [[VP_SUBSCRIPT]]
; CHECK-NEXT:       i32 [[VP12]] = add i32 [[VP10]] i32 1
; CHECK-NEXT:       float [[VP7]] = fadd float [[VP22]] float [[VP6]]
; CHECK-NEXT:       br [[BB2]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB2]]: # preds: [[BB3]], [[BB0]]
; CHECK-NEXT:     float [[VP5]] = phi  [ float [[VP7]], [[BB3]] ],  [ float [[VP6]], [[BB0]] ]
; CHECK-NEXT:     i32 [[VP11]] = phi  [ i32 [[VP12]], [[BB3]] ],  [ i32 [[VP10]], [[BB0]] ]
; CHECK-NEXT:     i32 [[VP21:%.*]] = compress-expand-index-inc i32 [[VP11]]
; CHECK-NEXT:     i64 [[VP8]] = add i64 [[VP9]] i64 [[VP__IND_INIT_STEP]]
; CHECK-NEXT:     i1 [[VP16:%.*]] = icmp slt i64 [[VP8]] i64 [[VP14]]
; CHECK-NEXT:     br i1 [[VP16]], [[BB0]], [[BB5:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB5]]: # preds: [[BB2]]
; CHECK-NEXT:     float [[VP_RED_FINAL:%.*]] = reduction-final{fadd} float [[VP5]] float [[R_0150]]
; CHECK-NEXT:     i64 [[VP__IND_FINAL:%.*]] = induction-final{add} i64 0 i64 1
; CHECK-NEXT:     i32 [[VP20:%.*]] = compress-expand-index-final i32 [[VP21]]
; CHECK-NEXT:     br [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB5]]
; CHECK-NEXT:     br <External Block>
; CHECK-EMPTY:
; CHECK-NEXT:  External Uses:
; CHECK-NEXT:  Id: 0   i32 [[VP20]] -> [[VP17:%.*]] = {%c.014}
; CHECK-EMPTY:
; CHECK-NEXT:  Id: 1   float [[VP_RED_FINAL]] -> [[VP18:%.*]] = {%r.015}

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[TGU0:%.*]] = zext.i32.i64([[N0:%.*]])  /u  8
; CHECK-NEXT:        [[VEC_TC0:%.*]] = [[TGU0]]  *  8
; CHECK-NEXT:        [[DOTVEC0:%.*]] = 0 == [[VEC_TC0]]
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[C_0140]]
; CHECK-NEXT:        [[PHI_TEMP20:%.*]] = [[R_0150]]
; CHECK-NEXT:        [[PHI_TEMP40:%.*]] = 0
; CHECK-NEXT:        [[EXTRACT_0_0:%.*]] = extractelement [[DOTVEC0]],  0
; CHECK-NEXT:        if ([[EXTRACT_0_0]] == 1)
; CHECK-NEXT:        {
; CHECK-NEXT:           goto [[MERGE_BLK0:merge.blk[0-9]+]].38
; CHECK-NEXT:        }
; CHECK-NEXT:        [[TGU60:%.*]] = zext.i32.i64([[N0]])  /u  8
; CHECK-NEXT:        [[VEC_TC70:%.*]] = [[TGU60]]  *  8
; CHECK-NEXT:        [[RED_INIT0:%.*]] = 0.000000e+00
; CHECK-NEXT:        [[INSERT0:%.*]] = insertelement zeroinitializer,  [[C_0140]],  0
; CHECK-NEXT:        [[PHI_TEMP80:%.*]] = [[RED_INIT0]]
; CHECK-NEXT:        [[PHI_TEMP100:%.*]] = [[INSERT0]]
; CHECK-NEXT:        [[LOOP_UB0:%.*]] = [[VEC_TC70]]  -  1
; CHECK:             + DO i1 = 0, [[LOOP_UB0]], 8   <DO_LOOP>  <MAX_TC_EST = 268435455>  <LEGAL_MAX_TC = 268435455> <auto-vectorized> <nounroll> <novectorize>
; CHECK-NEXT:        |   [[DOTVEC120:%.*]] = (<8 x i32>*)([[A0]])[i1]
; CHECK-NEXT:        |   [[DOTVEC130:%.*]] = [[DOTVEC120]] != 0
; CHECK-NEXT:        |   [[EXTRACT_0_140:%.*]] = extractelement &((<8 x float*>)([[F0]])[%phi.temp10]),  0
; CHECK-NEXT:        |   [[EXP_LOAD0:%.*]] = @llvm.masked.expandload.v8f32([[EXTRACT_0_140]],  [[DOTVEC130]],  undef)
; CHECK-NEXT:        |   [[DOTVEC150:%.*]] = [[EXP_LOAD0]]  +  [[PHI_TEMP80]]
; CHECK-NEXT:        |   [[SELECT0:%.*]] = ([[DOTVEC130]] == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? [[DOTVEC150]] : [[PHI_TEMP80]]
; CHECK-NEXT:        |   [[SELECT160:%.*]] = ([[DOTVEC130]] == <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>) ? [[PHI_TEMP100]] + 1 : [[PHI_TEMP100]]
; CHECK-NEXT:        |   [[VEC_REDUCE0:%.*]] = @llvm.vector.reduce.add.v8i32([[SELECT160]])
; CHECK-NEXT:        |   [[INSERT170:%.*]] = insertelement zeroinitializer,  [[VEC_REDUCE0]],  0
; CHECK-NEXT:        |   [[PHI_TEMP80]] = [[SELECT0]]
; CHECK-NEXT:        |   [[PHI_TEMP100]] = [[INSERT170]]
; CHECK-NEXT:        + END LOOP
; CHECK:             [[R_0150]] = @llvm.vector.reduce.fadd.v8f32([[R_0150]],  [[SELECT0]])
; CHECK-NEXT:        [[IND_FINAL0:%.*]] = 0 + [[VEC_TC70]]
; CHECK-NEXT:        [[EXTRACT_0_210:%.*]] = extractelement [[INSERT170]],  0
; CHECK-NEXT:        [[C_0140]] = [[EXTRACT_0_210]]
; CHECK-NEXT:        [[DOTVEC220:%.*]] = zext.i32.i64([[N0]]) == [[VEC_TC70]]
; CHECK-NEXT:        [[PHI_TEMP0]] = [[EXTRACT_0_210]]
; CHECK-NEXT:        [[PHI_TEMP20]] = [[R_0150]]
; CHECK-NEXT:        [[PHI_TEMP40]] = [[IND_FINAL0]]
; CHECK-NEXT:        [[PHI_TEMP260:%.*]] = [[EXTRACT_0_210]]
; CHECK-NEXT:        [[PHI_TEMP280:%.*]] = [[R_0150]]
; CHECK-NEXT:        [[PHI_TEMP300:%.*]] = [[IND_FINAL0]]
; CHECK-NEXT:        [[EXTRACT_0_320:%.*]] = extractelement [[DOTVEC220]],  0
; CHECK-NEXT:        if ([[EXTRACT_0_320]] == 1)
; CHECK-NEXT:        {
; CHECK-NEXT:           goto [[FINAL_MERGE:final.merge.[0-9]+]]
; CHECK-NEXT:        }
; CHECK-NEXT:        [[MERGE_BLK0]].38:
; CHECK-NEXT:        [[LB_TMP0:%.*]] = [[PHI_TEMP40]]
; CHECK-NEXT:        [[R_0150]] = [[PHI_TEMP20]]
; CHECK-NEXT:        [[C_0140]] = [[PHI_TEMP0]]
; CHECK:             + DO i1 = [[LB_TMP0]], zext.i32.i64([[N0]]) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 7>  <LEGAL_MAX_TC = 7> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 7>
; CHECK-NEXT:        |   if (([[A0]])[i1] != 0)
; CHECK-NEXT:        |   {
; CHECK-NEXT:        |      [[TMP1:%.*]] = ([[F0]])[%c.014]
; CHECK-NEXT:        |      [[C_0140]] = [[C_0140]]  +  1
; CHECK-NEXT:        |      [[R_0150]] = [[TMP1]]  +  [[R_0150]]
; CHECK-NEXT:        |   }
; CHECK-NEXT:        + END LOOP
; CHECK:             [[PHI_TEMP260]] = [[C_0140]]
; CHECK-NEXT:        [[PHI_TEMP280]] = [[R_0150]]
; CHECK-NEXT:        [[PHI_TEMP300]] = zext.i32.i64([[N0]]) + -1
; CHECK-NEXT:        [[FINAL_MERGE]]:
; CHECK-NEXT:  END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree nosync nounwind uwtable
define dso_local noundef i32 @_Z3fooPfiPi(float* noalias nocapture noundef readonly %f, i32 noundef %n, i32* noalias noundef %a) local_unnamed_addr #0 {
entry:
  call void @llvm.assume(i1 true) [ "align"(i32* %a, i64 32) ]
  %cmp13 = icmp sgt i32 %n, 0
  br i1 %cmp13, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %entry
  %wide.trip.count18 = zext i32 %n to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.inc
  %c.1.lcssa = phi i32 [ %c.1, %for.inc ]
  %r.1.lcssa = phi float [ %r.1, %for.inc ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %c.0.lcssa = phi i32 [ 0, %entry ], [ %c.1.lcssa, %for.cond.cleanup.loopexit ]
  %r.0.lcssa = phi float [ 0.000000e+00, %entry ], [ %r.1.lcssa, %for.cond.cleanup.loopexit ]
  %conv = sitofp i32 %c.0.lcssa to float
  %add4 = fadd fast float %r.0.lcssa, %conv
  %conv5 = fptosi float %add4 to i32
  ret i32 %conv5

for.body:                                         ; preds = %for.body.preheader, %for.inc
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %r.015 = phi float [ 0.000000e+00, %for.body.preheader ], [ %r.1, %for.inc ]
  %c.014 = phi i32 [ 0, %for.body.preheader ], [ %c.1, %for.inc ]
  %arrayidx = getelementptr inbounds i32, i32* %a, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !3
  %tobool.not = icmp eq i32 %0, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %inc = add nsw i32 %c.014, 1
  %idxprom1 = sext i32 %c.014 to i64
  %arrayidx2 = getelementptr inbounds float, float* %f, i64 %idxprom1
  %1 = load float, float* %arrayidx2, align 4, !tbaa !7
  %add = fadd fast float %1, %r.015
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %c.1 = phi i32 [ %inc, %if.then ], [ %c.014, %for.body ]
  %r.1 = phi float [ %add, %if.then ], [ %r.015, %for.body ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count18
  br i1 %exitcond.not, label %for.cond.cleanup.loopexit, label %for.body, !llvm.loop !9
}

; Function Attrs: inaccessiblememonly mustprogress nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #1

attributes #0 = { mustprogress nofree nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly mustprogress nofree nosync nounwind willreturn }

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
!8 = !{!"float", !5, i64 0}
!9 = distinct !{!9, !10, !11}
!10 = !{!"llvm.loop.mustprogress"}
!11 = !{!"llvm.loop.intel.vector.aligned"}
