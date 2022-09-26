; RUN: opt -S -vplan-vec -vplan-force-vf=4 %s | FileCheck %s

; Check that we generate scalar addrspacecast for last lane
; when we have stores of the casted value to uniform location.

define dso_local void @test_single_scalar(i32** %to.store, i32 addrspace(4)** %to.store.ascast) {
entry:
  %ptr = alloca i32
; CHECK:       [[VEC_ALLOCA:%.*]] = alloca <4 x i32>, align 16
; CHECK-NEXT:  [[VEC_ALLOCA_BC:%.*]] = bitcast <4 x i32>* [[VEC_ALLOCA]] to i32*
; CHECK-NEXT:  [[VEC_ALLOCA_BASE_ADDR:%.*]] = getelementptr i32, i32* [[VEC_ALLOCA_BC]], <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK-NEXT:  [[VEC_ALLOCA_BASE_ADDR_EXTRACT_3:%.*]] = extractelement <4 x i32*>  [[VEC_ALLOCA_BASE_ADDR]], i32 3
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"(i32* %ptr)]
  br label %simd.loop

simd.loop:
; CHECK: store i32* [[VEC_ALLOCA_BASE_ADDR_EXTRACT_3]], i32** %to.store, align 8
; CHECK: [[VEC_ALLOCA_BASE_ADDR_EXTRACT_3_AS_CAST:%.*]] = addrspacecast i32* [[VEC_ALLOCA_BASE_ADDR_EXTRACT_3]] to i32 addrspace(4)*
; CHECK-NEXT:  store i32 addrspace(4)* [[VEC_ALLOCA_BASE_ADDR_EXTRACT_3_AS_CAST]], i32 addrspace(4)** %to.store.ascast, align 8
  %index = phi i32 [ 0, %simd.begin.region], [ %indvar, %simd.loop]
  store i32* %ptr, i32** %to.store
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  %alloca.ascast = addrspacecast i32* %ptr to i32 addrspace(4)*
  store i32 addrspace(4)* %alloca.ascast, i32 addrspace(4)** %to.store.ascast
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
