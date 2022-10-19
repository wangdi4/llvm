; RUN: opt -opaque-pointers -vplan-vec -vplan-force-vf=2 -vplan-enable-soa -S %s 2>&1 | FileCheck %s

define void @merge_uniform_strided_soa_geps() {
; CHECK-LABEL: @merge_uniform_strided_soa_geps(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ARR_SOA_PRIV64:%.*]] = alloca [1024 x i64], align 4
; CHECK-NEXT:    [[ARR_SOA_PRIV64_SOA_VEC:%.*]] = alloca [1024 x <2 x i64>], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
; CHECK:       simd.begin.region:
; CHECK-NEXT:    br label [[SIMD_LOOP_PREHEADER:%.*]]
; CHECK:       simd.loop.preheader:
; CHECK-NEXT:    br label [[VPLANNEDBB:%.*]]
; CHECK:       VPlannedBB:
; CHECK-NEXT:    br label [[VPLANNEDBB1:%.*]]
; CHECK:       VPlannedBB1:
; CHECK-NEXT:    call void @llvm.lifetime.start.p0(i64 16384, ptr [[ARR_SOA_PRIV64_SOA_VEC]])
; CHECK-NEXT:    br label [[VECTOR_BODY:%.*]]
; CHECK:       vector.body:
; CHECK-NEXT:    [[UNI_PHI:%.*]] = phi i64 [ 0, [[VPLANNEDBB1]] ], [ [[TMP1:%.*]], [[VPLANNEDBB8:%.*]] ]
; CHECK-NEXT:    [[VEC_PHI:%.*]] = phi <2 x i64> [ <i64 0, i64 1>, [[VPLANNEDBB1]] ], [ [[TMP0:%.*]], [[VPLANNEDBB8]] ]
; CHECK-NEXT:    br i1 true, label [[VPLANNEDBB3:%.*]], label [[VPLANNEDBB4:%.*]]
; CHECK:       VPlannedBB4:
; CHECK-NEXT:    [[SOA_SCALAR_GEP5:%.*]] = getelementptr inbounds [1024 x <2 x i64>], ptr [[ARR_SOA_PRIV64_SOA_VEC]], i64 0, i64 1
; CHECK-NEXT:    [[SOA_SCALAR_GEP6:%.*]] = getelementptr inbounds [1024 x <2 x i64>], ptr [[ARR_SOA_PRIV64_SOA_VEC]], i64 0, i64 1
; CHECK-NEXT:    [[MM_VECTORGEP:%.*]] = getelementptr <2 x i64>, ptr [[SOA_SCALAR_GEP6]], <2 x i64> zeroinitializer, <2 x i64> <i64 0, i64 1>
; CHECK-NEXT:    [[WIDE_LOAD7:%.*]] = load <2 x i64>, ptr [[SOA_SCALAR_GEP5]], align 4
; CHECK-NEXT:    br label [[VPLANNEDBB8]]
; CHECK:       VPlannedBB3:
; CHECK-NEXT:    [[MM_VECTORGEP9:%.*]] = getelementptr inbounds [1024 x <2 x i64>], ptr [[ARR_SOA_PRIV64_SOA_VEC]], <2 x i64> zeroinitializer, <2 x i64> [[VEC_PHI]], <2 x i64> <i64 0, i64 1>
; CHECK-NEXT:    br label [[VPLANNEDBB8]]
; CHECK:       VPlannedBB6:
; CHECK-NEXT:    [[VEC_PHI10:%.*]] = phi <2 x ptr> [ [[MM_VECTORGEP]], [[VPLANNEDBB4]] ], [ [[MM_VECTORGEP9]], [[VPLANNEDBB3]] ]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER:%.*]] = call <2 x i64> @llvm.masked.gather.v2i64.v2p0(<2 x ptr> [[VEC_PHI10]], i32 4, <2 x i1> <i1 true, i1 true>, <2 x i64> poison)
; CHECK-NEXT:    [[MM_VECTORGEP11:%.*]] = getelementptr inbounds i64, <2 x ptr> [[VEC_PHI10]], <2 x i64> [[WIDE_MASKED_GATHER]]
; CHECK-NEXT:    [[WIDE_MASKED_GATHER12:%.*]] = call <2 x i64> @llvm.masked.gather.v2i64.v2p0(<2 x ptr> [[MM_VECTORGEP11]], i32 4, <2 x i1> <i1 true, i1 true>, <2 x i64> poison)
; CHECK-NEXT:    [[TMP0]] = add nuw nsw <2 x i64> [[VEC_PHI]], <i64 2, i64 2>
; CHECK-NEXT:    [[TMP1]] = add nuw nsw i64 [[UNI_PHI]], 2
; CHECK-NEXT:    [[TMP2:%.*]] = icmp ult i64 [[TMP1]], 1024
; CHECK-NEXT:    br i1 [[TMP2]], label [[VECTOR_BODY]], label [[VPLANNEDBB13:%.*]], !llvm.loop [[LOOP2:![0-9]+]]
; CHECK:       VPlannedBB10:
; CHECK-NEXT:    call void @llvm.lifetime.end.p0(i64 16384, ptr [[ARR_SOA_PRIV64_SOA_VEC]])
;
entry:
  %arr.soa.priv64 = alloca [1024 x i64], align 4
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %arr.soa.priv64, i64 zeroinitializer, i64 1024)]
  br label %simd.loop.preheader

simd.loop.preheader:
  br label %simd.loop

simd.loop:
  %iv1 = phi i64 [ 0, %simd.loop.preheader ], [ %iv1.next, %simd.check.phi]
  br i1 true, label %bb1, label %bb2
bb1:
  %str.if = getelementptr inbounds [1024 x i64], ptr %arr.soa.priv64, i64 0, i64 %iv1
  br label %simd.check.phi
bb2:
  %uni.else = getelementptr inbounds [1024 x i64], ptr %arr.soa.priv64, i64 0, i64 1
  %ld.else = load i64, ptr %uni.else, align 4
  br label %simd.check.phi
simd.check.phi:
  %phi.mix.uni = phi i64* [%uni.else, %bb2], [%str.if, %bb1]
  %ld = load i64, ptr %phi.mix.uni, align 4
  %gep.mix.uni = getelementptr inbounds i64, ptr %phi.mix.uni, i64 %ld
  %ld.phi.derived = load i64, ptr %gep.mix.uni, align 4
  %iv1.next = add nuw nsw i64 %iv1, 1
  %cmp = icmp ult i64 %iv1.next, 1024
  br i1 %cmp, label %simd.loop, label %simd.end
simd.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)
declare dso_local i64 @helper(i64*)
