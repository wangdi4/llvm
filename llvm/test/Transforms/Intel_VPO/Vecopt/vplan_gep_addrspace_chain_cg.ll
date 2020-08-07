; RUN: opt -VPlanDriver -vplan-force-vf=8 -vplan-dump-da -S %s 2>&1 | FileCheck %s

; CHECK: Printing Divergence info for Loop at depth 1 containing: [[BB0:BB[0-9]+]]<header><latch><exiting>
; CHECK-EMPTY:
; CHECK-NEXT:  Basic Block: [[BB0]]
; CHECK-NEXT:  Divergent: [Shape: Unit Stride, Stride: i64 1] i64 [[VP_PHI1:%.*]] = phi  [ i64 [[TMP:%.*]], [[BB1:BB[0-9]+]] ],  [ i64 [[VP_PHI_NEXT1:%.*]], [[BB0]] ]
; CHECK-NEXT:  Divergent: [Shape: Strided, Stride: i64 4] i32* [[VP_GEP:%.*]] = getelementptr inbounds [1024 x i32]* %src i64 0 i64 [[VP_PHI1]]
; CHECK-NEXT:  Divergent: [Shape: Strided, Stride: i64 4] i32 addrspace(4)* [[VP_GEP_ASCAST:%.*]] = addrspacecast i32* [[VP_GEP]]
; CHECK-NEXT:  Divergent: [Shape: Strided, Stride: i64 4] i32 addrspace(3)* [[VP_GEP_ASCAST2:%.*]] = addrspacecast i32 addrspace(4)* [[VP_GEP_ASCAST]]
; CHECK-NEXT:  Divergent: [Shape: Random] i32 [[VP_LOAD:%.*]] = load i32 addrspace(3)* [[VP_GEP_ASCAST2]]
; CHECK-NEXT:  Divergent: [Shape: Random] i32 [[VP_OP:%.*]] = mul i32 [[VP_LOAD]] i32 3

define dso_local void @test_gep_addrpspace_cast([1024 x i32]* %src) {
; CHECK:       VPlannedBB1:
; CHECK-NEXT:    br i1 false, label %scalar.ph, label %vector.ph
; CHECK-EMPTY:
; CHECK-NEXT:  vector.ph:
; CHECK-NEXT:    br label %vector.body
; CHECK-EMPTY:
; CHECK-NEXT:  vector.body:
; CHECK-NEXT:    [[UNI_PHI1:%.*]] = phi i64 [ 0, %vector.ph ], [ [[UNI_PHI1_NEXT:%.*]], %vector.body ]
; CHECK-NEXT:    [[VEC_PHI:%.*]] = phi <8 x i64> [ <i64 0, i64 1, i64 2, i64 3, i64 4, i64 5, i64 6, i64 7>, %vector.ph ], [ [[VEC_PHI_NEXT:%.]], %vector.body ]
; CHECK-NEXT:    [[SCALAR_GEP:%.*]] = getelementptr inbounds [1024 x i32], [1024 x i32]* %src, i64 0, i64 [[UNI_PHI1]]
; CHECK-NEXT:    [[SCALAR_GEP_ASCAST:%.*]] = addrspacecast i32* [[SCALAR_GEP]] to i32 addrspace(4)*
; CHECK-NEXT:    [[SCALAR_GEP_ASCAST2:%.*]] = addrspacecast i32 addrspace(4)* [[SCALAR_GEP_ASCAST]] to i32 addrspace(3)*
; CHECK-NEXT:    [[ASCAST_WIDE_LOAD:%.*]] = bitcast i32 addrspace(3)* [[SCALAR_GEP_ASCAST2]] to <8 x i32> addrspace(3)*
; CHECK-NEXT:    [[WIDE_LOAD:%.*]] = load <8 x i32>, <8 x i32> addrspace(3)* [[ASCAST_WIDE_LOAD]]
; CHECK-NEXT:    [[OP:%.*]] = mul <8 x i32> [[WIDE_LOAD]], <i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3, i32 3>
; CHECK-NEXT:    [[ASCAST_WIDE_STORE:%.*]] = bitcast i32*  [[SCALAR_GEP]] to <8 x i32>*
; CHECK-NEXT:    store <8 x i32> [[OP]], <8 x i32>* [[ASCAST_WIDE_STORE]]
; CHECK-NEXT:    [[VEC_PHI_NEXT]] = add nuw nsw <8 x i64> [[VEC_PHI]], <i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8, i64 8>
; CHECK-NEXT:    [[UNI_PHI1_NEXT]] = add nuw nsw i64 [[UNI_PHI1]], 8

DIR.OMP.SIMD.1:                                   ; preds = %omp.inner.for.body.lr.ph
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %iv1 = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %iv1.next, %for.body ]

  %gep = getelementptr inbounds [1024 x i32], [1024 x i32]* %src, i64 0, i64 %iv1
  %gep.ascast = addrspacecast i32* %gep to i32 addrspace(4)*
  %gep.ascast2 = addrspacecast i32 addrspace(4)* %gep.ascast to i32 addrspace(3)*
  %ld.1 = load i32, i32 addrspace(3)* %gep.ascast2, align 4
  %mul = mul i32 %ld.1, 3
  store i32 %mul, i32* %gep, align 4
  %iv1.next = add nuw nsw i64 %iv1, 1
  %cmp = icmp ult i64 %iv1.next, 1024
  br i1 %cmp, label %for.body, label %for.end
for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token %0)
