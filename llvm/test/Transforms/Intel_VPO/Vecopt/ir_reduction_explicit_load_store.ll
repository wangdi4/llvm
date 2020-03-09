; Test to check that VPlan vectorizer legality recognizes explicit reductions with stores to the
; reduction variable inside the loop and a loop invariant load for starting value, while reduction
; is performed in register (using PHI node).

; RUN: opt -VPlanDriver -S < %s | FileCheck %s

define float @load_store_reduction_add(float* nocapture %a) {
; CHECK-LABEL: @load_store_reduction_add(
; CHECK:       vector.body:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i64 [ 0, [[VECTOR_PH:%.*]] ], [ [[INDEX_NEXT:%.*]], [[VECTOR_BODY:%.*]] ]
; CHECK-NEXT:    [[UNI_PHI:%.*]] = phi i64 [ 0, [[VECTOR_PH]] ], [ [[TMP3:%.*]], [[VECTOR_BODY]] ]
; CHECK-NEXT:    [[VEC_PHI:%.*]] = phi <8 x i64> [ <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>, [[VECTOR_PH]] ], [ [[TMP2:%.*]], [[VECTOR_BODY]] ]
; CHECK-NEXT:    [[VEC_PHI1:%.*]] = phi <8 x float> [ zeroinitializer, [[VECTOR_PH]] ], [ [[TMP1:%.*]], [[VECTOR_BODY]] ]
; CHECK-NEXT:    [[SCALAR_GEP:%.*]] = getelementptr inbounds float, float* [[A:%.*]], i64 [[UNI_PHI]]
; CHECK-NEXT:    [[TMP0:%.*]] = bitcast float* [[SCALAR_GEP]] to <8 x float>*
; CHECK-NEXT:    [[WIDE_LOAD:%.*]] = load <8 x float>, <8 x float>* [[TMP0]], align 4
; CHECK-NEXT:    [[TMP1]] = fadd <8 x float> [[VEC_PHI1]], [[WIDE_LOAD]]
; CHECK-NEXT:    store <8 x float> [[TMP1]], <8 x float>* [[PRIVATE_MEM:%.*]], align 4
; CHECK-NEXT:    [[TMP2]] = add nuw nsw <8 x i64> [[VEC_PHI]], <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
; CHECK-NEXT:    [[TMP3]] = add nuw nsw i64 [[UNI_PHI]], 8
; CHECK-NEXT:    [[TMP4:%.*]] = icmp eq <8 x i64> [[TMP2]], <i64 1000, i64 1000, i64 1000, i64 1000, i64 1000, i64 1000, i64 1000, i64 1000>
; CHECK-NEXT:    [[DOTEXTRACT_0_:%.*]] = extractelement <8 x i1> [[TMP4]], i32 0
; CHECK-NEXT:    [[INDEX_NEXT]] = add i64 [[INDEX]], 8
; CHECK-NEXT:    [[TMP5:%.*]] = icmp eq i64 [[INDEX_NEXT]], 1000
; CHECK-NEXT:    br i1 [[TMP5]], label [[VPLANNEDBB:%.*]], label [[VECTOR_BODY]]
; CHECK:       VPlannedBB:
; CHECK-NEXT:    [[TMP6:%.*]] = call float @llvm.experimental.vector.reduce.v2.fadd.f32.v8f32(float %x.promoted, <8 x float> [[TMP1]])
; CHECK-NEXT:    store float [[TMP6]], float* [[X:%.*]], align 1
; CHECK-NEXT:    br label [[MIDDLE_BLOCK:%.*]]
;
entry:
  %x = alloca float, align 4
  store float 2.000000e+00, float* %x, align 4
  br label %entry.split

entry.split:                                      ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.REDUCTION.ADD"(float* %x) ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %entry.split
  %x.promoted = load float, float* %x, align 4
  br label %for.body

for.body:                                         ; preds = %for.body, %DIR.QUAL.LIST.END.2
  %indvars.iv = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %indvars.iv.next, %for.body ]
  %add7 = phi float [ %x.promoted, %DIR.QUAL.LIST.END.2 ], [ %add, %for.body ]
  %a.gep = getelementptr inbounds float, float* %a, i64 %indvars.iv
  %a.load = load float, float* %a.gep
  %add = fadd float %add7, %a.load
  store float %add, float* %x, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %add.lcssa = phi float [ %add, %for.body ]
  br label %for.end1

for.end1:                                         ; preds = %for.end
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %for.end
  store float %add.lcssa, float* %x, align 4
  %x1 = load float, float* %x, align 4
  ret float %x1
}

; Function Attrs: argmemonly nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: argmemonly nounwind
declare void @llvm.directive.region.exit(token)

