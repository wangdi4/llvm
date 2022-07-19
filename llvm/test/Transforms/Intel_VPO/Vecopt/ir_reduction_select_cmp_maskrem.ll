;RUN: opt -vplan-vec -vplan-enable-masked-vectorized-remainder -S %s | FileCheck %s

; CHECK-LABEL: @reduc_select_icmp_var_maskrem
; CHECK: vector.body
; CHECK:  [[ICMP1:%.*]] = icmp eq <8 x i32>
; CHECK:  [[SEL1:%.*]] = select <8 x i1> [[ICMP1]]
; CHECK: VPlannedBB7
; CHECK:  [[INSERT1:%.*]] = insertelement <8 x i32> poison, i32 %a, i32 0
; CHECK:  [[SPLAT1:%.*]] = shufflevector <8 x i32> [[INSERT1]], <8 x i32> poison, <8 x i32> zeroinitializer
; CHECK:  [[ICMP2:%.*]] = icmp ne <8 x i32> [[SEL1]], [[SPLAT1]]
; CHECK:  [[REDOR1:%.*]] = call i1 @llvm.vector.reduce.or.v8i1(<8 x i1> [[ICMP2]])
; CHECK:  [[SEL2:%.*]] = select i1 [[REDOR1]], i32 %b, i32 %a
; CHECK: merge.blk15
; CHECK:  [[PHI1:%.*]] = phi i32 [ [[SEL2]], %VPlannedBB9 ], [ %a, %VPlannedBB ]
; CHECK: VPlannedBB12
; CHECK:  [[INSERT2:%.*]] = insertelement <8 x i32> poison, i32 %a, i32 0
; CHECK:  [[SPLAT2:%.*]] = shufflevector <8 x i32> [[INSERT2]], <8 x i32> poison, <8 x i32> zeroinitializer
; CHECK: VPlannedBB13
; CHECK:  [[INSERT3:%.*]] = insertelement <8 x i32> [[SPLAT2]], i32 [[PHI1]], i32 0
; CHECK: VPlannedBB17
; CHECK:  [[PHI2:%.*]] = phi <8 x i32> [ [[INSERT3]], %VPlannedBB13 ], [ [[BLEND:%.*]], %new_latch ]
; CHECK:  [[MASK:%.*]] = icmp ult <8 x i64>
; CHECK: VPlannedBB23
; CHECK:  [[MLOAD:%.*]] = call <8 x i32> @llvm.masked.load.v8i32.p0v8i32
; CHECK:  [[ICMP2:%.*]] = icmp eq <8 x i32> [[MLOAD]]
; CHECK:  [[SEL3:%.*]] = select <8 x i1> [[ICMP2]]
; CHECK: new_latch
; CHECK:  [[BLEND]] = select <8 x i1> [[MASK]], <8 x i32> [[SEL3]], <8 x i32> [[PHI2]]
; CHECK: VPlannedBB27
; CHECK:  [[INSERT4:%.*]] = insertelement <8 x i32> poison, i32 [[PHI1]], i32 0
; CHECK:  [[SPLAT3:%.*]] = shufflevector <8 x i32> [[INSERT4]], <8 x i32> poison, <8 x i32> zeroinitializer
; CHECK:  [[ICMP3:%.*]] = icmp ne <8 x i32> [[BLEND]], [[SPLAT3]]
; CHECK:  [[REDOR2:%.*]] = call i1 @llvm.vector.reduce.or.v8i1(<8 x i1> [[ICMP3]])
; CHECK:  [[SEL4:%.*]] = select i1 [[REDOR2]], i32 %b, i32 [[PHI1]]

define noundef i32 @reduc_select_icmp_var_maskrem(i32* nocapture noundef readonly %v, i32 noundef %a, i32 noundef %b, i64 noundef %n) local_unnamed_addr {
entry:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %j.019 = phi i32 [ %a, %DIR.OMP.SIMD.1 ], [ %spec.select, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %v, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4
  %cmp5.not = icmp eq i32 %1, 3
  %spec.select = select i1 %cmp5.not, i32 %j.019, i32 %b
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:
  %spec.select.lcssa = phi i32 [ %spec.select, %omp.inner.for.body ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret i32 %spec.select.lcssa
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
