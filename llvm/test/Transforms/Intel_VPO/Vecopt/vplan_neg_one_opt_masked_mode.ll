; Test to check that we don't perform -1-stride optimization in masked mode loop

; RUN: opt -disable-output %s -passes=vplan-vec -vplan-enable-masked-vectorized-remainder -vplan-vec-scenario="n0;v16;m16" -print-after=vplan-vec 2>&1 | FileCheck %s
; RUN: opt -disable-output %s -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-enable-masked-vectorized-remainder -vplan-vec-scenario="n0;v16;m16" 2>&1 | FileCheck %s -check-prefixes=HIR

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
; CHECK-NEXT:    [[SCALAR_GEP0:%.*]] = getelementptr inbounds [1029 x i32], ptr @b, i64 0, i64 [[UNI_PHI40]]
; CHECK-NEXT:    [[TMP0:%.*]] = getelementptr i32, ptr [[SCALAR_GEP0]], i32 -15
; CHECK-NEXT:    [[WIDE_LOAD0:%.*]] = load <16 x i32>, ptr [[TMP0]], align 4
; CHECK-NEXT:    [[REVERSE0:%.*]] = shufflevector <16 x i32> [[WIDE_LOAD0]], <16 x i32> undef, <16 x i32> <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; CHECK-NEXT:    [[SCALAR_GEP60:%.*]] = getelementptr inbounds [1029 x i32], ptr @a, i64 0, i64 [[UNI_PHI0]]
; CHECK-NEXT:    store <16 x i32> [[REVERSE0]], ptr [[SCALAR_GEP60]], align 16
; CHECK-NEXT:    [[TMP3]] = sub nuw nsw <16 x i64> [[VEC_PHI50]], <i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16>
; CHECK-NEXT:    [[TMP4]] = sub nuw nsw i64 [[UNI_PHI40]], -16
; CHECK-NEXT:    [[TMP5:%.*]] = sub nuw nsw <16 x i64> [[VEC_PHI50]], <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[TMP6]] = add nuw nsw <16 x i64> [[VEC_PHI0]], <i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16>
; CHECK-NEXT:    [[TMP7]] = add nuw nsw i64 [[UNI_PHI0]], 16
; CHECK-NEXT:    [[TMP8:%.*]] = icmp uge i64 [[TMP7]], 1024
; CHECK-NEXT:    br i1 [[TMP8]], label [[VPLANNEDBB70:%.*]], label [[VECTOR_BODY0]]
;
; ======== remainder body
; CHECK:       VPlannedBB21:
; CHECK-NEXT:    [[MM_VECTORGEP0:%.*]] = getelementptr inbounds [1029 x i32], <16 x ptr> <ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b>, <16 x i64> zeroinitializer, <16 x i64> [[VEC_PHI180:%.*]]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER0:%.*]] = call <16 x i32> @llvm.masked.gather.v16i32.v16p0(<16 x ptr> [[MM_VECTORGEP0]], i32 4, <16 x i1> [[TMP12:%.*]], <16 x i32> poison)
; CHECK-NEXT:    [[SCALAR_GEP220:%.*]] = getelementptr inbounds [1029 x i32], ptr @a, i64 0, i64 [[DOTEXTRACT_0_0:%.*]]
; CHECK-NEXT:    call void @llvm.masked.store.v16i32.p0(<16 x i32> [[WIDE_MASKED_GATHER0]], ptr [[SCALAR_GEP220]], i32 16, <16 x i1> [[TMP12]])
; CHECK-NEXT:    [[TMP14:%.*]] = sub nuw nsw <16 x i64> [[VEC_PHI180]], <i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16>
; CHECK-NEXT:    [[TMP15:%.*]] = sub nuw nsw i64 [[UNI_PHI170:%.*]], -16
; CHECK-NEXT:    [[TMP16:%.*]] = sub nuw nsw <16 x i64> [[VEC_PHI180]], <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    br label [[NEW_LATCH0:%.*]]
;
; HIR-LABEL:  BEGIN REGION { modified }
; HIR:             + DO i1 = 0, 1023, 16   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR-NEXT:        |   [[DOTVEC130:%.*]] = undef
; HIR-NEXT:        |   [[DOTVEC30:%.*]] = (<16 x i32>*)(@b)[0][-1 * i1 + 1013]
; HIR-NEXT:        |   [[REVERSE0:%.*]] = shufflevector [[DOTVEC30]],  undef,  <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; HIR-NEXT:        |   (<16 x i32>*)(@a)[0][i1] = [[REVERSE0]]
; HIR-NEXT:        + END LOOP
; ====== remainder
; HIR:             [[DOTVEC40:%.*]] = 1029 == 1024
; HIR-NEXT:        [[PHI_TEMP0:%.*]] = 1024
; HIR-NEXT:        [[PHI_TEMP60:%.*]] = 1024
; HIR-NEXT:        [[EXTRACT_0_80:%.*]] = extractelement [[DOTVEC40]],  0
; HIR-NEXT:        if ([[EXTRACT_0_80]] == 1)
; HIR-NEXT:        {
; HIR-NEXT:           goto final.merge.49
; HIR-NEXT:        }
;
; HIR:             [[DOTVEC90:%.*]] = 1029  -  [[PHI_TEMP0]]
; HIR-NEXT:        [[EXTRACT_0_100:%.*]] = extractelement [[DOTVEC90]],  0
; HIR-NEXT:        [[LOOP_UB0:%.*]] = [[EXTRACT_0_100]]  -  1
; HIR-NEXT:        [[DOTVEC110:%.*]] = <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> <u [[DOTVEC90]]
; HIR-NEXT:        [[DOTVEC120:%.*]] = -1  *  [[PHI_TEMP0]] + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
; HIR-NEXT:        [[DOTSCAL0:%.*]] = -1  *  [[PHI_TEMP0]]
; HIR-NEXT:        [[DOTVEC130]] = (<16 x i32>*)(@b)[0][[[DOTVEC120]] + 1028], Mask = @{[[DOTVEC110]]}
; HIR-NEXT:        (<16 x i32>*)(@a)[0][[[PHI_TEMP0]]] = [[DOTVEC130]], Mask = @{[[DOTVEC110]]}
; HIR-NEXT:        [[DOTVEC140:%.*]] = <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> + 16 <u [[DOTVEC90]]
; HIR-NEXT:        [[TMP0:%.*]] = bitcast.<16 x i1>.i16([[DOTVEC140]])
; HIR-NEXT:        [[CMP0:%.*]] = [[TMP0]] == 0
; HIR-NEXT:        [[ALL_ZERO_CHECK0:%.*]] = [[CMP0]]
; HIR:       END REGION
;
entry:
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.i.preheader
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.body.i ], [ 0, %for.body.preheader ]
  %ind = phi i64 [ %ind.next.i, %for.body.i ], [ 1028, %for.body.preheader ]
  %arrayidx.i = getelementptr inbounds [1029 x i32], ptr @b, i64 0, i64 %ind
  %t_2 = load i32, ptr %arrayidx.i, align 4
  %arrayidx3.i = getelementptr inbounds [1029 x i32], ptr @a, i64 0, i64 %indvars.iv.i
  store i32 %t_2, ptr %arrayidx3.i, align 4
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
; CHECK-NEXT:    [[SCALAR_GEP0:%.*]] = getelementptr inbounds [1029 x i32], ptr @b, i64 0, i64 [[UNI_PHI40]]
; CHECK-NEXT:    [[TMP3:%.*]] = getelementptr i32, ptr [[SCALAR_GEP0]], i32 -15
; CHECK-NEXT:    [[WIDE_LOAD0:%.*]] = load <16 x i32>, ptr [[TMP3]], align 4
; CHECK-NEXT:    [[REVERSE0:%.*]] = shufflevector <16 x i32> [[WIDE_LOAD0]], <16 x i32> undef, <16 x i32> <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; CHECK-NEXT:    [[SCALAR_GEP60:%.*]] = getelementptr inbounds [1029 x i32], ptr @a, i64 0, i64 [[UNI_PHI0]]
; CHECK-NEXT:    store <16 x i32> [[REVERSE0]], ptr [[SCALAR_GEP60]], align 16
; CHECK-NEXT:    [[TMP6]] = sub nuw nsw <16 x i64> [[VEC_PHI50]], <i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16>
; CHECK-NEXT:    [[TMP7]] = sub nuw nsw i64 [[UNI_PHI40]], -16
; CHECK-NEXT:    [[TMP8:%.*]] = sub nuw nsw <16 x i64> [[VEC_PHI50]], <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    [[TMP9]] = add nuw nsw <16 x i64> [[VEC_PHI0]], <i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16, i64 16>
; CHECK-NEXT:    [[TMP10]] = add nuw nsw i64 [[UNI_PHI0]], 16
; CHECK-NEXT:    [[TMP11:%.*]] = icmp uge i64 [[TMP10]], [[TMP2:%.*]]
; CHECK-NEXT:    br i1 [[TMP11]], label [[VPLANNEDBB70:%.*]], label [[VECTOR_BODY0]]
;
; ======== remainder body
; CHECK:       VPlannedBB21:
; CHECK-NEXT:    [[MM_VECTORGEP0:%.*]] = getelementptr inbounds [1029 x i32], <16 x ptr> <ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b, ptr @b>, <16 x i64> zeroinitializer, <16 x i64> [[VEC_PHI180:%.*]]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER0:%.*]] = call <16 x i32> @llvm.masked.gather.v16i32.v16p0(<16 x ptr> [[MM_VECTORGEP0]], i32 4, <16 x i1> [[TMP20:%.*]], <16 x i32> poison)
; CHECK-NEXT:    [[SCALAR_GEP220:%.*]] = getelementptr inbounds [1029 x i32], ptr @a, i64 0, i64 [[DOTEXTRACT_0_0:%.*]]
; CHECK-NEXT:    call void @llvm.masked.store.v16i32.p0(<16 x i32> [[WIDE_MASKED_GATHER0]], ptr [[SCALAR_GEP220]], i32 16, <16 x i1> [[TMP20]])
; CHECK-NEXT:    [[TMP22:%.*]] = sub nuw nsw <16 x i64> [[VEC_PHI180]], <i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16, i64 -16>
; CHECK-NEXT:    [[TMP23:%.*]] = sub nuw nsw i64 [[UNI_PHI170]], -16
; CHECK-NEXT:    [[TMP24:%.*]] = sub nuw nsw <16 x i64> [[VEC_PHI180]], <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
; CHECK-NEXT:    br label [[NEW_LATCH0]]
;
; HIR-LABEL:  BEGIN REGION { modified }
; HIR:             + DO i1 = 0, [[LOOP_UB0:%.*]], 16   <DO_LOOP> <MAX_TC_EST = 64> <simd-vectorized> <nounroll> <novectorize>
; HIR-NEXT:        |   [[DOTVEC170:%.*]] = undef
; HIR-NEXT:        |   [[DOTVEC50:%.*]] = (<16 x i32>*)(@b)[0][-1 * i1 + 1013]
; HIR-NEXT:        |   [[REVERSE0:%.*]] = shufflevector [[DOTVEC50]],  undef,  <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; HIR-NEXT:        |   (<16 x i32>*)(@a)[0][i1] = [[REVERSE0]]
; HIR-NEXT:        + END LOOP
; ====== remainder body
; HIR:             [[DOTVEC110:%.*]] = [[N0:%.*]]  -  [[PHI_TEMP0:.*]];
; HIR:        [[EXTRACT_0_120:%.*]] = extractelement [[DOTVEC110]],  0
; HIR:        [[LOOP_UB130:%.*]] = [[EXTRACT_0_120]]  -  1
; HIR:        [[DOTVEC140:%.*]] = <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> <u [[DOTVEC110]];
; HIR:        [[DOTVEC150:%.*]] = -1  *  [[PHI_TEMP0]] + <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15>
; HIR-NEXT:        [[DOTSCAL0:%.*]] = -1  *  [[PHI_TEMP0]]
; HIR-NEXT:        [[REVERSE160:%.*]] = shufflevector [[DOTVEC140]],  undef,  <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; HIR-NEXT:        [[DOTVEC170]] = (<16 x i32>*)(@b)[0][[[DOTSCAL0]] + 1013], Mask = @{[[REVERSE160]]}
; HIR-NEXT:        [[REVERSE180:%.*]] = shufflevector [[DOTVEC170]],  undef,  <i32 15, i32 14, i32 13, i32 12, i32 11, i32 10, i32 9, i32 8, i32 7, i32 6, i32 5, i32 4, i32 3, i32 2, i32 1, i32 0>
; HIR-NEXT:        (<16 x i32>*)(@a)[0][[[PHI_TEMP0]]] = [[REVERSE180]], Mask = @{[[DOTVEC140]]}
; HIR-NEXT:        [[DOTVEC190:%.*]] = <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7, i64 8, i64 9, i64 10, i64 11, i64 12, i64 13, i64 14, i64 15> + 16 <u [[DOTVEC110]]
; HIR-NEXT:        [[TMP0:%.*]] = bitcast.<16 x i1>.i16([[DOTVEC190]])
; HIR-NEXT:        [[CMP0:%.*]] = [[TMP0]] == 0
; HIR-NEXT:        [[ALL_ZERO_CHECK0:%.*]] = [[CMP0]]
; HIR:       END REGION
;
entry:
  br label %for.body.preheader

for.body.preheader:                               ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 16) ]
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.i.preheader
  %indvars.iv.i = phi i64 [ %indvars.iv.next.i, %for.body.i ], [ 0, %for.body.preheader ]
  %ind = phi i64 [ %ind.next.i, %for.body.i ], [ 1028, %for.body.preheader ]
  %arrayidx.i = getelementptr inbounds [1029 x i32], ptr @b, i64 0, i64 %ind
  %t_2 = load i32, ptr %arrayidx.i, align 4
  %arrayidx3.i = getelementptr inbounds [1029 x i32], ptr @a, i64 0, i64 %indvars.iv.i
  store i32 %t_2, ptr %arrayidx3.i, align 4
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

