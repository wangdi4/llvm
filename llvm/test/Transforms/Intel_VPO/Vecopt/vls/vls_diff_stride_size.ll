; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -hir-details-no-verbose-indent < %s 2>&1 | FileCheck %s --check-prefix=HIRCHECK
; RUN: opt -disable-output -passes="vplan-vec,print" < %s 2>&1 | FileCheck %s --check-prefix=LLVMCHECK
;
; LIT test to check that VLS optimization kicks in for stride 2 accesses when the group has
; more than 2 consecutive elements (example: a[2*i], a[2*i+1], a[2*i+2]). We also check
; that if the stride is negative or if the loads are under a mask we only VLS optimize
; 2 of the accesses and do a gather for the third.
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Incoming HIR:
;      + DO i1 = 0, 31, 1   <DO_LOOP> <simd>
;      |   %0 = (%arr2)[2 * i1];
;      |   %1 = (%arr2)[2 * i1 + 1];
;      |   %2 = (%arr2)[2 * i1 + 2];
;      |   (%arr1)[i1] = %2 + (%0 * %1);
;      + END LOOP
;
; HIRCHECK:         + DO i1 = 0, 31, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIRCHECK-NEXT:    |   %.vls.load = undef
; HIRCHECK-NEXT:    |   %.vls.load = (<16 x i64>*)(%arr2)[2 * i1], Mask = @{<i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false>};
; HIRCHECK-NEXT:    |   %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; HIRCHECK-NEXT:    |   %vls.extract2 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; HIRCHECK-NEXT:    |   %vls.extract3 = shufflevector %.vls.load,  %.vls.load,  <i32 2, i32 4, i32 6, i32 8>;
; HIRCHECK-NEXT:    |   %.vec = %vls.extract  *  %vls.extract2;
; HIRCHECK-NEXT:    |   (<4 x i64>*)(%arr1)[i1] = %vls.extract3 + %.vec;
; HIRCHECK-NEXT:    + END LOOP
;
; LLVMCHECK:       vector.body:
; LLVMCHECK-NEXT:    [[UNI_PHI:%.*]] = phi i64 [ 0, [[VPLANNEDBB1:%.*]] ], [ [[TMP7:%.*]], [[VECTOR_BODY:%.*]] ]
; LLVMCHECK-NEXT:    [[VEC_PHI:%.*]] = phi <4 x i64> [ <i64 0, i64 1, i64 2, i64 3>, [[VPLANNEDBB1]] ], [ [[TMP6:%.*]], [[VECTOR_BODY]] ]
; LLVMCHECK-NEXT:    [[TMP0:%.*]] = shl nuw nsw <4 x i64> [[VEC_PHI]], <i64 1, i64 1, i64 1, i64 1>
; LLVMCHECK-NEXT:    [[DOTEXTRACT_0_:%.*]] = extractelement <4 x i64> [[TMP0]], i32 0
; LLVMCHECK-NEXT:    [[SCALAR_GEP:%.*]] = getelementptr inbounds i64, ptr [[ARR2:%.*]], i64 [[DOTEXTRACT_0_]]
; LLVMCHECK-NEXT:    [[VLS_LOAD:%.*]] = call <16 x i64> @llvm.masked.load.v16i64.p0(ptr [[SCALAR_GEP]], i32 8, <16 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false, i1 false>, <16 x i64> poison)
; LLVMCHECK-NEXT:    [[TMP1:%.*]] = shufflevector <16 x i64> [[VLS_LOAD]], <16 x i64> [[VLS_LOAD]], <4 x i32> <i32 0, i32 2, i32 4, i32 6>
; LLVMCHECK-NEXT:    [[TMP2:%.*]] = shufflevector <16 x i64> [[VLS_LOAD]], <16 x i64> [[VLS_LOAD]], <4 x i32> <i32 1, i32 3, i32 5, i32 7>
; LLVMCHECK-NEXT:    [[TMP3:%.*]] = shufflevector <16 x i64> [[VLS_LOAD]], <16 x i64> [[VLS_LOAD]], <4 x i32> <i32 2, i32 4, i32 6, i32 8>
; LLVMCHECK-NEXT:    [[TMP4:%.*]] = mul nsw <4 x i64> [[TMP2]], [[TMP1]]
; LLVMCHECK-NEXT:    [[TMP5:%.*]] = add nsw <4 x i64> [[TMP4]], [[TMP3]]
; LLVMCHECK-NEXT:    [[SCALAR_GEP3:%.*]] = getelementptr inbounds i64, ptr [[ARR1:%.*]], i64 [[UNI_PHI]]
; LLVMCHECK-NEXT:    store <4 x i64> [[TMP5]], ptr [[SCALAR_GEP3]], align 8
; LLVMCHECK-NEXT:    [[TMP6]] = add nuw nsw <4 x i64> [[VEC_PHI]], <i64 4, i64 4, i64 4, i64 4>
; LLVMCHECK-NEXT:    [[TMP7]] = add nuw nsw i64 [[UNI_PHI]], 4
; LLVMCHECK-NEXT:    [[TMP8:%.*]] = icmp uge i64 [[TMP7]], 32
; LLVMCHECK-NEXT:    br i1 [[TMP8]], label [[VPLANNEDBB4:%.*]], label [[VECTOR_BODY]]
;
define void @foo1(ptr noalias %arr1, ptr noalias %arr2) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %l1.016 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = shl nuw nsw i64 %l1.016, 1
  %arrayidx = getelementptr inbounds i64, ptr %arr2, i64 %mul
  %0 = load i64, ptr %arrayidx, align 8
  %add = add nuw nsw i64 %mul, 1
  %arrayidx2 = getelementptr inbounds i64, ptr %arr2, i64 %add
  %1 = load i64, ptr %arrayidx2, align 8
  %mul3 = mul nsw i64 %1, %0
  %add5 = add nuw nsw i64 %mul, 2
  %arrayidx6 = getelementptr inbounds i64, ptr %arr2, i64 %add5
  %2 = load i64, ptr %arrayidx6, align 8
  %add7 = add nsw i64 %mul3, %2
  %arrayidx8 = getelementptr inbounds i64, ptr %arr1, i64 %l1.016
  store i64 %add7, ptr %arrayidx8, align 8
  %inc = add nuw nsw i64 %l1.016, 1
  %exitcond.not = icmp eq i64 %inc, 32
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Incoming HIR:
;      + DO i1 = 0, 63, 1   <DO_LOOP> <simd>
;      |   %0 = (%arr2)[-2 * i1];
;      |   %1 = (%arr2)[-2 * i1 + 1];
;      |   %2 = (%arr2)[-2 * i1 + 2];
;      |   (%arr1)[i1] = %2 + (%0 * %1);
;      + END LOOP
;
;
; HIRCHECK:         + DO i1 = 0, 63, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIRCHECK-NEXT:    |   %.vls.load = (<8 x i64>*)(%arr2)[-2 * i1 + -6];
; HIRCHECK-NEXT:    |   %shuffle = shufflevector %.vls.load,  %.vls.load,  <i64 6, i64 7, i64 4, i64 5, i64 2, i64 3, i64 0, i64 1>;
; HIRCHECK-NEXT:    |   %vls.extract = shufflevector %shuffle,  %shuffle,  <i32 0, i32 2, i32 4, i32 6>;
; HIRCHECK-NEXT:    |   %vls.extract2 = shufflevector %shuffle,  %shuffle,  <i32 1, i32 3, i32 5, i32 7>;
; HIRCHECK-NEXT:    |   %.vec = (<4 x i64>*)(%arr2)[-2 * i1 + -2 * <i64 0, i64 1, i64 2, i64 3> + 2];
; HIRCHECK-NEXT:    |   %.vec3 = %vls.extract  *  %vls.extract2;
; HIRCHECK-NEXT:    |   (<4 x i64>*)(%arr1)[i1] = %.vec + %.vec3;
; HIRCHECK-NEXT:    + END LOOP
;
; LLVMCHECK:       vector.body:
; LLVMCHECK-NEXT:    [[UNI_PHI:%.*]] = phi i64 [ 0, [[VPLANNEDBB1:%.*]] ], [ [[TMP8:%.*]], [[VECTOR_BODY]] ]
; LLVMCHECK-NEXT:    [[VEC_PHI:%.*]] = phi <4 x i64> [ <i64 0, i64 1, i64 2, i64 3>, [[VPLANNEDBB1]] ], [ [[TMP7:%.*]], [[VECTOR_BODY]] ]
; LLVMCHECK-NEXT:    [[TMP0:%.*]] = mul nsw <4 x i64> [[VEC_PHI]], <i64 -2, i64 -2, i64 -2, i64 -2>
; LLVMCHECK-NEXT:    [[DOTEXTRACT_0_:%.*]] = extractelement <4 x i64> [[TMP0]], i32 0
; LLVMCHECK-NEXT:    [[SCALAR_GEP:%.*]] = getelementptr inbounds i64, ptr [[ARR2]], i64 [[DOTEXTRACT_0_]]
; LLVMCHECK-NEXT:    [[SCALAR_GEP3:%.*]] = getelementptr i64, ptr [[SCALAR_GEP]], i64 -6
; LLVMCHECK-NEXT:    [[VLS_LOAD:%.*]] = load <8 x i64>, ptr [[SCALAR_GEP3]], align 8
; LLVMCHECK-NEXT:    [[TMP1:%.*]] = shufflevector <8 x i64> [[VLS_LOAD]], <8 x i64> [[VLS_LOAD]], <8 x i32> <i32 6, i32 7, i32 4, i32 5, i32 2, i32 3, i32 0, i32 1>
; LLVMCHECK-NEXT:    [[TMP2:%.*]] = shufflevector <8 x i64> [[TMP1]], <8 x i64> [[TMP1]], <4 x i32> <i32 0, i32 2, i32 4, i32 6>
; LLVMCHECK-NEXT:    [[TMP3:%.*]] = shufflevector <8 x i64> [[TMP1]], <8 x i64> [[TMP1]], <4 x i32> <i32 1, i32 3, i32 5, i32 7>
; LLVMCHECK-NEXT:    [[TMP4:%.*]] = mul nsw <4 x i64> [[TMP3]], [[TMP2]]
; LLVMCHECK-NEXT:    [[TMP5:%.*]] = add nsw <4 x i64> [[TMP0]], <i64 2, i64 2, i64 2, i64 2>
; LLVMCHECK-NEXT:    [[MM_VECTORGEP:%.*]] = getelementptr inbounds i64, <4 x ptr> [[BROADCAST_SPLAT:%.*]], <4 x i64> [[TMP5]]
; LLVMCHECK-NEXT:    [[WIDE_MASKED_GATHER:%.*]] = call <4 x i64> @llvm.masked.gather.v4i64.v4p0(<4 x ptr> [[MM_VECTORGEP]], i32 8, <4 x i1> <i1 true, i1 true, i1 true, i1 true>, <4 x i64> poison)
; LLVMCHECK-NEXT:    [[TMP6:%.*]] = add nsw <4 x i64> [[TMP4]], [[WIDE_MASKED_GATHER]]
; LLVMCHECK-NEXT:    [[SCALAR_GEP4:%.*]] = getelementptr inbounds i64, ptr [[ARR1]], i64 [[UNI_PHI]]
; LLVMCHECK-NEXT:    store <4 x i64> [[TMP6]], ptr [[SCALAR_GEP4]], align 8
; LLVMCHECK-NEXT:    [[TMP7]] = add nuw nsw <4 x i64> [[VEC_PHI]], <i64 4, i64 4, i64 4, i64 4>
; LLVMCHECK-NEXT:    [[TMP8]] = add nuw nsw i64 [[UNI_PHI]], 4
; LLVMCHECK-NEXT:    [[TMP9:%.*]] = icmp uge i64 [[TMP8]], 64
; LLVMCHECK-NEXT:    br i1 [[TMP9]], label [[VPLANNEDBB5:%.*]], label [[VECTOR_BODY]], !llvm.loop [[LOOP2:![0-9]+]]
;
define void @foo2(ptr noalias %arr1, ptr noalias %arr2) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %l1.016 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %mul = mul nsw i64 %l1.016, -2
  %arrayidx = getelementptr inbounds i64, ptr %arr2, i64 %mul
  %0 = load i64, ptr %arrayidx, align 8
  %add = add nuw nsw i64 %mul, 1
  %arrayidx2 = getelementptr inbounds i64, ptr %arr2, i64 %add
  %1 = load i64, ptr %arrayidx2, align 8
  %mul3 = mul nsw i64 %1, %0
  %add5 = add nsw i64 %mul, 2
  %arrayidx6 = getelementptr inbounds i64, ptr %arr2, i64 %add5
  %2 = load i64, ptr %arrayidx6, align 8
  %add7 = add nsw i64 %mul3, %2
  %arrayidx8 = getelementptr inbounds i64, ptr %arr1, i64 %l1.016
  store i64 %add7, ptr %arrayidx8, align 8
  %inc = add nuw nsw i64 %l1.016, 1
  %exitcond.not = icmp eq i64 %inc, 64
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Incoming HIR:
;      + DO i1 = 0, 95, 1   <DO_LOOP> <simd>
;      |   if (-1 * i1 != 0)
;      |   {
;      |      %0 = (%arr2)[2 * i1];
;      |      %1 = (%arr2)[2 * i1 + 1];
;      |      %2 = (%arr2)[2 * i1 + 2];
;      |      (%arr1)[i1] = %2 + (%0 * %1);
;      |   }
;      + END LOOP
;
; HIRCHECK:         + DO i1 = 0, 95, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; HIRCHECK-NEXT:    |   %.vec3 = undef
; HIRCHECK-NEXT:    |   %.vls.load = undef
; HIRCHECK-NEXT:    |   %.vec = -1 * i1 + -1 * <i1 false, i1 true, i1 false, i1 true> != 0;
; HIRCHECK-NEXT:    |   %vls.mask = shufflevector %.vec,  zeroinitializer,  <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>;
; HIRCHECK-NEXT:    |   %.vls.load = (<8 x i64>*)(%arr2)[2 * i1], Mask = @{%vls.mask};
; HIRCHECK-NEXT:    |   %vls.extract = shufflevector %.vls.load,  %.vls.load,  <i32 0, i32 2, i32 4, i32 6>;
; HIRCHECK-NEXT:    |   %vls.extract2 = shufflevector %.vls.load,  %.vls.load,  <i32 1, i32 3, i32 5, i32 7>;
; HIRCHECK-NEXT:    |   %.vec3 = (<4 x i64>*)(%arr2)[2 * i1 + 2 * <i64 0, i64 1, i64 2, i64 3> + 2], Mask = @{%.vec};
; HIRCHECK-NEXT:    |   %.vec5 = %vls.extract  *  %vls.extract2;
; HIRCHECK-NEXT:    |   (<4 x i64>*)(%arr1)[i1] = %.vec3 + %.vec5, Mask = @{%.vec};
; HIRCHECK-NEXT:    + END LOOP
;
; LLVMCHECK:       vector.body:
; LLVMCHECK-NEXT:    [[UNI_PHI:%.*]] = phi i64 [ 0, [[VPLANNEDBB1:%.*]] ], [ [[TMP11:%.*]], [[VPLANNEDBB5:%.*]] ]
; LLVMCHECK-NEXT:    [[VEC_PHI:%.*]] = phi <4 x i64> [ <i64 0, i64 1, i64 2, i64 3>, [[VPLANNEDBB1]] ], [ [[TMP10:%.*]], [[VPLANNEDBB5]] ]
; LLVMCHECK-NEXT:    [[TMP0:%.*]] = and <4 x i64> [[VEC_PHI]], <i64 1, i64 1, i64 1, i64 1>
; LLVMCHECK-NEXT:    [[TMP1:%.*]] = icmp eq <4 x i64> [[TMP0]], zeroinitializer
; LLVMCHECK-NEXT:    [[TMP2:%.*]] = xor <4 x i1> [[TMP1]], <i1 true, i1 true, i1 true, i1 true>
; LLVMCHECK-NEXT:    br label [[VPLANNEDBB3:%.*]]
; LLVMCHECK:       VPlannedBB3:
; LLVMCHECK-NEXT:    [[TMP3:%.*]] = mul nsw <4 x i64> [[VEC_PHI]], <i64 2, i64 2, i64 2, i64 2>
; LLVMCHECK-NEXT:    [[DOTEXTRACT_0_:%.*]] = extractelement <4 x i64> [[TMP3]], i32 0
; LLVMCHECK-NEXT:    [[SCALAR_GEP:%.*]] = getelementptr inbounds i64, ptr [[ARR2]], i64 [[DOTEXTRACT_0_]]
; LLVMCHECK-NEXT:    [[TMP4:%.*]] = shufflevector <4 x i1> [[TMP2]], <4 x i1> zeroinitializer, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; LLVMCHECK-NEXT:    [[VLS_LOAD:%.*]] = call <8 x i64> @llvm.masked.load.v8i64.p0(ptr [[SCALAR_GEP]], i32 8, <8 x i1> [[TMP4]], <8 x i64> poison)
; LLVMCHECK-NEXT:    [[TMP5:%.*]] = shufflevector <8 x i64> [[VLS_LOAD]], <8 x i64> [[VLS_LOAD]], <4 x i32> <i32 0, i32 2, i32 4, i32 6>
; LLVMCHECK-NEXT:    [[TMP6:%.*]] = shufflevector <8 x i64> [[VLS_LOAD]], <8 x i64> [[VLS_LOAD]], <4 x i32> <i32 1, i32 3, i32 5, i32 7>
; LLVMCHECK-NEXT:    [[TMP7:%.*]] = mul nsw <4 x i64> [[TMP6]], [[TMP5]]
; LLVMCHECK-NEXT:    [[TMP8:%.*]] = add nsw <4 x i64> [[TMP3]], <i64 2, i64 2, i64 2, i64 2>
; LLVMCHECK-NEXT:    [[MM_VECTORGEP:%.*]] = getelementptr inbounds i64, <4 x ptr> [[BROADCAST_SPLAT]], <4 x i64> [[TMP8]]
; LLVMCHECK-NEXT:    [[WIDE_MASKED_GATHER:%.*]] = call <4 x i64> @llvm.masked.gather.v4i64.v4p0(<4 x ptr> [[MM_VECTORGEP]], i32 8, <4 x i1> [[TMP2]], <4 x i64> poison)
; LLVMCHECK-NEXT:    [[TMP9:%.*]] = add nsw <4 x i64> [[TMP7]], [[WIDE_MASKED_GATHER]]
; LLVMCHECK-NEXT:    [[SCALAR_GEP4:%.*]] = getelementptr inbounds i64, ptr [[ARR1]], i64 [[UNI_PHI]]
; LLVMCHECK-NEXT:    call void @llvm.masked.store.v4i64.p0(<4 x i64> [[TMP9]], ptr [[SCALAR_GEP4]], i32 8, <4 x i1> [[TMP2]])
; LLVMCHECK-NEXT:    br label [[VPLANNEDBB5]]
; LLVMCHECK:       VPlannedBB5:
; LLVMCHECK-NEXT:    [[TMP10]] = add nuw nsw <4 x i64> [[VEC_PHI]], <i64 4, i64 4, i64 4, i64 4>
; LLVMCHECK-NEXT:    [[TMP11]] = add nuw nsw i64 [[UNI_PHI]], 4
; LLVMCHECK-NEXT:    [[TMP12:%.*]] = icmp uge i64 [[TMP11]], 96
; LLVMCHECK-NEXT:    br i1 [[TMP12]], label [[VPLANNEDBB6:%.*]], label [[VECTOR_BODY]], !llvm.loop [[LOOP3:![0-9]+]]
;
define void @foo3(ptr noalias %arr1, ptr noalias %arr2) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %l1.017 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %rem = and i64 %l1.017, 1
  %tobool.not = icmp eq i64 %rem, 0
  br i1 %tobool.not, label %for.inc, label %if.then

if.then:
  %mul = mul nsw i64 %l1.017, 2
  %arrayidx = getelementptr inbounds i64, ptr %arr2, i64 %mul
  %0 = load i64, ptr %arrayidx, align 8
  %add = add nuw nsw i64 %mul, 1
  %arrayidx2 = getelementptr inbounds i64, ptr %arr2, i64 %add
  %1 = load i64, ptr %arrayidx2, align 8
  %mul3 = mul nsw i64 %1, %0
  %add5 = add nsw i64 %mul, 2
  %arrayidx6 = getelementptr inbounds i64, ptr %arr2, i64 %add5
  %2 = load i64, ptr %arrayidx6, align 8
  %add7 = add nsw i64 %mul3, %2
  %arrayidx8 = getelementptr inbounds i64, ptr %arr1, i64 %l1.017
  store i64 %add7, ptr %arrayidx8, align 8
  br label %for.inc

for.inc:
  %inc = add nuw nsw i64 %l1.017, 1
  %exitcond.not = icmp eq i64 %inc, 96
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
