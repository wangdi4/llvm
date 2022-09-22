; This test verifies that correct alignment is used for gather/scatter.

;RUN: opt -vplan-enable-soa=false -S -vplan-vec -vplan-force-vf=2 %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

define void @_ZGVdN2u_test_vector(i8 %uni) {
; CHECK-LABEL: @_ZGVdN2u_test_vector
; CHECK:  entry:
; CHECK:    [[SPRIVATESTORAGE_DST_VEC0:%.*]] = alloca [2 x [2 x <4 x i8>]], align 4
; CHECK:    [[SPRIVATESTORAGE_DST_VEC_BC0:%.*]] = bitcast [2 x [2 x <4 x i8>]]* [[SPRIVATESTORAGE_DST_VEC0]] to [2 x <4 x i8>]*
; CHECK:    [[SPRIVATESTORAGE_DST_VEC_BASE_ADDR0:%.*]] = getelementptr [2 x <4 x i8>], [2 x <4 x i8>]* [[SPRIVATESTORAGE_DST_VEC_BC0]], <2 x i32> <i32 0, i32 1>
; CHECK:    [[SPRIVATESTORAGE_SRC_VEC0:%.*]] = alloca [2 x [2 x <4 x i8>]], align 4
; CHECK:    [[SPRIVATESTORAGE_SRC_VEC_BC0:%.*]] = bitcast [2 x [2 x <4 x i8>]]* [[SPRIVATESTORAGE_SRC_VEC0]] to [2 x <4 x i8>]*
; CHECK:    [[SPRIVATESTORAGE_SRC_VEC_BASE_ADDR0:%.*]] = getelementptr [2 x <4 x i8>], [2 x <4 x i8>]* [[SPRIVATESTORAGE_SRC_VEC_BC0]], <2 x i32> <i32 0, i32 1>
; CHECK:  vector.body:
; CHECK:    [[MM_VECTORGEP0:%.*]] = getelementptr inbounds [2 x <4 x i8>], <2 x [2 x <4 x i8>]*> [[SPRIVATESTORAGE_SRC_VEC_BASE_ADDR0]], <2 x i64> zeroinitializer, <2 x i64> zeroinitializer
; CHECK:    [[MM_VECTORGEP30:%.*]] = getelementptr inbounds [2 x <4 x i8>], <2 x [2 x <4 x i8>]*> [[SPRIVATESTORAGE_DST_VEC_BASE_ADDR0]], <2 x i64> zeroinitializer, <2 x i64> zeroinitializer

; CHECK:    [[TMP1:%.*]] = bitcast <2 x <4 x i8>*> [[MM_VECTORGEP0]] to <2 x i8*>
; CHECK:    [[VECBASEPTR_0:%.*]] = shufflevector <2 x i8*> [[TMP1]], <2 x i8*> undef, <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1>
; CHECK:    [[ELEMBASEPTR_0:%.*]] = getelementptr i8, <8 x i8*> [[VECBASEPTR_0]], <8 x i64> <i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3>
; CHECK:    [[WIDE_MASKED_GATHER0:%.*]] = call <8 x i8> @llvm.masked.gather.v8i8.v8p0i8(<8 x i8*> [[ELEMBASEPTR_0]], i32 1, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>, <8 x i8> poison)

; CHECK:    [[TMP2:%.*]] = bitcast <2 x <4 x i8>*> [[MM_VECTORGEP30]] to <2 x i8*>
; CHECK:    [[VECBASEPTR_40:%.*]] = shufflevector <2 x i8*> [[TMP2]], <2 x i8*> undef, <8 x i32> <i32 0, i32 0, i32 0, i32 0, i32 1, i32 1, i32 1, i32 1>
; CHECK:    [[ELEMBASEPTR_50:%.*]] = getelementptr i8, <8 x i8*> [[VECBASEPTR_40]], <8 x i64> <i64 0, i64 1, i64 2, i64 3, i64 0, i64 1, i64 2, i64 3>
; CHECK:    call void @llvm.masked.scatter.v8i8.v8p0i8(<8 x i8> [[WIDE_MASKED_GATHER0]], <8 x i8*> [[ELEMBASEPTR_50]], i32 1, <8 x i1> <i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true, i1 true>)

entry:
  %sPrivateStorage.src = alloca [2 x <4 x i8>], align 4
  %sPrivateStorage.dst = alloca [2 x <4 x i8>], align 4
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(i8 %uni), "QUAL.OMP.PRIVATE:TYPED"([2 x <4 x i8>]* %sPrivateStorage.src, <4 x i8> zeroinitializer, i32 2), "QUAL.OMP.PRIVATE:TYPED"([2 x <4 x i8>]* %sPrivateStorage.dst, <4 x i8> zeroinitializer, i32 2)]
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop ]
  %0 = sext i32 %index to i64
  %arrayidx.src = getelementptr inbounds [2 x <4 x i8>], [2 x <4 x i8>]* %sPrivateStorage.src, i64 0, i64 0
  %arrayidx.dst = getelementptr inbounds [2 x <4 x i8>], [2 x <4 x i8>]* %sPrivateStorage.dst, i64 0, i64 0
  %load = load <4 x i8>, <4 x i8>* %arrayidx.src, align 4
  store <4 x i8> %load, <4 x i8>* %arrayidx.dst, align 4
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define void @_ZGVdN2u_test_scalar(i8 %uni) {
; CHECK-LABEL: @_ZGVdN2u_test_scalar
; CHECK:  entry:
; CHECK:    [[SPRIVATESTORAGE_DST_VEC0:%.*]] = alloca [2 x [2 x i32]], align 4
; CHECK:    [[SPRIVATESTORAGE_DST_VEC_BC0:%.*]] = bitcast [2 x [2 x i32]]* [[SPRIVATESTORAGE_DST_VEC0]] to [2 x i32]*
; CHECK:    [[SPRIVATESTORAGE_DST_VEC_BASE_ADDR0:%.*]] = getelementptr [2 x i32], [2 x i32]* [[SPRIVATESTORAGE_DST_VEC_BC0]], <2 x i32> <i32 0, i32 1>
; CHECK:    [[SPRIVATESTORAGE_SRC_VEC0:%.*]] = alloca [2 x [2 x i32]], align 4
; CHECK:    [[SPRIVATESTORAGE_SRC_VEC_BC0:%.*]] = bitcast [2 x [2 x i32]]* [[SPRIVATESTORAGE_SRC_VEC0]] to [2 x i32]*
; CHECK:    [[SPRIVATESTORAGE_SRC_VEC_BASE_ADDR0:%.*]] = getelementptr [2 x i32], [2 x i32]* [[SPRIVATESTORAGE_SRC_VEC_BC0]], <2 x i32> <i32 0, i32 1>
; CHECK:  vector.body:
; CHECK:    [[MM_VECTORGEP0:%.*]] = getelementptr inbounds [2 x i32], <2 x [2 x i32]*> [[SPRIVATESTORAGE_SRC_VEC_BASE_ADDR0]], <2 x i64> zeroinitializer, <2 x i64> zeroinitializer
; CHECK:    [[MM_VECTORGEP30:%.*]] = getelementptr inbounds [2 x i32], <2 x [2 x i32]*> [[SPRIVATESTORAGE_DST_VEC_BASE_ADDR0]], <2 x i64> zeroinitializer, <2 x i64> zeroinitializer
; CHECK:    [[WIDE_MASKED_GATHER0:%.*]] = call <2 x i32> @llvm.masked.gather.v2i32.v2p0i32(<2 x i32*> [[MM_VECTORGEP0]], i32 4, <2 x i1> <i1 true, i1 true>, <2 x i32> poison)
; CHECK:    call void @llvm.masked.scatter.v2i32.v2p0i32(<2 x i32> [[WIDE_MASKED_GATHER0]], <2 x i32*> [[MM_VECTORGEP30]], i32 4, <2 x i1> <i1 true, i1 true>)
entry:
  %sPrivateStorage.src = alloca [2 x i32], align 4
  %sPrivateStorage.dst = alloca [2 x i32], align 4
  br label %simd.begin.region

simd.begin.region:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.UNIFORM"(i8 %uni), "QUAL.OMP.PRIVATE:TYPED"([2 x i32]* %sPrivateStorage.src, i32 0, i32 2), "QUAL.OMP.PRIVATE:TYPED"([2 x i32]* %sPrivateStorage.dst, i32 0, i32 2)]
  br label %simd.loop

simd.loop:
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %arrayidx.src = getelementptr inbounds [2 x i32], [2 x i32]* %sPrivateStorage.src, i64 0, i64 0
  %arrayidx.dst = getelementptr inbounds [2 x i32], [2 x i32]* %sPrivateStorage.dst, i64 0, i64 0
  %load = load i32, i32* %arrayidx.src, align 4
  store i32 %load, i32* %arrayidx.dst, align 4
  br label %simd.loop.exit

simd.loop.exit:
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}
