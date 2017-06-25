; RUN: opt -VPlanDriver -S %s | FileCheck %s


; CHECK: vector.body:
; Prepare GEP for gather
; CHECK:   %[[INDICES:.*]] = load <4 x i32>, <4 x i32>* 
; CHECK:   %[[G_PTR_TO_SCALAR:.*]] = bitcast <2 x i32>* {{.*}} to i32*
; CHECK:   %[[G_INDICES_FOR_ELTS_0:.*]] = mul <4 x i32> %[[INDICES]], <i32 2, i32 2, i32 2, i32 2>
; CHECK:   %[[G_INDICES_FOR_ELTS_1:.*]] = add <4 x i32> %[[G_INDICES_FOR_ELTS_0]], <i32 1, i32 1, i32 1, i32 1>
; CHECK:   %[[G_INDICES_FOR_ELTS_0_AND_1:.*]] = shufflevector <4 x i32> %[[G_INDICES_FOR_ELTS_0]], <4 x i32> %[[G_INDICES_FOR_ELTS_1]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK:   %[[G_GEP:.*]] = getelementptr inbounds i32, i32* %[[G_PTR_TO_SCALAR]], <8 x i32> %[[G_INDICES_FOR_ELTS_0_AND_1]]
; CHECK:   %replicatedMaskVec. = shufflevector <4 x i1> %[[MASK:.*]], <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK:   %wide.masked.gather = call <8 x i32> @llvm.masked.gather.v8i32(<8 x i32*> %[[G_GEP]], i32 4, <8 x i1> %replicatedMaskVec., <8 x i32> undef)
; Calculate value to be stored
; CHECK:   %[[VALUE_TO_STORE:.*]] = add <8 x i32> %wide.masked.gather, <i32 5, i32 5, i32 5, i32 5, i32 6, i32 6, i32 6, i32 6>
; Prepare GEP for scatter
; CHECK:   %[[S_PTR_TO_SCALAR:.*]] = bitcast <2 x i32>* {{.*}} to i32*
; CHECK:   %[[S_INDICES_FOR_ELTS_0:.*]] = mul <4 x i32> %[[INDICES]], <i32 2, i32 2, i32 2, i32 2>
; CHECK:   %[[S_INDICES_FOR_ELTS_1:.*]] = add <4 x i32> %[[S_INDICES_FOR_ELTS_0]], <i32 1, i32 1, i32 1, i32 1>
; CHECK:   %[[S_INDICES_FOR_ELTS_0_AND_1:.*]] = shufflevector <4 x i32> %[[S_INDICES_FOR_ELTS_0]], <4 x i32> %[[S_INDICES_FOR_ELTS_1]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK:   %[[S_GEP_FOR_SCATTER:.*]] = getelementptr inbounds i32, i32* %[[S_PTR_TO_SCALAR]], <8 x i32> %[[S_INDICES_FOR_ELTS_0_AND_1]]
; CHECK:   %[[ReplicatedMaskVec:.*]] = shufflevector <4 x i1> %[[MASK]], <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK:   call void @llvm.masked.scatter.v8i32(<8 x i32> %[[VALUE_TO_STORE]], <8 x i32*> %[[S_GEP_FOR_SCATTER]], i32 4, <8 x i1> %[[ReplicatedMaskVec]])


@arr2p = external global <2 x i32>*, align 8
@arrB = external global i32*, align 8

define void @foo1()  {
entry:
  %0 = load <2 x i32>*, <2 x i32>** @arr2p, align 8
  %ptrToB = load i32*, i32** @arrB, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.SIMD.1
  br label %for.body

for.body:                                         ; preds = %for.inc, %entry
  %indvars.iv = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %indvars.iv.next, %for.inc ]
  %1 = trunc i64 %indvars.iv to i32
  %arrayidxB = getelementptr inbounds i32, i32* %ptrToB, i64 %indvars.iv
  %B = load i32, i32* %arrayidxB, align 4
  %tobool = icmp eq i32 %B, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds <2 x i32>, <2 x i32>* %0, i32 %B
  %2 = load <2 x i32>, <2 x i32>* %arrayidx, align 4
  %3 = add <2 x i32> %2, <i32 5, i32 6>
  store <2 x i32> %3, <2 x i32>* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                    ; preds = %omp.inner.for.body
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %omp.loop.exit

  ret void
}


; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

