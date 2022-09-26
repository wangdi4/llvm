; The test checks that if nested aliases of a private are outside the SIMD loop,
; and the user of a nested alias is inside the SIMD loop, then VPlan will
; correctly widen the private and update uses of its alias.

; RUN: opt -vplan-enable-soa=false -S -vplan-vec -vplan-force-vf=4 %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo() local_unnamed_addr {
; CHECK-LABEL: @foo()
; CHECK-LABEL:  simd.loop.preheader:
; CHECK:    [[SRC_GEP0:%.*]] = getelementptr inbounds i32, i32* [[PRIVATE_SRC0:%.*]], i64 0
; CHECK:    [[SRC_BC_00:%.*]] = bitcast i32* [[SRC_GEP0]] to i8*
; CHECK:    [[SRC_BC_10:%.*]] = bitcast i8* [[SRC_BC_00]] to i32*
; CHECK:    [[DST_GEP0:%.*]] = getelementptr inbounds i32, i32* [[PRIVATE_DST0:%.*]], i64 0
; CHECK:    [[DST_BC_00:%.*]] = bitcast i32* [[DST_GEP0]] to i8*
; CHECK:    [[DST_BC_10:%.*]] = bitcast i8* [[DST_BC_00]] to i32*

; CHECK-LABEL: vector.body:
; CHECK:    [[TMP4:%.*]] = bitcast i32* [[TMP1:%.*]] to <4 x i32>*
; CHECK:    [[WIDE_LOAD0:%.*]] = load <4 x i32>, <4 x i32>* [[TMP4]], align 4
; CHECK:    [[TMP5:%.*]] = bitcast i32* [[TMP3:%.*]] to <4 x i32>*
; CHECK:    store <4 x i32> [[WIDE_LOAD0]], <4 x i32>* [[TMP5]], align 4
;

entry:
  %private.src = alloca i32
  %private.dst = alloca i32
  br label %simd.begin.region


simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"(i32* %private.src, i32* %private.dst), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %simd.loop.preheader

simd.loop.preheader:
  ; nested aliases
  %src.gep = getelementptr inbounds i32, i32* %private.src, i64 0
  %src.bc.0 = bitcast i32* %src.gep to i8*
  %src.bc.1 = bitcast i8* %src.bc.0 to i32*
  %dst.gep = getelementptr inbounds i32, i32* %private.dst, i64 0
  %dst.bc.0 = bitcast i32* %dst.gep to i8*
  %dst.bc.1 = bitcast i8* %dst.bc.0 to i32*
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.exit ]
  %load = load i32, i32* %src.bc.1
  store i32 %load, i32* %dst.bc.1
  br label %simd.loop.exit

simd.loop.exit:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define dso_local void @test_alias_store_outside_loop() local_unnamed_addr {
; CHECK: @test_alias_store_outside_loop
; CHECK-NOT: {{.*}} = alloca <4 x i32>
entry:
  %x = alloca i32
  %y = alloca float*
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"(i32* %x, float** %y)]
  br label %simd.loop.preheader

simd.loop.preheader:
  %cast_src = bitcast i32* %x to float*
  %cast_dst = bitcast float** %y to i32**
  store float* %cast_src, float** %y
  store i32* %x, i32** %cast_dst
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop]
  %ld = load float*, float** %y
  store float 7.0, float* %cast_src
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Test to make sure that the legality is not tripped and a store within the loop does not prevent vectorization.
define dso_local void @test_legality_safe_store(i32** %y) {
; CHECK: @test_legality_safe_store
; CHECK: {{.*}} = alloca <4 x i32>
entry:
  %x = alloca i32
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"(i32* %x)]
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.begin.region], [ %indvar, %simd.loop]
  store i32* %x, i32** %y
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 4
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
