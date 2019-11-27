; RUN: opt -VPlanDriver -vplan-force-vf=4 -enable-vp-value-codegen=false -S %s | FileCheck %s --check-prefixes=CHECK,CHECK-IRCG
; RUN: opt -VPlanDriver -vplan-force-vf=4 -enable-vp-value-codegen -S %s | FileCheck %s  --check-prefixes=CHECK,CHECK-VPCG

; CHECK-LABEL: foo1
; CHECK: entry:
; CHECK: [[PTR:%.*]] = load <2 x i32>*, <2 x i32>** {{.*}}, align 8

; CHECK:      vector.body:
; CHECK:        [[IDX:%.*]] = phi i64
; CHECK-VPCG:   [[UNI_PHI:%.*]] = phi i64 [ 0, %vector.ph ], [ [[UNI_PHI_LOOP:%.*]], %vector.body ]
; CHECK-IRCG:   [[GEP1:%.*]] = getelementptr inbounds <2 x i32>, <2 x i32>* [[PTR]], i64 [[IDX]]
; CHECK-VPCG:   [[GEP1:%.*]] = getelementptr inbounds <2 x i32>, <2 x i32>* [[PTR]], i64 [[UNI_PHI]]
; CHECK:        [[BC_1:%.*]] = bitcast <2 x i32>* [[GEP1]] to <8 x i32>*
; CHECK:        [[REP_MASK_1:%.*]] = shufflevector <4 x i1> [[MASK:%.*]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK:        [[WIDE_LOAD:%.*]] = call <8 x i32> @llvm.masked.load.v8i32.p0v8i32(<8 x i32>* [[BC_1]], i32 4, <8 x i1> [[REP_MASK_1]], <8 x i32> undef)
; CHECK:        %[[ADD:.*]] = add <8 x i32> [[WIDE_LOAD]], <i32 5, i32 6, i32 5, i32 6, i32 5, i32 6, i32 5, i32 6>
; CHECK:        bitcast <8 x i32> {{.*}} to <4 x i64>
; CHECK:        [[BC:.*]] = bitcast
; CHECK:        [[BC_2:%.*]] = bitcast <2 x i32>* [[GEP1]] to <8 x i32>*
; CHECK:        [[REP_MASK_2:%.*]] = shufflevector <4 x i1> [[MASK:%.*]], <4 x i1> undef, <8 x i32> <i32 0, i32 0, i32 1, i32 1, i32 2, i32 2, i32 3, i32 3>
; CHECK:        call void @llvm.masked.store.v8i32.p0v8i32(<8 x i32> {{.*}}, <8 x i32>* {{.*}}, i32 4, <8 x i1> [[REP_MASK_2]]
; CHECK-IRCG:   %[[GEP3:.*]] = getelementptr inbounds <2 x i32>, <2 x i32>* [[PTR]], i64 [[IDX]]
; CHECK-VPCG:   %[[GEP3:.*]] = getelementptr inbounds <2 x i32>, <2 x i32>* [[PTR]], i64 [[UNI_PHI]]
; CHECK:        %[[ADDR:.*]] = bitcast <2 x i32>* %[[GEP3]] to <8 x i32>*
; CHECK:        %[[LOAD:.*]] = load <8 x i32>, <8 x i32>* %[[ADDR]], align 4
; CHECK:        %[[ADD2:.*]] = add <8 x i32> %[[LOAD]], <i32 7, i32 8, i32 7, i32 8, i32 7, i32 8, i32 7, i32 8>
; CHECK:        %[[ADDR2:.*]] = bitcast <2 x i32>* %[[GEP3]] to <8 x i32>*
; CHECK:        store <8 x i32> %[[ADD2]], <8 x i32>* %[[ADDR2]], align 4
; CHECK-VPCG:   [[UNI_PHI_LOOP]] = add nuw nsw i64 [[UNI_PHI]], 4


@arr2p = external global <2 x i32>*, align 8
@arrB = external global i32*, align 8

define void @foo1()  {
entry:
  %0 = load <2 x i32>*, <2 x i32>** @arr2p, align 8
  %ptrToB = load i32*, i32** @arrB, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
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
  %4 = bitcast <2 x i32> %3 to i64
  %5 = add i64 %4, 6688
  %6 = bitcast i64 %5 to <2 x i32>
  store <2 x i32> %6, <2 x i32>* %arrayidx, align 4
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
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %omp.loop.exit

  ret void
}

; CHECK-LABEL: simd_copy_loop
; CHECK: %[[WideLoad:.*]] = load
; CHECK: store <4 x i64> %[[WideLoad]]
define void @simd_copy_loop(<2 x i32>* %src, <2 x i32>* %dest){
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %entry, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds <2 x i32>, <2 x i32>* %src, i64 %indvars.iv
  %0 = bitcast <2 x i32>* %arrayidx to i64*
  %1 = load i64, i64* %0, align 8
  %arrayidx2 = getelementptr inbounds <2 x i32>, <2 x i32>* %dest, i64 %indvars.iv
  %2 = bitcast <2 x i32>* %arrayidx2 to i64*
  store i64 %1, i64* %2, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %omp.loop.exit
  ret void
}

; CHECK-LABEL: simd_copy_reverse
; CHECK: %[[WideLoad:.*]] = load
; CHECK: %[[Rev1:.*]] = shufflevector <4 x i64> %[[WideLoad]], <4 x i64> undef, <4 x i32> <i32 3, i32 2, i32 1, i32 0>
; CHECK: %[[Rev2:.*]] = shufflevector <4 x i64> %[[Rev1]], <4 x i64> undef, <4 x i32> <i32 3, i32 2, i32 1, i32 0>
; CHECK: store <4 x i64> %[[Rev2]]
define void @simd_copy_reverse(<2 x i32>* %src, <2 x i32>* %dest) #0 {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %entry, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %omp.inner.for.body ]
  %0 = sub nuw nsw i64 1023, %indvars.iv
  %arrayidx = getelementptr inbounds <2 x i32>, <2 x i32>* %src, i64 %0
  %1 = bitcast <2 x i32>* %arrayidx to i64*
  %2 = load i64, i64* %1, align 8
  %arrayidx2 = getelementptr inbounds <2 x i32>, <2 x i32>* %dest, i64 %0
  %3 = bitcast <2 x i32>* %arrayidx2 to i64*
  store i64 %2, i64* %3, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp ne i64 %indvars.iv.next, 1024
  br i1 %exitcond, label %omp.inner.for.body, label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %omp.loop.exit
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
