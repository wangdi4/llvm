; Test to check that VPlan DA's knowledge about uniform loads & loads/stores
; with unit-strided pointers is transferred to VPOLegal, which ensures that
; gathers/scatters are not generated for vector code.

; REQUIRES: asserts
; RUN: opt -VPlanDriver -disable-vplan-predicator -debug-only=vplan-divergence-analysis \
; RUN: -vplan-force-vf=16 -S -enable-vp-value-codegen=false %s 2>&1 | FileCheck %s

; Check that DA identified inner loop loads as uniform or unit-stride.
; CHECK-LABEL: Basic Block: BB4
; CHECK-NEXT: Uniform: [Shape: Uniform] i64 [[PHI:%vp.*]] = phi  [ i64 0, BB3 ],  [ i64 [[ADD:%vp.*]], BB4 ]
; CHECK-NEXT: Uniform: [Shape: Uniform] i32* [[UNI_GEP:%vp.*]] = getelementptr inbounds i32* [[OUTER_GEP:%vp.*]] i64 [[PHI]]
; CHECK-NEXT: Uniform: [Shape: Uniform] i32 [[UNI_LOAD:%vp.*]] = load i32* [[UNI_GEP]]
; CHECK-NEXT: Uniform: [Shape: Uniform] i32 [[LOAD_USER:%vp.*]] = add i32 [[UNI_LOAD]] i32 42
; CHECK-NEXT: Divergent: [Shape: Strided, Stride: i64 4] i32* [[UNIT_STRIDE_GEP:%vp.*]] = getelementptr inbounds i32* [[OUTER_GEP]] i64 [[OUTER_IV:%vp.*]]
; CHECK-NEXT: Divergent: [Shape: Random] i32 [[UNIT_STRIDE_LOAD:%vp.*]] = load i32* [[UNIT_STRIDE_GEP]]
; CHECK-NEXT: Divergent: [Shape: Random] store i32 [[UNIT_STRIDE_LOAD]] i32* [[UNIT_STRIDE_GEP]]

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

define void @test1(float* nocapture %ptr, i64 %n, i64* %arr, i32* %arr1) {
  entry:
  %cmp = icmp sgt i64 %n, 0
  br i1 %cmp, label %for.body.lr.ph, label %exit

for.body.lr.ph:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(i32* %arr1, i64* %arr) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %for.latch ]
  %outer.gep = getelementptr inbounds i64, i64* %arr, i64 42
  %outer.load = load i64, i64* %outer.gep
  %outer.gep2 = getelementptr inbounds i32, i32* %arr1, i64 %outer.load
  br label %for.body2

for.body2:
; CHECK-LABEL: VPlannedBB:
  %indvars.iv2 = phi i64 [ 0, %for.body ], [ %indvars.iv.next2, %for.body2 ]
; CHECK-NEXT: [[IV_PHI:%.*]] = phi <16 x i64> [ zeroinitializer, [[VECTOR_BODY:%.*]] ], [ [[IV_ADD:%.*]], [[VPLANNEDBB:%.*]] ]
  %uni.gep = getelementptr inbounds i32, i32* %outer.gep2, i64 %indvars.iv2
; CHECK-NEXT: [[VEC_GEP2:%.*]] = getelementptr inbounds i32, <16 x i32*> [[VEC_GEP1:%.*]], <16 x i64> [[IV_PHI]]
  %uni.load = load i32, i32* %uni.gep
; CHECK-NEXT: [[UNI_LOAD_GEP:%.*]] = extractelement <16 x i32*> [[VEC_GEP2]], i32 0
; CHECK-NEXT: [[UNI_LOAD:%.*]] = load i32, i32* [[UNI_LOAD_GEP]]
  %user = add i32 %uni.load, 42
; CHECK-NEXT: [[BCAST_INSERT:%.*]] = insertelement <16 x i32> undef, i32 [[UNI_LOAD]], i32 0
; CHECK-NEXT: [[BCAST_SHUFFLE:%.*]] = shufflevector <16 x i32> [[BCAST_INSERT]], <16 x i32> undef, <16 x i32> zeroinitializer
; CHECK-NEXT: [[WIDE_USER:%.*]] = add <16 x i32> [[BCAST_SHUFFLE]], <i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42, i32 42>
  %unit.stride.gep = getelementptr inbounds i32, i32* %outer.gep2, i64 %indvars.iv
  %unit.stride.load = load i32, i32* %unit.stride.gep
; CHECK: [[UNIT_GEP:%.*]] = getelementptr inbounds i32, i32* {{.*}}, i64 {{.*}}
; CHECK-NEXT: [[UNIT_GEP_BC:%.*]] = bitcast i32* [[UNIT_GEP]] to <16 x i32>*
; CHECK-NEXT: [[WIDE_LOAD:%.*]] = load <16 x i32>, <16 x i32>* [[UNIT_GEP_BC]], align 4
  store i32 %unit.stride.load, i32* %unit.stride.gep
; CHECK-NEXT: [[UNIT_GEP_BC:%.*]] = bitcast i32* [[UNIT_GEP]] to <16 x i32>*
; CHECK-NEXT: store <16 x i32> [[WIDE_LOAD]], <16 x i32>* [[UNIT_GEP_BC]], align 4
  %indvars.iv.next2 = add nuw nsw i64 %indvars.iv2, 1
  %exitcond2 = icmp eq i64 %indvars.iv.next2, %n
  br i1 %exitcond2, label %for.latch, label %for.body2

for.latch:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"()]
  br label %exit

exit:
  ret void
}
