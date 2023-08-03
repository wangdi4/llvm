; RUN: opt -S -passes=function-attrs %s | FileCheck %s
; CHECK: define void @pluto(ptr nocapture readonly %arg)

; This larger test hits the CaptureTracking limit implemented in llvm.org
; D126236. Unless we relax the limit, attribute analysis cannot get
; the "nocapture" attribute on "%arg".

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.foo = type { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }

define void @pluto(ptr %arg) local_unnamed_addr #0 {
bb:
  %tmp = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 0
  %tmp1 = load ptr, ptr %tmp, align 1
  %tmp2 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 6, i64 0, i32 1
  %tmp3 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp2, i32 0)
  %tmp4 = load i64, ptr %tmp3, align 1
  %tmp5 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp2, i32 1)
  %tmp6 = load i64, ptr %tmp5, align 1
  %tmp7 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp2, i32 2)
  br label %bb8

bb8:                                              ; preds = %bb
  br i1 undef, label %bb16, label %bb9

bb9:                                              ; preds = %bb8
  %tmp10 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 0
  %tmp11 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 6, i64 0, i32 1
  %tmp12 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp11, i32 1)
  %tmp13 = load i64, ptr %tmp12, align 1
  %tmp14 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp11, i32 2)
  %tmp15 = load i64, ptr %tmp14, align 1
  unreachable

bb16:                                             ; preds = %bb8
  %tmp17 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 0
  %tmp18 = load ptr, ptr %tmp17, align 1
  %tmp19 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 6, i64 0, i32 1
  %tmp20 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp19, i32 0)
  %tmp21 = load i64, ptr %tmp20, align 1
  %tmp22 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp19, i32 1)
  %tmp23 = load i64, ptr %tmp22, align 1
  %tmp24 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp19, i32 2)
  %tmp25 = load i64, ptr %tmp24, align 1
  br label %bb26

bb26:                                             ; preds = %bb16
  br i1 undef, label %bb37, label %bb27

bb27:                                             ; preds = %bb26
  %tmp28 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 0
  %tmp29 = load ptr, ptr %tmp28, align 1
  %tmp30 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 6, i64 0, i32 1
  %tmp31 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp30, i32 0)
  %tmp32 = load i64, ptr %tmp31, align 1
  %tmp33 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp30, i32 1)
  %tmp34 = load i64, ptr %tmp33, align 1
  %tmp35 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp30, i32 2)
  %tmp36 = load i64, ptr %tmp35, align 1
  unreachable

bb37:                                             ; preds = %bb26
  br label %bb38

bb38:                                             ; preds = %bb37
  br i1 undef, label %bb80, label %bb39

bb39:                                             ; preds = %bb38
  %tmp40 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 0
  %tmp41 = load ptr, ptr %tmp40, align 1
  %tmp42 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 6, i64 0, i32 1
  %tmp43 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp42, i32 0)
  %tmp44 = load i64, ptr %tmp43, align 1
  %tmp45 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp42, i32 1)
  %tmp46 = load i64, ptr %tmp45, align 1
  %tmp47 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp42, i32 2)
  %tmp48 = load i64, ptr %tmp47, align 1
  %tmp49 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 0
  %tmp50 = load ptr, ptr %tmp49, align 1
  %tmp51 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 6, i64 0, i32 1
  %tmp52 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp51, i32 0)
  %tmp53 = load i64, ptr %tmp52, align 1
  %tmp54 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp51, i32 1)
  %tmp55 = load i64, ptr %tmp54, align 1
  %tmp56 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp51, i32 2)
  %tmp57 = load i64, ptr %tmp56, align 1
  %tmp58 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 0
  %tmp59 = load ptr, ptr %tmp58, align 1
  %tmp60 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 6, i64 0, i32 1
  %tmp61 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp60, i32 0)
  %tmp62 = load i64, ptr %tmp61, align 1
  %tmp63 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp60, i32 1)
  %tmp64 = load i64, ptr %tmp63, align 1
  %tmp65 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp60, i32 2)
  %tmp66 = load i64, ptr %tmp65, align 1
  %tmp67 = load ptr, ptr %tmp58, align 1
  %tmp68 = load i64, ptr %tmp61, align 1
  %tmp69 = load i64, ptr %tmp63, align 1
  %tmp70 = load i64, ptr %tmp65, align 1
  %tmp71 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 0
  %tmp72 = load ptr, ptr %tmp71, align 1
  %tmp73 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 6, i64 0, i32 1
  %tmp74 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp73, i32 0)
  %tmp75 = load i64, ptr %tmp74, align 1
  %tmp76 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp73, i32 1)
  %tmp77 = load i64, ptr %tmp76, align 1
  %tmp78 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp73, i32 2)
  %tmp79 = load i64, ptr %tmp78, align 1
  unreachable

bb80:                                             ; preds = %bb38
  %tmp81 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 0
  %tmp82 = load ptr, ptr %tmp81, align 1
  %tmp83 = getelementptr inbounds %struct.foo, ptr %arg, i64 0, i32 6, i64 0, i32 1
  %tmp84 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp83, i32 0)
  %tmp85 = load i64, ptr %tmp84, align 1
  %tmp86 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp83, i32 1)
  %tmp87 = load i64, ptr %tmp86, align 1
  %tmp88 = call ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %tmp83, i32 2)
  %tmp89 = load i64, ptr %tmp88, align 1
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, ptr, i32) #1

attributes #0 = { "unsafe-fp-math"="true" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
