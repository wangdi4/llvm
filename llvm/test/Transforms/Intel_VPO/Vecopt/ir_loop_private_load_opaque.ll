; RUN: opt -vplan-enable-soa=false -passes=vplan-vec -vplan-force-vf=4 -S %s | FileCheck %s

define void @foo(ptr nocapture %arr) {
; CHECK-LABEL: @foo(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[PRIV:%.*]] = alloca i32, align 4
; CHECK-NEXT:    [[PRIV_VEC:%.*]] = alloca <4 x i32>, align 16
; CHECK-NEXT:    [[PRIV_VEC_BASE_ADDR:%.*]] = getelementptr i32, ptr [[PRIV_VEC]], <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK-NEXT:    [[PRIV_VEC_BASE_ADDR_EXTRACT_3_:%.*]] = extractelement <4 x ptr> [[PRIV_VEC_BASE_ADDR]], i32 3
; CHECK-NEXT:    [[PRIV_VEC_BASE_ADDR_EXTRACT_2_:%.*]] = extractelement <4 x ptr> [[PRIV_VEC_BASE_ADDR]], i32 2
; CHECK-NEXT:    [[PRIV_VEC_BASE_ADDR_EXTRACT_1_:%.*]] = extractelement <4 x ptr> [[PRIV_VEC_BASE_ADDR]], i32 1
; CHECK-NEXT:    [[PRIV_VEC_BASE_ADDR_EXTRACT_0_:%.*]] = extractelement <4 x ptr> [[PRIV_VEC_BASE_ADDR]], i32 0
; CHECK:       vector.body:
; CHECK-NEXT:    [[UNI_PHI1:%.*]] = phi i32 [ 0, [[VECTOR_PH:%.*]] ], [ [[TMP3:%.*]], [[VECTOR_BODY:%.*]] ]
; CHECK-NEXT:    [[VEC_PHI:%.*]] = phi <4 x i32> [ <i32 0, i32 1, i32 2, i32 3>, [[VECTOR_PH]] ], [ [[TMP2:%.*]], [[VECTOR_BODY]] ]
; CHECK-NEXT:    call void @baz(ptr nonnull [[PRIV_VEC_BASE_ADDR_EXTRACT_0_]])
; CHECK-NEXT:    call void @baz(ptr nonnull [[PRIV_VEC_BASE_ADDR_EXTRACT_1_]])
; CHECK-NEXT:    call void @baz(ptr nonnull [[PRIV_VEC_BASE_ADDR_EXTRACT_2_]])
; CHECK-NEXT:    call void @baz(ptr nonnull [[PRIV_VEC_BASE_ADDR_EXTRACT_3_]])
; CHECK-NEXT:    [[WIDE_LOAD:%.*]] = load <4 x i32>, ptr [[PRIV_VEC]], align 4
; CHECK-NEXT:    [[TMP0:%.*]] = sext <4 x i32> [[VEC_PHI]] to <4 x i64>
; CHECK-NEXT:    [[DOTEXTRACT_0_:%.*]] = extractelement <4 x i64> [[TMP0]], i32 0
; CHECK-NEXT:    [[SCALAR_GEP:%.*]] = getelementptr inbounds i32, ptr [[ARR:%.*]], i64 [[DOTEXTRACT_0_]]
; CHECK-NEXT:    store <4 x i32> [[WIDE_LOAD]], ptr [[SCALAR_GEP]], align 4
;
entry:
  %priv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %priv, i32 0, i32 1) ]
  br label %inner.for.body

inner.for.body:                               ; preds = %inner.for.body, %DIR.OMP.SIMD.1
  %index = phi i32 [ 0, %DIR.OMP.SIMD.1 ], [ %add1, %inner.for.body ]
  call void @baz(ptr nonnull %priv)
  %priv.val = load i32, ptr %priv, align 4
  %idxprom = sext i32 %index to i64
  %arrayidx = getelementptr inbounds i32, ptr %arr, i64 %idxprom
  store i32 %priv.val, ptr %arrayidx, align 4
  %add1 = add nuw nsw i32 %index, 1
  %exitcond = icmp eq i32 %add1, 100
  br i1 %exitcond, label %DIR.OMP.END.SIMD.4, label %inner.for.body

DIR.OMP.END.SIMD.4:                               ; preds = %inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.4
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @baz(ptr)
