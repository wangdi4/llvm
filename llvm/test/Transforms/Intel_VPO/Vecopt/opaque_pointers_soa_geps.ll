; RUN: opt -passes=vplan-vec -vplan-enable-soa-phis -vplan-force-vf=2 -vplan-enable-soa -S %s 2>&1 | FileCheck %s

define void @merge_uniform_strided_soa_geps() {
; CHECK-LABEL: @merge_uniform_strided_soa_geps(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[ARR_SOA_PRIV640:%.*]] = alloca [1024 x i64], align 4
; CHECK-NEXT:    [[ARR_SOA_PRIV64_SOA_VEC0:%.*]] = alloca [1024 x <2 x i64>], align 16
; CHECK-NEXT:    [[BROADCAST_SPLATINSERT0:%.*]] = insertelement <2 x ptr> poison, ptr [[ARR_SOA_PRIV64_SOA_VEC0]], i64 0
; CHECK-NEXT:    [[BROADCAST_SPLAT0:%.*]] = shufflevector <2 x ptr> [[BROADCAST_SPLATINSERT0]], <2 x ptr> poison, <2 x i32> zeroinitializer
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION0:%.*]]
; CHECK:       simd.begin.region:
; CHECK-NEXT:    br label [[SIMD_LOOP_PREHEADER0:%.*]]
; CHECK:       simd.loop.preheader:
; CHECK-NEXT:    br label [[VPLANNEDBB:%.*]]
; CHECK:       VPlannedBB:
; CHECK-NEXT:    br label [[VPLANNEDBB1:%.*]]
; CHECK:       VPlannedBB1:
; CHECK-NEXT:    call void @llvm.lifetime.start.p0(i64 16384, ptr [[ARR_SOA_PRIV64_SOA_VEC0]])
; CHECK-NEXT:    br label [[VECTOR_BODY:%.*]]
; CHECK:       vector.body:
; CHECK-NEXT:    [[UNI_PHI0:%.*]] = phi i64 [ 0, [[VPLANNEDBB1]] ], [ [[TMP1:%.*]], [[VPLANNEDBB50:%.*]] ]
; CHECK-NEXT:    [[VEC_PHI0:%.*]] = phi <2 x i64> [ <i64 0, i64 1>, [[VPLANNEDBB1]] ], [ [[TMP0:%.*]], [[VPLANNEDBB50]] ]
; CHECK-NEXT:    br i1 true, label [[VPLANNEDBB30:%.*]], label [[VPLANNEDBB40:%.*]]
; CHECK:       VPlannedBB4:
; CHECK-NEXT:    [[SOA_SCALAR_GEP0:%.*]] = getelementptr inbounds [1024 x <2 x i64>], ptr [[ARR_SOA_PRIV64_SOA_VEC0]], i64 0, i64 1
; CHECK-NEXT:    [[SOA_VECTORGEP0:%.*]] = getelementptr inbounds [1024 x <2 x i64>], <2 x ptr> [[BROADCAST_SPLAT0]], <2 x i64> zeroinitializer, <2 x i64> <i64 1, i64 1>
; CHECK-NEXT:    [[WIDE_LOAD0:%.*]] = load <2 x i64>, ptr [[SOA_SCALAR_GEP0]], align 4
; CHECK-NEXT:    br label [[VPLANNEDBB50]]
; CHECK:       VPlannedBB3:
; CHECK-NEXT:    [[SOA_VECTORGEP60:%.*]] = getelementptr inbounds [1024 x <2 x i64>], ptr [[ARR_SOA_PRIV64_SOA_VEC0]], <2 x i64> zeroinitializer, <2 x i64> [[VEC_PHI0]]
; CHECK-NEXT:    br label [[VPLANNEDBB50]]
; CHECK:       VPlannedBB5:
; CHECK-NEXT:    [[VEC_PHI70:%.*]] = phi <2 x ptr> [ [[SOA_VECTORGEP0]], [[VPLANNEDBB40]] ], [ [[SOA_VECTORGEP60]], [[VPLANNEDBB30]] ]
; CHECK-NEXT:    [[SOA_VECTORGEP80:%.*]] = getelementptr <2 x i64>, <2 x ptr> [[VEC_PHI70]], <2 x i32> zeroinitializer, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER0:%.*]] = call <2 x i64> @llvm.masked.gather.v2i64.v2p0(<2 x ptr> [[SOA_VECTORGEP80]], i32 4, <2 x i1> <i1 true, i1 true>, <2 x i64> poison)
; CHECK-NEXT:    [[SOA_VECTORGEP90:%.*]] = getelementptr inbounds <2 x i64>, <2 x ptr> [[VEC_PHI70]], <2 x i64> [[WIDE_MASKED_GATHER0]]
; CHECK-NEXT:    [[SOA_VECTORGEP100:%.*]] = getelementptr <2 x i64>, <2 x ptr> [[SOA_VECTORGEP90]], <2 x i32> zeroinitializer, <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[WIDE_MASKED_GATHER110:%.*]] = call <2 x i64> @llvm.masked.gather.v2i64.v2p0(<2 x ptr> [[SOA_VECTORGEP100]], i32 4, <2 x i1> <i1 true, i1 true>, <2 x i64> poison)
; CHECK:       VPlannedBB12:
; CHECK-NEXT:    call void @llvm.lifetime.end.p0(i64 16384, ptr [[ARR_SOA_PRIV64_SOA_VEC0]])
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
  %phi.mix.uni = phi ptr [%uni.else, %bb2], [%str.if, %bb1]
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
declare dso_local i64 @helper(ptr)
