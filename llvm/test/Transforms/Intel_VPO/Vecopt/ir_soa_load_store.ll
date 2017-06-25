; RUN: opt -VPlanDriver -S %s | FileCheck %s


; CHECK: vector.body:
; CHECK:  %replicatedMaskElts. = shufflevector <4 x i1> %[[MASK:.*]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK:  %wide.masked.load = call <8 x i32> @llvm.masked.load.v8i32.p0v8i32(<8 x i32>* %{{.*}}, i32 4, <8 x i1> %replicatedMaskElts., <8 x i32> undef)
; CHECK:  %transposed.wide.masked.load = shufflevector <8 x i32> %wide.masked.load, <8 x i32> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 1, i32 3, i32 5, i32 7>
; CHECK:  %[[ADD:.*]] = add <8 x i32> %transposed.wide.masked.load, <i32 5, i32 5, i32 5, i32 5, i32 6, i32 6, i32 6, i32 6>
; CHECK:  %[[GEP1:.*]] = getelementptr inbounds <2 x i32>, <2 x i32>* %0, i64 %1
; CHECK:  %20 = bitcast <2 x i32>* %[[GEP1]] to <8 x i32>*
; CHECK:  %normalized. = shufflevector <8 x i32> %[[ADD]], <8 x i32> undef, <8 x i32> <i32 0, i32 4, i32 1, i32 5, i32 2, i32 6, i32 3, i32 7>
; CHECK:  %[[ReplicatedMaskElts:.*]] = shufflevector <4 x i1> %[[MASK]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK:  call void @llvm.masked.store.v8i32.p0v8i32(<8 x i32> %normalized., <8 x i32>* %20, i32 4, <8 x i1> %[[ReplicatedMaskElts]]
; CHECK:  %[[GEP3:.*]] = getelementptr inbounds <2 x i32>, <2 x i32>* %0, i64 %1
; CHECK:  %[[ADDR:.*]] = bitcast <2 x i32>* %[[GEP3]] to <8 x i32>*
; CHECK:  %[[LOAD:.*]] = load <8 x i32>, <8 x i32>* %[[ADDR]], align 4
; CHECK:  %[[TR_LOAD:.*]] = shufflevector <8 x i32> %[[LOAD]], <8 x i32> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 1, i32 3, i32 5, i32 7>
; CHECK:  %[[ADD2:.*]] = add <8 x i32> %[[TR_LOAD]], <i32 7, i32 7, i32 7, i32 7, i32 8, i32 8, i32 8, i32 8>
; CHECK:  %[[GEP4:.*]] = getelementptr inbounds <2 x i32>, <2 x i32>* %0, i64 %1
; CHECK:  %[[ADDR2:.*]] = bitcast <2 x i32>* %[[GEP4]] to <8 x i32>*
; CHECK:  %[[N_DATA:.*]] = shufflevector <8 x i32> %[[ADD2]], <8 x i32> undef, <8 x i32> <i32 0, i32 4, i32 1, i32 5, i32 2, i32 6, i32 3, i32 7>
; CHECK:  store <8 x i32> %[[N_DATA]], <8 x i32>* %[[ADDR2]], align 4


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
  %arrayidx = getelementptr inbounds <2 x i32>, <2 x i32>* %0, i64 %indvars.iv
  %2 = load <2 x i32>, <2 x i32>* %arrayidx, align 4
  %3 = add <2 x i32> %2, <i32 5, i32 6>
  store <2 x i32> %3, <2 x i32>* %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then
  %arrayidx1 = getelementptr inbounds <2 x i32>, <2 x i32>* %0, i64 %indvars.iv
  %x2 = load <2 x i32>, <2 x i32>* %arrayidx1, align 4
  %x3 = add <2 x i32> %x2, <i32 7, i32 8>
  store <2 x i32> %x3, <2 x i32>* %arrayidx1, align 4

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

