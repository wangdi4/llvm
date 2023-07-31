; RUN: opt -S -passes=vplan-vec -vplan-force-vf=4 %s | FileCheck %s

; Check that we generate scalar addrspacecast for last lane
; when we have stores of the casted value to uniform location.

define dso_local void @test_single_scalar(ptr %to.store, ptr %to.store.ascast) {
entry:
  %ptr = alloca i32
; CHECK:       [[VEC_ALLOCA:%.*]] = alloca <4 x i32>, align 16
; CHECK-NEXT:  [[VEC_ALLOCA_BASE_ADDR:%.*]] = getelementptr i32, ptr [[VEC_ALLOCA]], <4 x i32> <i32 0, i32 1, i32 2, i32 3>
; CHECK-NEXT:  [[VEC_ALLOCA_BASE_ADDR_EXTRACT_3:%.*]] = extractelement <4 x ptr>  [[VEC_ALLOCA_BASE_ADDR]], i32 3
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(ptr %ptr, i32 0, i32 1)]
  br label %simd.loop

simd.loop:
; CHECK: store ptr [[VEC_ALLOCA_BASE_ADDR_EXTRACT_3]], ptr %to.store, align 8
; CHECK: [[VEC_ALLOCA_BASE_ADDR_EXTRACT_3_AS_CAST:%.*]] = addrspacecast ptr [[VEC_ALLOCA_BASE_ADDR_EXTRACT_3]] to ptr addrspace(4)
; CHECK-NEXT:  store ptr addrspace(4) [[VEC_ALLOCA_BASE_ADDR_EXTRACT_3_AS_CAST]], ptr %to.store.ascast, align 8
  %index = phi i32 [ 0, %simd.begin.region], [ %indvar, %simd.loop]
  store ptr %ptr, ptr %to.store
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  %alloca.ascast = addrspacecast ptr %ptr to ptr addrspace(4)
  store ptr addrspace(4) %alloca.ascast, ptr %to.store.ascast
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
