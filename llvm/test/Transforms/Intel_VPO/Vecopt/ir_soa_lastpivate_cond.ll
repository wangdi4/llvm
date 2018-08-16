; RUN: opt -VPlanDriver -vplan-force-vf=4 -S %s | FileCheck %s

; CHECK-LABEL: @foo1
; CHECK:  %tmp.vec = alloca <8 x i32>, align 4
; CHECK: %tmp = alloca <2 x i32>, align 4
; CHECK: vector.ph:                                        ; preds = %min.iters.checked
; CHECK:   %tmpInitVal = load <2 x i32>, <2 x i32>* %tmp
; CHECK:   %[[TransposeAndSplat:.*]] = shufflevector <2 x i32> %tmpInitVal
; CHECK:   store <8 x i32> %[[TransposeAndSplat]], <8 x i32>* %tmp.vec

; CHECK: vector.body:                                      ; preds = %vector.body, %vector.ph
; CHECK:   %replicatedMaskElts. = shufflevector <4 x i1> {{.*}}, <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK:   %wide.masked.load = call <8 x i32> @llvm.masked.load.v8i32.p0v8i32({{.*}} <8 x i1> %replicatedMask
; CHECK:   %transposed.wide.masked.load = shufflevector <8 x i32> %wide.masked.load, <8 x i32> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 1, i32 3, i32 5, i32 7>
; CHECK:   %[[ReplicatedMaskVec:.*]] = shufflevector <4 x i1> {{.*}}, <4 x i1> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
; CHECK:   call void @llvm.masked.store.v8i32.p0v8i32(<8 x i32> %transposed.wide.masked.load, <8 x i32>* %tmp.vec, i32 4, <8 x i1> %[[ReplicatedMaskVec]])

; CHECK: middle.block:                                     ; preds = %VPlannedBB{{[0-9]*}}
; CHECK:   %[[MASK_INT:.*]] = load i4, i4* %tmp.mask
; CHECK:   %ctlz = call i4 @llvm.ctlz.i4(i4 %[[MASK_INT]], i1 true)
; CHECK:   %LaneToCopyFrom = sub i4 3, %ctlz
; CHECK:   %[[PtrToFirstEltInPrivateVec4:.*]] = bitcast <8 x i32>* %tmp.vec to i32*
; CHECK:   %PtrToFirstEltInOrigPrivate = bitcast <2 x i32>* %tmp to i32*
; CHECK:   %PtrInsidePrivVec = getelementptr i32, i32* %[[PtrToFirstEltInPrivateVec4]], i4 %LaneToCopyFrom
; CHECK:   %LastVal = load i32, i32* %PtrInsidePrivVec
; CHECK:   store i32 %LastVal, i32* %PtrToFirstEltInOrigPrivate
; CHECK:   %[[LaneToCopyFrom5:.*]] = add i4 %LaneToCopyFrom, 4
; CHECK:   %[[PtrInsidePrivVec6:.*]] = getelementptr i32, i32* %[[PtrToFirstEltInPrivateVec4]], i4 %[[LaneToCopyFrom5]]
; CHECK:   %[[LastVal7:.*]] = load i32, i32* %[[PtrInsidePrivVec6]]
; CHECK:   %PtrToNextEltInOrigPrivate = getelementptr i32, i32* %PtrToFirstEltInOrigPrivate, i32 1
; CHECK:   store i32 %[[LastVal7]], i32* %PtrToNextEltInOrigPrivate



@arr2p = external global <2 x i32>*, align 8
@arrB = external global i32*, align 8

define <2 x i32> @foo1()  {
entry:
  %tmp = alloca <2 x i32>, align 4
  store <2 x i32><i32 5, i32 6>, <2 x i32>* %tmp, align 4
  %0 = load <2 x i32>*, <2 x i32>** @arr2p, align 8
  %ptrToB = load i32*, i32** @arrB, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LASTPRIVATE:CONDITIONAL", <2 x i32>* nonnull %tmp)
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
  store <2 x i32> %2, <2 x i32>* %tmp, align 4
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

  %res = load <2 x i32>, <2 x i32> *%tmp 
  ret <2 x i32> %res
}

; This test checks load/store in private space though a bitcast, 
; when the bitcast operaion converts vector to scalar.
; CHECK-LABEL: @foo2
; CHECK: %tmp.vec = alloca <8 x i32>, align 8
; CHECK: %[[WideLoad:.*]] = call <8 x i32> @llvm.masked.load.v8i32.p0v8i32(<8 x i32>*
; CHECK:  %[[Transposed:.*]] = shufflevector <8 x i32> %[[WideLoad]], <8 x i32> undef, <8 x i32> <i32 0, i32 2, i32 4, i32 6, i32 1, i32 3, i32 5, i32 7>
; CHECK:  %[[ToStore:.*]] = bitcast <8 x i32> %[[Transposed:.*]] to <4 x i64>

; CHECK: call void @llvm.masked.store.v4i64.p0v4i64(<4 x i64> %[[ToStore]]

define <2 x i32> @foo2()  {
entry:
  %tmp = alloca <2 x i32>, align 8
  store <2 x i32><i32 5, i32 6>, <2 x i32>* %tmp, align 4
  %0 = load <2 x i32>*, <2 x i32>** @arr2p, align 8
  %ptrToB = load i32*, i32** @arrB, align 8
  %tmp_64 = bitcast <2 x i32>* %tmp to i64*
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.LASTPRIVATE:CONDITIONAL", <2 x i32>* nonnull %tmp)
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
  %3 = bitcast <2 x i32> %2 to i64
  store i64 %3, i64* %tmp_64, align 8
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

  %res = load <2 x i32>, <2 x i32> *%tmp 
  ret <2 x i32> %res
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

