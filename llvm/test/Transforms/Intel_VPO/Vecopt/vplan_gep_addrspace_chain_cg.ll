; RUN: opt -passes=vplan-vec -vplan-force-vf=8 -vplan-dump-da -S %s 2>&1 | FileCheck %s

; CHECK: Printing Divergence info for Loop at depth 1 containing: [[BB0:BB[0-9]+]]<header><latch><exiting>
; CHECK-EMPTY:
; CHECK-NEXT:  Basic Block: [[BB0]]
; CHECK-NEXT:  Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[VP_PHI1:%.*]] = phi  [ i64 [[TMP:%.*]], [[BB1:BB[0-9]+]] ],  [ i64 [[VP_PHI_NEXT1:%.*]], [[BB0]] ]
; CHECK-NEXT:  Divergent: [Shape: Strided, Stride: i64 4] ptr [[VP_GEP:%.*]] = getelementptr inbounds [1024 x i32], ptr %src i64 0 i64 [[VP_PHI1]]
; CHECK-NEXT:  Divergent: [Shape: Strided, Stride: i64 4] ptr addrspace(4) [[VP_GEP_ASCAST:%.*]] = addrspacecast ptr [[VP_GEP]]
; CHECK-NEXT:  Divergent: [Shape: Strided, Stride: i64 4] ptr addrspace(3) [[VP_GEP_ASCAST2:%.*]] = addrspacecast ptr addrspace(4) [[VP_GEP_ASCAST]]
; CHECK-NEXT:  Divergent: [Shape: Random] i32 [[VP_LOAD:%.*]] = load ptr addrspace(3) [[VP_GEP_ASCAST2]]
; CHECK-NEXT:  Divergent: [Shape: Random] i32 [[VP_OP:%.*]] = mul i32 [[VP_LOAD]] i32 3

define dso_local void @test_gep_addrpspace_cast(ptr %src) {
; CHECK:       VPlannedBB:
; CHECK-NEXT:    br label [[VPLANNEDBB10:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB1:
; CHECK-NEXT:    br label [[VECTOR_BODY0:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  vector.body:
; CHECK-NEXT:    [[UNI_PHI0:%.*]] = phi i64 [ 0, [[VPLANNEDBB10]] ], [ [[TMP6:%.*]], [[VECTOR_BODY0]] ]
; CHECK-NEXT:    [[VEC_PHI0:%.*]] = phi <8 x i64> [ <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>, [[VPLANNEDBB10]] ], [ [[TMP5:%.*]], [[VECTOR_BODY0]] ]
; CHECK-NEXT:    [[SCALAR_GEP0:%.*]] = getelementptr inbounds [1024 x i32], ptr [[SRC0:%.*]], i64 0, i64 [[UNI_PHI0]]
; CHECK-NEXT:    [[TMP0:%.*]] = addrspacecast ptr [[SCALAR_GEP0]] to ptr addrspace(4)
; CHECK-NEXT:    [[TMP1:%.*]] = addrspacecast ptr addrspace(4) [[TMP0]] to ptr addrspace(3)
; CHECK-NEXT:    [[WIDE_LOAD0:%.*]] = load <8 x i32>, ptr addrspace(3) [[TMP1]], align 4
; CHECK-NEXT:    [[TMP3:%.*]] = mul <8 x i32> [[WIDE_LOAD0]], <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
; CHECK-NEXT:    store <8 x i32> [[TMP3]], ptr [[SCALAR_GEP0]], align 4
; CHECK-NEXT:    [[TMP5]] = add nuw nsw <8 x i64> [[VEC_PHI0]], <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
; CHECK-NEXT:    [[TMP6]] = add nuw nsw i64 [[UNI_PHI0]], 8

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv1 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %iv1.next, %for.body ]

  %gep = getelementptr inbounds [1024 x i32], ptr %src, i64 0, i64 %iv1
  %gep.ascast = addrspacecast ptr %gep to ptr addrspace(4)
  %gep.ascast2 = addrspacecast ptr addrspace(4) %gep.ascast to ptr addrspace(3)
  %ld.1 = load i32, ptr addrspace(3) %gep.ascast2, align 4
  %mul = mul i32 %ld.1, 3
  store i32 %mul, ptr %gep, align 4
  %iv1.next = add nuw nsw i64 %iv1, 1
  %cmp = icmp ult i64 %iv1.next, 1024
  br i1 %cmp, label %for.body, label %for.end
for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)
