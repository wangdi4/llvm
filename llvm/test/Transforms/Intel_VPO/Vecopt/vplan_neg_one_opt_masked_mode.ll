; Test to check that we don't perform -1-stride optimization in masked mode loop

; RUN: opt -disable-output %s -vplan-vec -vplan-enable-masked-vectorized-remainder -vplan-vec-scenario="n0;v16;m16" -print-after=vplan-vec 2>&1 | FileCheck %s
; RUN: opt -disable-output %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-vec-dir-insert -hir-vplan-vec -vplan-enable-masked-vectorized-remainder -vplan-vec-scenario="n0;v16;m16" -print-after=hir-vplan-vec 2>&1 | FileCheck %s -check-prefixes=HIR

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@b = external dso_local local_unnamed_addr global [1029 x i32], align 16
@a = external dso_local local_unnamed_addr global [1029 x i32], align 16

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @disable_opt() local_unnamed_addr #0 {
; CHECK:       vector.body:
; CHECK-NEXT:    [[UNI_PHI0:%.*]] = phi i64 [ [[TMP7:%.*]], [[VECTOR_BODY0:%.*]] ], [ 0, [[VPLANNEDBB20:%.*]] ]
; CHECK-NEXT:    [[VEC_PHI0:%.*]] = phi <16 x i64> [ [[TMP6:%.*]], [[VECTOR_BODY0]] ], [ <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>, [[VPLANNEDBB20]] ]
; CHECK-NEXT:    [[UNI_PHI40:%.*]] = phi i64 [ [[TMP4:%.*]], [[VECTOR_BODY0]] ], [ 1028, [[VPLANNEDBB20]] ]
; CHECK-NEXT:    [[VEC_PHI50:%.*]] = phi <16 x i64> [ [[TMP3:%.*]], [[VECTOR_BODY0]] ], [ <i64 1028, i64 1029, i64 1030, i64 1031, i64 1032, i64 1033, i64 1034, i64 1035, i64 1036, i64 1037, i64 1038, i64 1039, i64 1040, i64 1041, i64 1042, i64 1043>, [[VPLANNEDBB20]] ]
; CHECK-NEXT:    [[SCALAR_GEP0:%.*]] = getelementptr inbounds [1029 x i32], [1029 x i32]* @b, i64 0, i64 [[UNI_PHI40]]
; CHECK-NEXT:    [[TMP0:%.*]] = getelementptr i32, i32* [[SCALAR_GEP0]], i32 -15
; CHECK-NEXT:    [[TMP1:%.*]] = bitcast i32* [[TMP0]] to <16 x i32>*
; CHECK-NEXT:    [[WIDE_LOAD0:%.*]] = load <16 x i32>, <16 x i32>* [[TMP1]], align 4
; CHECK-NEXT:    [[REVERSE0:%.*]] = shufflevector <16 x i32> [[WIDE_LOAD0]], <16 x i32> undef, <16 x i32> <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; CHECK-NEXT:    [[SCALAR_GEP60:%.*]] = getelementptr inbounds [1029 x i32], [1029 x i32]* @a, i64 0, i64 [[UNI_PHI0]]
; CHECK-NEXT:    [[TMP2:%.*]] = bitcast i32* [[SCALAR_GEP60]] to <16 x i32>*
; CHECK-NEXT:    store <16 x i32> [[REVERSE0]], <16 x i32>* [[TMP2]], align 16
; CHECK-NEXT:    [[TMP3]] = sub <16 x i64> [[VEC_PHI50]], <i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16>
; CHECK-NEXT:    [[TMP4]] = sub i64 [[UNI_PHI40]], -16
; CHECK-NEXT:    [[TMP5:%.*]] = sub nuw nsw <16 x i64> [[VEC_PHI50]], <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[TMP6]] = add nuw nsw <16 x i64> [[VEC_PHI0]], <i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16>
; CHECK-NEXT:    [[TMP7]] = add nuw nsw i64 [[UNI_PHI0]], 16
; CHECK-NEXT:    [[TMP8:%.*]] = icmp uge i64 [[TMP7]], 1024
; CHECK-NEXT:    br i1 [[TMP8]], label [[VPLANNEDBB70:%.*]], label [[VECTOR_BODY0]]
;
; ======== remainder body
; CHECK:       VPlannedBB19:
; CHECK-NEXT:    [[MM_VECTORGEP0:%.*]] = getelementptr inbounds [1029 x i32], <16 x [1029 x i32]*> <[1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b>, <16 x i64> zeroinitializer, <16 x i64> [[VEC_PHI180:%.*]]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER0:%.*]] = call <16 x i32> @llvm.masked.gather.v16i32.v16p0i32(<16 x i32*> [[MM_VECTORGEP0]], i32 4, <16 x i1> [[TMP11:%.*]], <16 x i32> poison)
; CHECK-NEXT:    [[SCALAR_GEP200:%.*]] = getelementptr inbounds [1029 x i32], [1029 x i32]* @a, i64 0, i64 [[UNI_PHI150:%.*]]
; CHECK-NEXT:    [[TMP12:%.*]] = bitcast i32* [[SCALAR_GEP200]] to <16 x i32>*
; CHECK-NEXT:    call void @llvm.masked.store.v16i32.p0v16i32(<16 x i32> [[WIDE_MASKED_GATHER0]], <16 x i32>* [[TMP12]], i32 16, <16 x i1> [[TMP11]])
; CHECK-NEXT:    [[TMP13:%.*]] = sub <16 x i64> [[VEC_PHI180:%.*]], <i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16>
; CHECK-NEXT:    [[TMP14:%.*]] = sub i64 [[UNI_PHI170:%.*]], -16
; CHECK-NEXT:    [[TMP15:%.*]] = sub nuw nsw <16 x i64> [[VEC_PHI180]], <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    br label [[NEW_LATCH0:%.*]]
;
; HIR-LABEL:  BEGIN REGION { modified }
; HIR:             + DO i1 = 0, 1023, 16   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:        |   [[DOTVEC100:%.*]] = undef
; HIR-NEXT:        |   [[DOTVEC30:%.*]] = (<16 x i32>*)(@b)[0][-1 * i1 + 1013]
; HIR-NEXT:        |   [[REVERSE0:%.*]] = shufflevector [[DOTVEC30]],  undef,  <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; HIR-NEXT:        |   (<16 x i32>*)(@a)[0][i1] = [[REVERSE0]]
; HIR-NEXT:        + END LOOP
; ====== remainder body
; HIR:             + DO i1 = [[PHI_TEMP0:%.*]], 1028, 16   <DO_LOOP> <vector-remainder> <nounroll> <novectorize>
; HIR-NEXT:        |   [[DOTVEC90:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> <u 1029
; HIR-NEXT:        |   [[DOTVEC100]] = (<16 x i32>*)(@b)[0][-1 * i1 + -1 * <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> + 1028], Mask = @{[[DOTVEC90]]}
; HIR-NEXT:        |   (<16 x i32>*)(@a)[0][i1] = [[DOTVEC100]], Mask = @{[[DOTVEC90]]}
; HIR-NEXT:        |   [[DOTVEC110:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> + 16 <u 1029
; HIR-NEXT:        |   [[TMP1:%.*]] = bitcast.<16 x i1>.i16([[DOTVEC110]])
; HIR-NEXT:        |   [[CMP0:%.*]] = [[TMP1]] == 0
; HIR-NEXT:        |   [[ALL_ZERO_CHECK0:%.*]] = [[CMP0]]
; HIR-NEXT:        + END LOOP
;
entry:
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.i.preheader
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.body.i ], [ 0, %for.body.preheader ]
  %ind = phi i64 [ %ind.next.i, %for.body.i ], [ 1028, %for.body.preheader ]
  %arrayidx.i = getelementptr inbounds [1029 x i32], [1029 x i32]* @b, i64 0, i64 %ind
  %t_2 = load i32, i32* %arrayidx.i, align 4
  %arrayidx3.i = getelementptr inbounds [1029 x i32], [1029 x i32]* @a, i64 0, i64 %indvars.iv.i
  store i32 %t_2, i32* %arrayidx3.i, align 4
  %ind.next.i = sub nuw nsw i64 %ind, 1
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.not.i = icmp eq i64 %indvars.iv.next.i, 1029
  br i1 %exitcond.not.i, label %for.end17, label %for.body.i

for.end17:                                        ; preds = %for.inc15
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret i32 0
}

define dso_local i32 @enable_opt(i64 %N) local_unnamed_addr #0 {
; CHECK:       vector.body:
; CHECK-NEXT:    [[UNI_PHI0:%.*]] = phi i64 [ [[TMP10:%.*]], [[VECTOR_BODY0:%.*]] ], [ 0, [[VPLANNEDBB20:%.*]] ]
; CHECK-NEXT:    [[VEC_PHI0:%.*]] = phi <16 x i64> [ [[TMP9:%.*]], [[VECTOR_BODY0]] ], [ <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>, [[VPLANNEDBB20]] ]
; CHECK-NEXT:    [[UNI_PHI40:%.*]] = phi i64 [ [[TMP7:%.*]], [[VECTOR_BODY0]] ], [ 1028, [[VPLANNEDBB20]] ]
; CHECK-NEXT:    [[VEC_PHI50:%.*]] = phi <16 x i64> [ [[TMP6:%.*]], [[VECTOR_BODY0]] ], [ <i64 1028, i64 1029, i64 1030, i64 1031, i64 1032, i64 1033, i64 1034, i64 1035, i64 1036, i64 1037, i64 1038, i64 1039, i64 1040, i64 1041, i64 1042, i64 1043>, [[VPLANNEDBB20]] ]
; CHECK-NEXT:    [[SCALAR_GEP0:%.*]] = getelementptr inbounds [1029 x i32], [1029 x i32]* @b, i64 0, i64 [[UNI_PHI40]]
; CHECK-NEXT:    [[TMP3:%.*]] = getelementptr i32, i32* [[SCALAR_GEP0]], i32 -15
; CHECK-NEXT:    [[TMP4:%.*]] = bitcast i32* [[TMP3]] to <16 x i32>*
; CHECK-NEXT:    [[WIDE_LOAD0:%.*]] = load <16 x i32>, <16 x i32>* [[TMP4]], align 4
; CHECK-NEXT:    [[REVERSE0:%.*]] = shufflevector <16 x i32> [[WIDE_LOAD0]], <16 x i32> undef, <16 x i32> <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; CHECK-NEXT:    [[SCALAR_GEP60:%.*]] = getelementptr inbounds [1029 x i32], [1029 x i32]* @a, i64 0, i64 [[UNI_PHI0]]
; CHECK-NEXT:    [[TMP5:%.*]] = bitcast i32* [[SCALAR_GEP60]] to <16 x i32>*
; CHECK-NEXT:    store <16 x i32> [[REVERSE0]], <16 x i32>* [[TMP5]], align 16
; CHECK-NEXT:    [[TMP6]] = sub <16 x i64> [[VEC_PHI50]], <i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16>
; CHECK-NEXT:    [[TMP7]] = sub i64 [[UNI_PHI40]], -16
; CHECK-NEXT:    [[TMP8:%.*]] = sub nuw nsw <16 x i64> [[VEC_PHI50]], <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[TMP9]] = add nuw nsw <16 x i64> [[VEC_PHI0]], <i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16>
; CHECK-NEXT:    [[TMP10]] = add nuw nsw i64 [[UNI_PHI0]], 16
; CHECK-NEXT:    [[TMP11:%.*]] = icmp uge i64 [[TMP10]], [[TMP2:%.*]]
; CHECK-NEXT:    br i1 [[TMP11]], label [[VPLANNEDBB70:%.*]], label [[VECTOR_BODY0]]
;
; ======== remainder body
; CHECK:       VPlannedBB19:
; CHECK-NEXT:    [[MM_VECTORGEP0:%.*]] = getelementptr inbounds [1029 x i32], <16 x [1029 x i32]*> <[1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b, [1029 x i32]* @b>, <16 x i64> zeroinitializer, <16 x i64> [[VEC_PHI180:%.*]]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER0:%.*]] = call <16 x i32> @llvm.masked.gather.v16i32.v16p0i32(<16 x i32*> [[MM_VECTORGEP0]], i32 4, <16 x i1> [[TMP19:%.*]], <16 x i32> poison)
; CHECK-NEXT:    [[SCALAR_GEP200:%.*]] = getelementptr inbounds [1029 x i32], [1029 x i32]* @a, i64 0, i64 [[UNI_PHI150:%.*]]
; CHECK-NEXT:    [[TMP20:%.*]] = bitcast i32* [[SCALAR_GEP200]] to <16 x i32>*
; CHECK-NEXT:    call void @llvm.masked.store.v16i32.p0v16i32(<16 x i32> [[WIDE_MASKED_GATHER0]], <16 x i32>* [[TMP20]], i32 16, <16 x i1> [[TMP19]])
; CHECK-NEXT:    [[TMP21:%.*]] = sub <16 x i64> [[VEC_PHI180]], <i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16>
; CHECK-NEXT:    [[TMP22:%.*]] = sub i64 [[UNI_PHI170:%.*]], -16
; CHECK-NEXT:    [[TMP23:%.*]] = sub nuw nsw <16 x i64> [[VEC_PHI180]], <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    br label [[NEW_LATCH0]]
;
; HIR-LABEL:  BEGIN REGION { modified }
; HIR:             + DO i1 = 0, [[LOOP_UB0:%.*]], 16   <DO_LOOP> <MAX_TC_EST = 64> <simd-vectorized> <nounroll> <novectorize>
; HIR-NEXT:        |   [[DOTVEC140:%.*]] = undef
; HIR-NEXT:        |   [[DOTVEC50:%.*]] = (<16 x i32>*)(@b)[0][-1 * i1 + 1013]
; HIR-NEXT:        |   [[REVERSE0:%.*]] = shufflevector [[DOTVEC50]],  undef,  <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; HIR-NEXT:        |   (<16 x i32>*)(@a)[0][i1] = [[REVERSE0]]
; HIR-NEXT:        + END LOOP
; ====== remainder body
; HIR:             + DO i1 = [[PHI_TEMP0]], [[LOOP_UB110:%.*]], 16   <DO_LOOP>  <MAX_TC_EST = 1>  <LEGAL_MAX_TC = 1> <vector-remainder> <nounroll> <novectorize> <max_trip_count = 1>
; HIR-NEXT:        |   [[DOTVEC120:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> <u [[N0:%.*]]
; HIR-NEXT:        |   [[REVERSE130:%.*]] = shufflevector [[DOTVEC120]],  undef,  <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; HIR-NEXT:        |   [[DOTVEC140]] = (<16 x i32>*)(@b)[0][-1 * i1 + 1013], Mask = @{[[REVERSE130]]}
; HIR-NEXT:        |   [[REVERSE150:%.*]] = shufflevector [[DOTVEC140]],  undef,  <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; HIR-NEXT:        |   (<16 x i32>*)(@a)[0][i1] = [[REVERSE150]], Mask = @{[[DOTVEC120]]}
; HIR-NEXT:        |   [[DOTVEC160:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> + 16 <u [[N0]]
; HIR-NEXT:        |   [[TMP1:%.*]] = bitcast.<16 x i1>.i16([[DOTVEC160]])
; HIR-NEXT:        |   [[CMP0:%.*]] = [[TMP1]] == 0
; HIR-NEXT:        |   [[ALL_ZERO_CHECK0:%.*]] = [[CMP0]]
; HIR-NEXT:        + END LOOP
;
entry:
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.i.preheader
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.body.i ], [ 0, %for.body.preheader ]
  %ind = phi i64 [ %ind.next.i, %for.body.i ], [ 1028, %for.body.preheader ]
  %arrayidx.i = getelementptr inbounds [1029 x i32], [1029 x i32]* @b, i64 0, i64 %ind
  %t_2 = load i32, i32* %arrayidx.i, align 4
  %arrayidx3.i = getelementptr inbounds [1029 x i32], [1029 x i32]* @a, i64 0, i64 %indvars.iv.i
  store i32 %t_2, i32* %arrayidx3.i, align 4
  %ind.next.i = sub nuw nsw i64 %ind, 1
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.not.i = icmp eq i64 %indvars.iv.next.i, %N
  br i1 %exitcond.not.i, label %for.end17, label %for.body.i

for.end17:                                        ; preds = %for.inc15
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  ret i32 0
};; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

